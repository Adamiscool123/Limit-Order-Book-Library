# Project Recap — Limit Order Book Library

> **Purpose of this file:** Read this when you come back to the project after a long break.
> It explains what the project is, how every file fits together, how the code actually works,
> how to build and use it, and what the known quirks / unfinished parts are.
>
> Last updated: 2026-10-14
>
> Public-facing documentation lives in `README.md` and `docs/`.

---

## 1. What is this project?

A **C++ limit order book + matching engine** that simulates how a stock exchange works,
with **automated trading agents** (bots) that generate order flow, and **Python bindings**
(via pybind11) so the whole thing can be driven from Python.

You built it to learn:
- Market microstructure (bid/ask books, spreads, price discovery)
- How exchange matching engines work (FIFO priority, partial fills)
- C++ systems programming (maps, lists, threads, mutexes)
- Latency benchmarking

There is **no `main.cpp`** — the project builds as a **library** (`Limit_Order_Book`) plus a
**Python module** (`orderbook_wrapper`). You use it from Python, not by running an .exe.

---

## 2. File-by-file map

| File | What it contains |
|---|---|
| `variables.h` | Core data structures: `Order`, `PriceLevel`, and `Global_Variables` (the shared market state everything operates on) |
| `matching_engine.h/.cpp` | `Matching_Engine` class — pulls orders off the queue and matches buyers with sellers |
| `order_book.h/.cpp` | `Order_Book` class — just **printing/display** of the book to the console (despite the name, it holds no data) |
| `Agents.h/.cpp` | `Agent_Base` and the 5 agent types (manual, market maker, noise trader, trend follower, whale) |
| `python_bindings.cpp` | pybind11 wrapper exposing everything to Python as module `orderbook_wrapper` |
| `benchmark.cpp` | Google Benchmark performance harness |
| `test.cpp` | Deterministic matching-engine correctness tests |
| `CMakeLists.txt` | Builds the library, Python module, benchmark, and tests; downloads pinned dependencies |
| `README.md` | Public-facing description + benchmark results table |
| `docs/` | Architecture, agents, API reference, testing, and benchmarking guides |
| `build/` | CMake build output (generated; contains the compiled `.pyd`) |

---

## 3. Core data structures (`variables.h`)

```cpp
enum class Side { Buy, Sell };
enum class OrderType { Limit, Market };

struct Order {
    Side side;              // Side::Buy or Side::Sell
    int price;              // integer price (whole dollars)
    int shares;             // remaining quantity
    OrderType order_type;   // OrderType::Limit or OrderType::Market
    int order_id;     // taken from Global_Variables::count, then count++
};

struct PriceLevel {
    int total_shares;          // running total at this price
    std::list<Order> orders;   // FIFO (First in First out) queue -> time priority for equal prices
};
```

**`Global_Variables`** is the single shared "market" object passed by reference everywhere:

- `TradingQueue` — `std::queue<Order>`: all new orders land here first
- `buyMap` — `std::map<int, PriceLevel, std::greater<int>>`: bids, **sorted highest price first** (best bid = `begin()`)
- `sellMap` — `std::map<int, PriceLevel>`: asks, **sorted lowest price first** (best ask = `begin()`)
- `price_history` — `std::vector<int>`: every trade's execution price gets pushed here; agents read `.back()` as "last price"
- `market_mutex` — protects the queue/book because agents can run on threads
- `starting_price = 100` — where the market begins before any trades
- `count` — global counter used to hand out order IDs
- `rng` — `std::mt19937` seeded from the system clock; all agents draw randomness from it

**Side and order type are `enum class` values, not plain ints.** Write `Side::Buy` /
`Side::Sell` and `OrderType::Limit` / `OrderType::Market`. The compiler rejects plain
numbers like `0` or `1`, so an invalid side or order type can't be created, and
`trade()` no longer needs to validate them at runtime.

---

## 4. How an order flows through the system

