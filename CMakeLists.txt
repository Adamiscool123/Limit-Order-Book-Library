cmake_minimum_required(VERSION 3.14)
project(LimitOrderBook CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
  GIT_TAG        v3.0.1
)
FetchContent_MakeAvailable(pybind11)

# Download Google Benchmark for the standalone benchmark executable.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.5
)
FetchContent_MakeAvailable(googlebenchmark)

# 3. Create the Python Module
# 'order_book_module' must match the name in your PYBIND11_MODULE(order_book_module, m)
# ${CMAKE_CURRENT_SOURCE_DIR} makes it look exactly where CMakeLists.txt is
pybind11_add_module(orderbook_wrapper ${CMAKE_CURRENT_SOURCE_DIR}/python_bindings.cpp)

# 4. Link everything together
# This tells the module to use the logic from your C++ library
target_link_libraries(orderbook_wrapper PRIVATE Limit_Order_Book)

# 5. Build the agent and matching-engine benchmark harness.
add_executable(benchmark_test benchmark.cpp)
target_link_libraries(benchmark_test PRIVATE Limit_Order_Book benchmark::benchmark)

# 6. Build and register the correctness-test executable.
include(CTest)
if(BUILD_TESTING)
    add_executable(orderbook_tests test.cpp)
    target_link_libraries(orderbook_tests PRIVATE Limit_Order_Book)

    # Keep standard assert() checks enabled when the project is built in Release mode.
    if(MSVC)
        target_compile_options(orderbook_tests PRIVATE /UNDEBUG)
    else()
        target_compile_options(orderbook_tests PRIVATE -UNDEBUG)
    endif()

    add_test(NAME orderbook_tests COMMAND orderbook_tests)
endif()

# Statically link MinGW runtime so the .pyd has no libgcc/libstdc++ DLL dependencies
if(MINGW)
    target_link_options(orderbook_wrapper PRIVATE -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
    target_link_options(benchmark_test PRIVATE -static-libgcc -static-libstdc++)
    if(TARGET orderbook_tests)
        target_link_options(orderbook_tests PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()