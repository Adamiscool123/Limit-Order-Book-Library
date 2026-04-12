cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook CXX)
add_library(Limit_Order_Book Agents.cpp order_book.cpp matching_engine.cpp)
# Add this to the GitHub repo's CMakeLists.txt
target_include_directories(Limit_Order_Book PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
