cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook CXX)
add_library(Limit_Order_Book Agents.cpp order_book.cpp matching_engine.cpp)

