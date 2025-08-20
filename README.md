# crypto-market-dashboard
This project aims to aggregate crypto market data, making more information available in one place. Cryptocurrency exchanges being decentralized is valuable for accessibility, but can lead to some discrepancies in market data insights. Aggregating markets for informative purposes allows people to make the best decisions with more information.

## WebSocket Clients
This portion uses ```boost.beast``` and ```boost.asio``` to set up websocket connections for cryptocurrency exchanges. The clients are meant to be flexible for a variety of possible websocket streams and exchanges. 

## Client Manager (in development)
Provides a concrete usage framework for the WebSocket Clients. This way certain functionalities, like a cross-exchange Orderbook, are pre-built. High flexibility with the WebSocket Clients means that lots of heavy-lifting must be done to ensure the Client Manager provides useful options.
