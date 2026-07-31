import json
import threading
import socketserver
from collections import defaultdict
from pathlib import Path

import pandas as pd
import streamlit as st

st.set_page_config(page_title="Crypto Market Dashboard", layout="wide")

st.title("Crypto Market Dashboard")
st.markdown("A live view of the analytics updates emitted by the C++ data manager.")

LIVE_HOST = "127.0.0.1"
LIVE_PORT = 8765
MAX_BUFFER_SIZE = 2000

live_rows = defaultdict(list)
listener_lock = threading.Lock()
listener_started = False
listener_thread = None


class AnalyticsStreamHandler(socketserver.StreamRequestHandler):
    def handle(self):
        while True:
            raw_line = self.rfile.readline()
            if not raw_line:
                break
            line = raw_line.decode("utf-8", errors="replace").strip()
            if not line:
                continue

            try:
                payload = json.loads(line)
            except json.JSONDecodeError:
                continue

            if payload.get("type") != "analytics_update":
                continue

            entry = {
                "product": payload.get("product"),
                "timestamp": payload.get("timestamp"),
                "price": payload.get("price"),
                "buyVolume": payload.get("buyVolume"),
                "sellVolume": payload.get("sellVolume"),
                "tradesLastMinute": payload.get("tradesLastMinute"),
                "mid": payload.get("mid"),
                "spread": payload.get("spread"),
                "variance": payload.get("variance"),
                "vol30s": payload.get("vol30s"),
                "vol5m": payload.get("vol5m"),
            }

            with listener_lock:
                live_rows[entry["product"]].append(entry)
                if len(live_rows[entry["product"]]) > MAX_BUFFER_SIZE:
                    live_rows[entry["product"]] = live_rows[entry["product"]][-MAX_BUFFER_SIZE:]


class AnalyticsTCPServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True


def ensure_live_listener():
    global listener_started, listener_thread

    if listener_started:
        return

    server = AnalyticsTCPServer((LIVE_HOST, LIVE_PORT), AnalyticsStreamHandler)

    def serve_forever():
        server.serve_forever()

    listener_thread = threading.Thread(target=serve_forever, daemon=True)
    listener_thread.start()
    listener_started = True


ensure_live_listener()


def candidate_analytics_paths() -> list[Path]:
    dashboard_dir = Path(__file__).resolve().parent
    repo_root = dashboard_dir.parent
    return [
        dashboard_dir / "analytics.json",
        repo_root / "build" / "dashboard" / "analytics.json",
        Path.cwd() / "dashboard" / "analytics.json",
        Path.cwd() / "build" / "dashboard" / "analytics.json",
    ]


def resolve_analytics_path() -> Path:
    candidates = candidate_analytics_paths()
    existing = [path for path in candidates if path.exists()]
    if not existing:
        return candidates[0]

    def row_count(path: Path) -> int:
        try:
            with path.open("r", encoding="utf-8") as handle:
                payload = json.load(handle)
        except (OSError, json.JSONDecodeError):
            return -1

        if isinstance(payload, list):
            return len(payload)
        return 0

    return max(existing, key=lambda path: (row_count(path), path.stat().st_mtime))


DATA_PATH = resolve_analytics_path()

st.sidebar.header("Filters")
auto_refresh = st.sidebar.checkbox("Auto refresh", value=True)

if auto_refresh:
    st.sidebar.caption("Refreshing on each rerun")

st.autorefresh(interval=1000, key="live_refresh")


def load_analytics_data(path: Path) -> pd.DataFrame:
    if not path.exists():
        return pd.DataFrame(columns=["product", "timestamp", "price", "buyVolume", "sellVolume", "tradesLastMinute", "mid", "spread", "variance", "vol30s", "vol5m"])

    with path.open("r", encoding="utf-8") as handle:
        payload = json.load(handle)

    if not payload:
        return pd.DataFrame(columns=["product", "timestamp", "price", "buyVolume", "sellVolume", "tradesLastMinute", "mid", "spread", "variance", "vol30s", "vol5m"])

    frame = pd.DataFrame(payload)
    frame["timestamp"] = pd.to_datetime(frame["timestamp"], unit="ms")
    return frame


with listener_lock:
    live_payload = []
    for product, entries in live_rows.items():
        live_payload.extend(entries)

if live_payload:
    analytics_df = pd.DataFrame(live_payload)
    analytics_df["timestamp"] = pd.to_datetime(analytics_df["timestamp"], unit="ms")
else:
    analytics_df = load_analytics_data(DATA_PATH)

if analytics_df.empty:
    st.info("No analytics snapshots have been written yet. Start the C++ feed and the dashboard will populate automatically.")
    st.stop()

symbols = sorted(analytics_df["product"].dropna().unique().tolist())
symbol = st.sidebar.selectbox("Instrument", symbols)

filtered = analytics_df[analytics_df["product"] == symbol].copy()
filtered = filtered.sort_values("timestamp")

if filtered.empty:
    st.warning("No rows are available for the selected instrument yet.")
    st.stop()

latest = filtered.iloc[-1]

col1, col2, col3, col4 = st.columns(4)
col1.metric("Latest price", f"{latest['price']:,.2f}")
col2.metric("Spread", f"{latest['spread']:,.4f}")
col3.metric("Vol 30s", f"{latest['vol30s']:,.4f}")
col4.metric("Vol 5m", f"{latest['vol5m']:,.4f}")

st.subheader(f"{symbol} price")
st.line_chart(filtered, x="timestamp", y=["price"])

st.subheader(f"{symbol} volatility")
st.line_chart(filtered, x="timestamp", y=["vol30s", "vol5m"])

st.subheader("Recent metrics")
st.dataframe(filtered[["timestamp", "price", "buyVolume", "sellVolume", "tradesLastMinute", "spread", "variance", "vol30s", "vol5m"]].tail(20), use_container_width=True)
