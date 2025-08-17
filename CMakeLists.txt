cmake_minimum_required(VERSION 3.16)

project(terminal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Exec
	src/main.cpp
	src/OrderBook.cpp
	src/BinanceClient.cpp
	src/ClientManager.cpp
	)

find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json 3.6.1 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(fmt REQUIRED)

target_link_libraries(Exec PRIVATE
	Boost::system
	OpenSSL::SSL
	OpenSSL::Crypto
	fmt::fmt
	nlohmann_json::nlohmann_json
	)


target_include_directories(Exec PRIVATE ./include)

