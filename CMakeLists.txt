cmake_minimum_required(VERSION 3.10)
project(LimitOrderBook CXX)

# 1. Your existing library
# Keep this as is, but we'll link to it later
add_library(Limit_Order_Book STATIC
    Agents.cpp
    order_book.cpp
    matching_engine.cpp
)
target_include_directories(Limit_Order_Book PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 2. Download pybind11 automatically
include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.11.1
)
FetchContent_MakeAvailable(pybind11)

# 3. Create the Python Module
# 'order_book_module' must match the name in your PYBIND11_MODULE(order_book_module, m)
# ${CMAKE_CURRENT_SOURCE_DIR} makes it look exactly where CMakeLists.txt is
pybind11_add_module(orderbook_wrapper ${CMAKE_CURRENT_SOURCE_DIR}/python_bindings.cpp)

# 4. Link everything together
# This tells the module to use the logic from your C++ library
target_link_libraries(orderbook_wrapper PRIVATE Limit_Order_Book)