import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import streamlit as st

st.set_page_config(page_title="Crypto Market Dashboard", layout="wide")

st.title("Crypto Market Dashboard")
st.markdown("A dashboard to visualize cryptocurrency market data.")

st.sidebar.header("Filters")
symbol = st.sidebar.selectbox("Select Coin", ["BTC", "ETH", "XRP"])
expriry = st.sidebar.selectbox("Select Max Expiry", ["1M", "2w", "1w"])
autoRefresh = st.sidebar.checkbox("Auto Refresh", value=True)

# DEMO DATA (Coins and OPTIONS)
demoOptionData = pd.DataFrame({
    "strike": [30000, 35000, 40000, 45000, 50000],
    "type": ["call", "call", "call", "put", "put"],
    "expiry": ["1M", "1M", "2w", "1w", "1w"],
    "expiryVals": [2, 2, 1, 0, 0],
    "price": [1500, 1200, 800, 600, 400],
    "IV" : [0.4, 0.45, 0.4, 0.35, 0.3]
})

demoOptionList = [
    [600, 400], # 1w
    [800, 900],      # 2w
    [1500, 1200] # 1M
]

demoCoinData = pd.DataFrame({
    "price": np.random.rand(100).cumsum() * 40000,
    "timestamp" : pd.date_range(start="2023-01-01", periods=100, freq='H')
})

col1, col2 = st.columns([2, 2])

with col1:
    st.subheader(f"{symbol} Price Chart")

    fig, ax = plt.subplots()
    ax.plot(demoCoinData["timestamp"], demoCoinData["price"], label=f"{symbol} Price")
    ax.set_xlabel("Time")
    ax.set_ylabel("Price (USD)")
    ax.legend()
    st.pyplot(fig)

with col2:
    st.subheader(f"{symbol} Options Data (Max Expiry: {expriry})")

    fig, ax2 = plt.subplots()
    im = ax2.imshow(demoOptionList, cmap='viridis', aspect='auto')

    ax.set_xlabel("Moneyness")
    ax.set_ylabel("Time to Maturity")

    ax.set_yticks(np.arange(demoOptionData["expiryVals"].max()))
    ax.set_xticks(np.arange(len(demoOptionData["strike"])))
    st.pyplot(fig)
