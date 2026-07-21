# Project Recap — Limit Order Book Library

> **Purpose of this file:** Read this when you come back to the project after a long break.
> It explains what the project is, how every file fits together, how the code actually works,
> how to build and use it, and what the known quirks / unfinished parts are.
>
> Last updated: 2026-07-21

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
| `CMakeLists.txt` | Builds the static library + the Python module; auto-downloads pybind11 v3.0.1 |
| `README.md` | Public-facing description + benchmark results table |
| `build/` | CMake build output (generated; contains the compiled `.pyd`) |

---

## 3. Core data structures (`variables.h`)

```cpp
struct Order {
    int side;         // 0 = BUY, 1 = SELL
    int price;        // integer price (whole dollars)
    int shares;       // remaining quantity
    int order_type;   // 0 = limit, 1 = market  (currently always 0 — market orders not implemented)
    int order_id;     // taken from Global_Variables::count, then count++
    long long timestamp;
    bool traded = false;   // currently never set anywhere
};

struct PriceLevel {
    int total_shares;          // running total at this price
    std::list<Order> orders;   // FIFO queue -> time priority for equal prices
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

**Key convention used everywhere (documented at top of `Agents.cpp`):**
`side: 0 = buy, 1 = sell` • `order_type: 0 = limit, 1 = market`

---

## 4. How an order flows through the system

1. An agent (or `manual.trade(...)`) creates an `Order`, locks `market_mutex`, and pushes it onto `TradingQueue`.
2. `Matching_Engine::checker(m)` is called (agents call it automatically after each `execute_agent()`).
3. `checker`:
   - Locks the mutex, pops **one** order off the queue.
   - Loops while the order still has shares:
     - **Buy order:** if `trader.price >= best ask` (front of `sellMap`), it trades against the oldest order at the best ask. Three cases: taker bigger (pop maker, keep looping), equal (both done), taker smaller (reduce maker, done). The execution price (the maker's price) is pushed to `price_history` on every fill.
     - **Sell order:** mirror image against the best bid (front of `buyMap`).
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
| **manual** | `execute_agent()` does nothing. Instead call `trade(price, shares, buy_sell, limit_market, m)` directly to place a specific order (this is you trading by hand). |
| **market_maker** | Places a **pair** of orders every run: a buy at `last_price - 1` and a sell at `last_price + 1` (i.e. quotes around the last trade), 1–10 shares each. Before any trade exists it quotes randomly around `starting_price` (buys 95–101, sells 99–105). |
| **noise_trader** | Random side, random price within **±20% of last price** (buys below last, sells above), 1–10 shares. Provides random background flow. |
| **trend_follower** | Needs ≥5 prices in history, otherwise does nothing. Compares last price vs 5 trades ago: price up → **buys** near last price (last−2..last), price down → **sells** (last..last+2). Big size: 100–200 shares. Momentum trader. |
| **whale** | Random side, price within ±10 dollars of last, **100–200 shares** — a large player that moves the market. |

All agent orders are limit orders (`order_type = 0`). All of them lock the mutex before
pushing to the queue.

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

# Place your own order: price=100, 10 shares, buy(0), limit(0)
me.trade(100, 10, 0, 0, m)

book.printer(m)          # print the book
print(m.price_history)   # trade prices over time
```

---

## 8. Building the project

Requirements: CMake ≥ 3.10, a C++17 compiler (you use **MinGW** on Windows), internet
access the first time (CMake `FetchContent` downloads **pybind11 v3.0.1**, chosen for
Python 3.14 support).

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Outputs:
- `Limit_Order_Book` — static library with the C++ logic
- `orderbook_wrapper...pyd` — the Python module (in `build/`)

On MinGW the runtime (libgcc/libstdc++/winpthread) is **statically linked** into the `.pyd`
so it works without extra DLLs (see the `if(MINGW)` block in `CMakeLists.txt` — commit `48d61eb`).

---

## 9. Benchmarks (from README)

You wrote a benchmarking harness that measured per-order latency (ns) in three scenarios
and exported `existing.csv` / `new.csv` / `cross.csv`. Results: **~200–350 ns typical**
per order (median 200–300 ns), with rare outliers up to ~170 µs from OS noise.

⚠️ **Note:** the benchmark harness source is **not in the repo right now** — the README
describes it but there's no benchmark `.cpp` file committed. If you want to re-run the
numbers you'll need to rewrite the harness (submit orders in a loop, time with
`std::chrono::high_resolution_clock`, write CSV).

---

## 10. Known quirks / unfinished things (honest list)

These are things to be aware of before extending the code:

1. **Market orders don't exist yet.** `order_type` is stored but the matching engine never
   checks it — everything behaves as a limit order.
2. **`Order::timestamp` is meaningless.** Every agent does `start = now(); end = now();`
   back-to-back, so timestamp is ~0. FIFO ordering works anyway (it comes from list order,
   not the timestamp), but the field is effectively dead.
3. **`Order::traded` is never set** anywhere.
4. **`Matching_Engine::break_loop` member is unused** — a local `break_loop` in `checker()`
   is what's actually used and passed by reference.
5. **`checker()` processes only ONE order per call.** Agents call it after each
   `execute_agent()`, so this works out, but note the market maker pushes 2 orders and the
   subsequent `checker` call only matches one; the second gets processed on the next cycle.
6. **Dead code in `trend_follower`:** the `price_history.empty()` branches can never run
   because the function returns early when history has < 5 entries.
7. **`PriceLevel::total_shares`** is maintained by the engine but the printer recomputes
   totals by iterating orders — the two could drift; the printer's sum is the truth.
8. **`variables.h` include guard typo:** `VARIBLES_H` (harmless, just misspelled).
9. **No cancel/modify order support** — orders can only rest or fill.
10. **No tests** — verification has been manual (run agents, print book, eyeball it).
11. Recent fix (`aeb7cc4`, latest commit): agent price distributions were corrected so
    buys/sells land on the proper side of the last price (the ranges in section 5 reflect
    the fixed behavior).

---

## 11. Ideas that were on the table for "next steps"

- Re-add / rewrite the latency benchmark harness (README section 9)
- Implement real market orders (honor `order_type == 1`: match at any price)
- Order cancellation + order lookup by `order_id`
- Set `traded` flag and give trades real timestamps
- Multi-threaded simulation using `infinite_loop()` with one thread per agent
- A Python script that runs a full simulation and plots `price_history` (matplotlib)

---

## 12. Quick mental model (TL;DR)

> Agents throw orders into a mutex-protected queue → the matching engine pops one at a
> time and crosses it against the best opposite price level (sorted `std::map`s, FIFO
> lists per price) → fills push prices into `price_history` → leftovers rest in the book →
> agents look at `price_history.back()` to decide their next order → Python drives it all
> through `orderbook_wrapper`.
