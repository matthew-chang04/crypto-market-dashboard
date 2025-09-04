#include "WebSocketClient.hpp"
#include <nlohmann/json.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp> 
#include <optional>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp> // verify this
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <string>
#include <optional>
#include <condition_variable>
#include <thread>
#include <iostream>
#include <mutex>
#include <queue>
#include <fmt/format.h>
#include <stdexcept>
#include <chrono>

namespace beast = boost::beast;   
namespace http = beast::http;          
namespace websocket = beast::websocket;
namespace net = boost::asio;           
using tcp = boost::asio::ip::tcp;

void WebSocketClient::start() {  
    do_resolve();
}

void WebSocketClient::stop() {
	beast::error_code ec;
	if (ws_ && ws_->is_open()) {
		std::shared_ptr<WebSocketClient> self = shared_from_this();
		ws_->async_close(websocket::close_code::normal, 
			[self](beast::error_code ec) {
				if (ec == boost::asio::ssl::error::stream_truncated || ec == boost::asio::error::eof) {
					ec.clear();
				} else if (ec) {
					std::cerr << "Error closing WebSocket: " << ec.message() << std::endl;
				}

				if (self->ws_ && self->ws_->next_layer().lowest_layer().is_open()) {
					boost::system::error_code ec2;
					self->ws_->next_layer().lowest_layer().close(ec2);
					if (ec2) {
						std::cerr << "Error closing TCP socket: " << ec2.message() << std::endl;
					}
					{
						std::lock_guard<std::mutex> lock(self->mutex_);
						self->setInterrupted(true);
						self->readDump_.consume(self->readDump_.size());
						std::cout << "WebSocket closed successfully." << std::endl;
					}		
				} else {
					std::cout << "WebSocket already closed." << std::endl;
				}
			});
	} else {
		std::cout << "WebSocket already closed." << std::endl;
	}
}



void WebSocketClient::do_resolve() {
    std::shared_ptr<WebSocketClient> self = shared_from_this();

    resolver_.async_resolve(host_, port_, 
        [self](beast::error_code ec, tcp::resolver::results_type results) {
            if (ec) return self->retryStart(ec);
            self->do_connect(results);
        });
}

void WebSocketClient::do_connect(tcp::resolver::results_type results) {
    std::shared_ptr<WebSocketClient> self = shared_from_this();
	net::async_connect(beast::get_lowest_layer(*ws_), results,
		[self](beast::error_code ec, tcp::endpoint ep) {
			if (ec) return self->retryStart(ec);

			if (!SSL_set_tlsext_host_name(self->ws_->next_layer().native_handle(), self->host_.c_str())) {
				beast::error_code ec(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
				return self->retryStart(ec);
			}
			self->do_ssl_handshake();
		});
}

void WebSocketClient::do_ssl_handshake() {
	std::shared_ptr<WebSocketClient> self = shared_from_this();
	ws_->next_layer().set_verify_callback(net::ssl::host_name_verification(host_));

	ws_->set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
	ws_->set_option(websocket::stream_base::decorator(
		[](websocket::request_type& req) {
			req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + "websocket-client-coro");
		}));

	ws_->next_layer().async_handshake(net::ssl::stream_base::client, 
		[self](beast::error_code ec) {
			if (ec) return self->retryStart(ec);
			return self->do_ws_handshake();
		});
}

void WebSocketClient::do_ws_handshake() {
	std::shared_ptr<WebSocketClient> self = shared_from_this();
	self->ws_->async_handshake(host_, target_, 
		[self](beast::error_code ec) {
			if (ec) return self->retryStart(ec);
			std::cout << "Sucessfully connected to" << self->host_ << std::endl;

			self->subscribe();
		});
}

void WebSocketClient::retryStart(beast::error_code ec) {
    std::cerr << "Retrying connection due to error: " << ec.message() << std::endl;
    std::shared_ptr<WebSocketClient> self = shared_from_this();
    self->do_resolve();
}

void WebSocketClient::do_read() {
	std::shared_ptr<WebSocketClient> self = shared_from_this();
	ws_->async_read(readDump_,
		[self](beast::error_code ec, size_t bytes_written) {
			if (ec) {
				std::cerr << "WebSocket Read Error: " << ec.message() << std::endl;
				self->setInterrupted(true);
				self->reset();
				return;
			}
			std::string msg = beast::buffers_to_string(self->readDump_.data());
			if (self->on_message_) {
				self->on_message_(msg);
			}
			self->readDump_.consume(self->readDump_.size());
			self->do_read();
		});
}

void WebSocketClient::reset() {
	stop();
	start();
}

void WebSocketClient::subscribe() {
	if (target_ == "orderbook") {
		subscribe_orderbook(normalize_symbol(symbol_));
	} else if (target_ == "ticker") {
		subscribe_ticker(normalize_symbol(symbol_), "1m"); 
	} else {
		std::cerr << "Unknown subscription target: " << target_ << std::endl;
	}
}
