#include <map>
#include <string>

class OrderBook
{
public:
	OrderBook() {}	
	void populateSnapshot(const std::string& response) {}
	void update(const std::string& jsonUpdate) {}
	

private:
	std::unique_ptr<WebSocketClient> webSocket;
	uint64_t lastUpdateID;
	std::map<double, double> bids;
	std::map<double, double> asks;


};