1. An agent (or `manual.trade(...)`) creates an `Order`, locks `market_mutex`, and pushes it onto `TradingQueue`.
2. `Matching_Engine::checker(m)` is called (agents call it automatically after each `execute_agent()`).
3. `checker`:
   - Locks the mutex and drains the queued orders one at a time.
   - Loops while the order still has shares:
     - **Buy order:** a market order, or a limit order where `trader.price >= best ask` (front of `sellMap`), trades against the oldest order at the best ask. Three cases: taker bigger (pop maker, keep looping), equal (both done), taker smaller (reduce maker, done). The execution price (the maker's price) is pushed to `price_history` on every fill.
     - **Sell order:** mirror image against the best bid (front of `buyMap`); market orders ignore their stored price.
     - If the order can't cross (no counterparty or price not aggressive enough), the loop breaks.
   - Any **leftover shares rest in the book**: appended to the back of the `PriceLevel::orders` list at that price (this is what gives FIFO / time priority).
4. Empty price levels are erased from the map so `begin()` is always the true best bid/ask.

So: **price priority** comes from the sorted maps, **time priority** comes from the FIFO list
inside each price level, and **partial fills** are handled by the three-case share comparison.

---

## 5. The agents (`Agents.cpp`)

All agents inherit from `Agent_Base`, which stores a reference to the shared `Global_Variables`
and provides three ways to run:

- `execute()` — run the agent's logic **once**, then run the matching engine
- `loop(int times)` — run it N times
- `infinite_loop(int ms)` — run forever with a sleep between iterations (meant for its own thread)

Each subclass overrides `execute_agent()`:

| Agent | Behavior |
|---|---|
| **manual** | `execute_agent()` does nothing. Instead call `trade(price, shares, side, order_type, m)` directly to place a specific order (this is you trading by hand). |
| **market_maker** | Places a **pair** of orders every run: a buy at `last_price - 1` and a sell at `last_price + 1` (i.e. quotes around the last trade), 1–10 shares each. Before any trade exists it quotes randomly around `starting_price` (buys 95–101, sells 99–105). |
| **noise_trader** | Random side, 1–10 shares. Buys in `[0.8 × last, last + 2]`, sells in `[last − 2, 1.2 × last]`, so some orders cross the spread and move the price. Before any trade: buys 95–101, sells 99–105. |
| **trend_follower** | Needs ≥5 prices in history, otherwise does nothing. Compares last price vs `price_history[size − 5]`: price up → **buys** (last−2..last), price down → **sells** (last..last+2), flat → does nothing. 10–30 shares. Momentum trader. |
| **whale** | Random side, **100–200 shares**. 50/50 limit or market: limit buys in last−10..last, limit sells in last..last+10; market orders take the best opposite prices and discard any unfilled remainder. |

The whale's market orders are the only automated market orders; every other agent sends
limit orders (`OrderType::Limit`). All of them lock the mutex before pushing to the queue.

---

## 6. The printer (`order_book.cpp`)

`Order_Book::printer(m)` clears the terminal (ANSI escape `\033[2J\033[1;1H`) and prints:

```
       Order Book
SELL:   $105 | 23      <- asks, lowest at... (iterates sellMap ascending)
--------------------------------------
BUY:   $99  | 14       <- bids, highest first
```

It recomputes totals by summing each level's order list (it doesn't trust `total_shares`)
and skips empty levels.

---

## 7. Python bindings (`python_bindings.cpp`)

Module name: **`orderbook_wrapper`** (the compiled file is `orderbook_wrapper.*.pyd` in `build/`).

Exposed classes: `Order`, `GlobalVariables`, `Matching_Engine`, `Order_Book`, `AgentBase`,
`Manual`, `MarketMaker`, `NoiseTrader`, `TrendFollower`, `Whale`, `PriceLevel`.

Exposed enums: `Side` (`Side.Buy`, `Side.Sell`) and `OrderType` (`OrderType.Limit`,
`OrderType.Market`), registered with `py::enum_`. Without that registration Python can't
pass or read these values, so `trade()` and `Order.side` wouldn't work from Python.

Agent constructors take a `GlobalVariables` and use `py::keep_alive` so Python won't
garbage-collect the market state while an agent still references it.

Typical usage from Python (run from the `build/` folder or add it to `sys.path`):

```python
import orderbook_wrapper as ob

m = ob.GlobalVariables()
book = ob.Order_Book()

mm = ob.MarketMaker(m)
noise = ob.NoiseTrader(m)
whale = ob.Whale(m)
trend = ob.TrendFollower(m)
me = ob.Manual(m)

# Seed the market and generate flow
mm.loop(50)
noise.loop(100)
whale.execute()
trend.execute()

# Place your own order: limit buy, 10 shares at $100
me.trade(100, 10, ob.Side.Buy, ob.OrderType.Limit, m)

book.printer(m)          # print the book
print(m.price_history)   # trade prices over time
```

---

## 8. Building the project

Requirements: CMake ≥ 3.14, a C++17 compiler (you use **MinGW** on Windows), internet
access the first time (CMake `FetchContent` downloads **pybind11 v3.0.1** and
**Google Benchmark v1.9.5**).

