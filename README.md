# Limit Order Book Library

A C++17 limit order book and matching-engine project for exploring price-time priority, partial fills, market simulation, Python bindings, and performance measurement.

## Features

- Buy and sell orders
- Limit and market order handling
- Price priority through sorted maps
- FIFO priority within each price level
- Exact and partial fills across multiple orders
- Manual, market-maker, noise-trader, trend-follower, and whale agents
- Python bindings through pybind11
- Google Benchmark performance harness

## Project structure

```text
.
|-- Agents.cpp              Automated and manual trading agents
|-- Agents.h
|-- benchmark.cpp           Google Benchmark harness
|-- matching_engine.cpp     Queue draining and order matching
|-- matching_engine.h
|-- order_book.cpp          Console order-book display
|-- order_book.h
|-- python_bindings.cpp     pybind11 module definitions
|-- variables.h             Orders, price levels, and shared market state
|-- CMakeLists.txt
|-- PROJECT_RECAP.md        Detailed design and maintenance notes
`-- README.md
```

## Building

Requirements:

- CMake 3.14 or newer
- A C++17 compiler
- Python development files for the Python module
- Internet access during the first configuration, when CMake downloads pinned pybind11 and Google Benchmark releases

Configure and build an optimized MinGW release on Windows:

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build produces:

- `Limit_Order_Book`: static C++ library
- `orderbook_wrapper...pyd`: Python extension module
- `benchmark_test.exe`: standalone benchmark executable

## Python usage

Run from the build directory or add it to `sys.path`:

```python
import orderbook_wrapper as ob

market = ob.GlobalVariables()
maker = ob.MarketMaker(market)
noise = ob.NoiseTrader(market)
book = ob.Order_Book()

maker.execute()
noise.loop(100)
book.printer(market)
```

## Benchmark methodology

`benchmark.cpp` measures complete agent execution, including order generation, queue insertion, locking, and matching-engine processing. Every measured batch starts from a freshly rebuilt market with a fixed RNG seed. Book construction and cleanup occur while Google Benchmark timing is paused.

Benchmark names contain two arguments:

```text
Agent_Test_Fixed_Book/<setup submissions>/<timed agent executions>
```

For example:

```text
Noise_Trader_Test_Fixed_Book/100/1000
```

This performs 100 deterministic noise-trader setup submissions, then measures a batch of 1,000 noise-trader executions.

Important interpretation details:

- The first argument counts setup submissions, not guaranteed resting orders. Compatible setup orders may match.
- The second argument counts agent executions in the timed batch.
- One market-maker execution submits two orders: one bid and one ask.
- Google Benchmark reports time per batch. Divide by the second argument for time per agent execution, accounting for the market maker's two orders when calculating per-order throughput.
- The book is identical at the beginning of every measured batch but evolves during that batch.

Run repeated measurements:

```powershell
.\build\benchmark_test.exe --benchmark_repetitions=5 --benchmark_report_aggregates_only=true
```

Latency results are meaningful only when compared using the same compiler optimization, hardware, operating-system conditions, and benchmark methodology.

## Current limitations

- No cancel or modify-order support
- No fill/cancellation report for market-order remainders
- No automated correctness-test target yet
- `timestamp` and `traded` are not fully implemented
- Concurrent agents access some shared state outside `market_mutex`; treat the current implementation as primarily single-threaded
- The Python STL bindings expose converted containers, so in-place Python mutations may operate on copies

## Purpose

This is a learning project for market microstructure, exchange matching, C++ data structures, concurrency, Python interoperability, and responsible performance benchmarking. It is not intended to represent a production exchange.
