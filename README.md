# crypto-market-dashboard
This project displays **Cryptocurrency market data** with supplementary insights from **options data**

It involves:
* [WebSocket Client Manager](#websocket-client-manager) - To get real-time financial data from different Cryptocurrency exchanges
* [Data Manager](#data-manager) - To parse JSON information obtained from WebSockets for in-depth analysis
* Data Visualisation - Displays Coin information trade information, orderbooks, and Option insights (ie: Implied Volatility Surface).

<img src="https://github.com/matthew-chang04/crypto-market-dashboard/blob/main/resources/IVSurvace" alt="Implied Volatility Heatmap" width="50%" />

* Options Insight Engine - Performs calculations to derrive **Implied Volatility from real-time option prices**, and use the Greeks to provide more context to simple market prices and orderbooks.

## WebSocket Client Manager
This portion uses ```boost.beast``` and ```boost.asio``` to set up websocket connections for cryptocurrency exchanges. The clients are meant to be flexible for a variety of possible websocket streams and exchanges. 

The Client Manager handles establishing the connections and running IO Context threads for them to read and write to and from. It also contains a special Options Data client that takes care of maintaining the right spread of options available depending on the current spot price of the underlying. 

## Data Manager

A single data manager handles reads from websockets that are parsed into custom data structures to track the current spot price, and option spread of a given coin. The data manager feeds information into the Options Engine.
