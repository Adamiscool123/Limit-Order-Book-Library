# Architecture

This page describes the data structures, the matching algorithm, and the concurrency
model of the library. For the behaviour of the trading agents see [agents.md](agents.md);
for the public interface see [api.md](api.md).

## Components

| Component | File | Responsibility |
|---|---|---|
| `Order`, `PriceLevel`, `Side`, `OrderType` | [variables.h](../variables.h) | Plain data types |
| `Global_Variables` | [variables.h](../variables.h) | The shared market state: inbound queue, both sides of the book, trade history, RNG, mutex |
| `Matching_Engine` | [matching_engine.cpp](../matching_engine.cpp) | Drains the inbound queue and matches orders |
| `Order_Book` | [order_book.cpp](../order_book.cpp) | Renders the book to the console (it holds no data) |
| `Agent_Base` and subclasses | [Agents.cpp](../Agents.cpp) | Generate orders and trigger matching |
| `orderbook_wrapper` | [python_bindings.cpp](../python_bindings.cpp) | Python module exposing all of the above |

Everything operates on a single `Global_Variables` instance passed by reference, so
several independent markets can exist side by side (the tests and benchmarks create a
fresh one per scenario).

## Data model

```cpp
enum class Side      { Buy, Sell };
enum class OrderType { Limit, Market };

struct Order {
    Side side;
    int price;            // ignored for market orders
    int shares;           // remaining quantity, reduced as the order fills
    OrderType order_type;
    int order_id;         // taken from Global_Variables::count
};

struct PriceLevel {
    int total_shares = 0;     // sum of shares resting at this price
    std::list<Order> orders;  // FIFO queue: front = oldest
};
```

`Side` and `OrderType` are scoped enums, so an invalid side or order type cannot be
constructed in C++ or passed from Python.

### Market state

| Member | Type | Purpose |
|---|---|---|
| `TradingQueue` | `std::queue<Order>` | Orders waiting to be matched |
| `buyMap` | `std::map<int, PriceLevel, std::greater<int>>` | Bids, highest price first |
| `sellMap` | `std::map<int, PriceLevel>` | Asks, lowest price first |
| `price_history` | `std::vector<int>` | Execution price of every fill, in order |
| `market_mutex` | `std::mutex` | Guards the queue and the book |
| `starting_price` | `int` (100) | Reference price before the first trade |
| `count` | `int` | Next order ID |
| `seed`, `rng` | `unsigned`, `std::mt19937` | Randomness for agents; seeded from the clock by default |

Because both maps are ordered so that the most aggressive price comes first, the best
bid is `buyMap.begin()` and the best ask is `sellMap.begin()` without any scanning.

## Matching algorithm

`Matching_Engine::checker` takes the mutex once and processes every queued order:

```text
lock market_mutex
while TradingQueue is not empty:
    order = pop front of TradingQueue
    while order.shares > 0:
        if order is Buy:
            best = sellMap.begin()
            stop if sellMap is empty, or (order is Limit and order.price < best price)
        else:
            best = buyMap.begin()
            stop if buyMap is empty, or (order is Limit and order.price > best price)

        maker = oldest order at best
        filled = min(order.shares, maker.shares)
        append best price to price_history
        reduce order, maker and the level's total_shares by filled
        if maker is fully filled: remove it; erase the level if it is now empty

    if order.shares > 0 and order is Limit:
        append order to the back of its price level on its own side
    (a Market order's remainder is discarded)
```

The code implements the `min` as three explicit cases (incoming larger, equal, or
smaller than the resting order), which keeps each branch easy to follow and test.

### Properties

- **Price priority.** An incoming order always trades with the best opposite price first,
  because the maps keep that price at `begin()`.
- **Time priority.** Within a price, the order that arrived first is at the front of the
  list and fills first. New orders are appended at the back. No timestamps are needed.
- **Execution price.** Fills happen at the resting (maker) order's price, so an aggressive
  limit order can receive price improvement.
- **No empty levels.** A level is erased as soon as its last order fills, so `begin()` is
  always a real, tradeable price.
- **One history entry per fill.** An order that sweeps three resting orders adds three
  entries to `price_history`.

### Complexity

With L price levels on a side and k fills produced by an incoming order:

| Operation | Cost |
|---|---|
| Find best bid / ask | O(1) |
| Each fill | O(1) amortized (list `pop_front`, map erase by iterator) |
| Rest a limit remainder | O(log L) map lookup plus O(1) list append |
| Whole incoming order | O(k + log L) |
| Cancel by order ID | O(N) linear search over resting orders, then O(1) list erase |

`std::map` and `std::list` allocate one node per level and per order, so memory
allocation is a meaningful share of the cost. A production engine would typically use
an array indexed by price tick and a pooled intrusive list instead.

## Concurrency model

Orders are pushed onto `TradingQueue` while holding `market_mutex`, and `checker` holds
the same mutex for the whole time it drains the queue and mutates the book. This makes
the queue and the book safe to modify from several threads.

The design is not yet fully thread-safe: agents read `price_history`, draw from `rng`,
and increment `count` without holding the lock. Running several agents concurrently
through `Agent_Base::infinite_loop` is therefore a data race. Treat the library as
single-threaded until those accesses are moved under the lock.

## Rendering

`Order_Book::printer` clears the terminal with an ANSI escape sequence and prints asks
then bids, one line per price level with the total shares at that level. It recomputes
each total from the orders in the level rather than trusting `PriceLevel::total_shares`.

## Python bindings

The module exposes every class and both enums. Agent constructors use
`py::keep_alive<1, 2>` so the `GlobalVariables` object cannot be garbage-collected while
an agent still refers to it. `price_history`, `buyMap`, and `sellMap` are converted to
Python containers on access, which produces copies (see [api.md](api.md#python-api)).
