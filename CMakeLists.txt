cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook CXX)
add_executable(Limit_Order_Book main.cpp Agents.cpp order_book.cpp matching_engine.cpp)