```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Outputs:
- `Limit_Order_Book` — static library with the C++ logic
- `orderbook_wrapper...pyd` — the Python module (in `build/`)
- `benchmark_test.exe` — standalone Google Benchmark executable (in `build/`)
- `orderbook_tests.exe` — correctness-test executable (in `build/`)

On MinGW the runtime (libgcc/libstdc++/winpthread) is **statically linked** into the `.pyd`
so it works without extra DLLs (see the `if(MINGW)` block in `CMakeLists.txt` — commit `48d61eb`).

---

## 9. Correctness tests

`test.cpp` covers empty-book insertion, crossing and non-crossing limit orders,
exact and partial fills, multiple price levels, FIFO priority, and unfilled
market-order remainders in both directions. CTest runs it with:

```powershell
cmake --build build --target orderbook_tests
ctest --test-dir build --output-on-failure
```

The CMake target explicitly keeps standard `assert()` checks active in Release
builds.

---

## 10. Benchmarks

`benchmark.cpp` uses Google Benchmark and builds as `benchmark_test.exe`. Six benchmarks:

- `Matching_Engine_Test_Fixed_Book` — engine only: a pre-generated, seeded order stream is
  pushed and matched, so no RNG or agent logic is timed. This is the headline number.
- `Mixed_Market_Test_Fixed_Book` — market maker, noise trader, trend follower, and whale
  take turns in one market (4 agent executions per round).
- One benchmark per agent type on its own (`Noise_Trader_...`, `Whale_...`, etc.).

Every iteration rebuilds a seeded market while the timer is paused. Names are
`<Benchmark>/<setup>/<timed items>`; the grid is setup {10, 100, 1000, 10000} × batch
{1000, 10000}. Settings live in `fixed_book_args()` (1 iteration per repetition,
20 repetitions, wall-clock time, microseconds), so just run:

```powershell
.\build\benchmark_test.exe
.\build\benchmark_test.exe --benchmark_filter=Book/1000/10000   # headline cases only
```

Each result reports `fills` and `resting` counters so you can see what a batch actually
did. On their own, the market maker never trades and the trend follower (setup ≤ 1000)
submits nothing; that's expected, and the mixed benchmark covers them. CMake now
defaults to Release when no build type is given. Results and caveats: `docs/benchmarking.md`.

---

## 11. Known quirks / unfinished things (honest list)

These are things to be aware of before extending the code:

1. **Market-order reporting is minimal.** Market orders consume available opposite-side
   liquidity and discard any unfilled remainder, but there is no fill/cancellation report.
2. **There are no timestamps.** FIFO priority comes from list insertion order, so
   `Order::timestamp` and `Order::traded` were removed.
3. **Order IDs can have gaps.** `trend_follower` takes an ID from `count` before deciding
   whether to submit anything.
4. **`infinite_loop()` never returns**, and from Python it blocks the interpreter.
5. **Concurrent agent execution is not fully synchronized.** Queue and matching operations
   use `market_mutex`, but agents access `count`, `rng`, and `price_history` outside the lock.
6. **Single-agent benchmarks can be degenerate.** On its own the market maker never trades and
   the trend follower (setup ≤ 1000) submits nothing — use the engine and mixed-market
   benchmarks for headline numbers (see `docs/benchmarking.md`).
7. **`PriceLevel::total_shares`** is maintained by the engine but the printer recomputes
   totals by iterating orders — the two could drift; the printer's sum is the truth.
8. **Cancel is a linear search.** `Agent_Base::cancel(id)` scans every resting order;
   an `order_id` → list-iterator index would make it O(1). No modify-order support.
9. **Python STL properties are copied by automatic conversion.** In-place mutations such
    as `m.price_history.append(...)` do not update the underlying C++ container.

---

## 12. Ideas that were on the table for "next steps"

- Expand tests to cover invalid inputs and deterministic agent behavior
- O(1) cancellation via an `order_id` → list-iterator index
- Execution reports (fills and discarded market-order remainders)
- Multi-threaded simulation using `infinite_loop()` with one thread per agent
- A Python script that runs a full simulation and plots `price_history` (matplotlib)

---

## 13. Quick mental model (TL;DR)

> Agents throw orders into a mutex-protected queue → the matching engine pops one at a
> time and crosses it against the best opposite price level (sorted `std::map`s, FIFO
> lists per price) → fills push prices into `price_history` → leftovers rest in the book →
> agents look at `price_history.back()` to decide their next order → Python drives it all
> through `orderbook_wrapper`.
