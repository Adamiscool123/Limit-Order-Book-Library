# API reference

The library is used either from C++ by linking the `Limit_Order_Book` static library,
or from Python through the `orderbook_wrapper` extension module. Both expose the same
classes.

## C++ API

Include the headers from the repository root and link `Limit_Order_Book`:

```cmake
target_link_libraries(my_app PRIVATE Limit_Order_Book)
```

### Types (`variables.h`)

| Type | Members |
|---|---|
| `enum class Side` | `Buy`, `Sell` |
| `enum class OrderType` | `Limit`, `Market` |
| `struct Order` | `side`, `price`, `shares`, `order_type`, `order_id` |
| `struct PriceLevel` | `total_shares`, `orders` (`std::list<Order>`, oldest first) |
| `class Global_Variables` | `TradingQueue`, `buyMap`, `sellMap`, `price_history`, `market_mutex`, `starting_price`, `count`, `seed`, `rng` |

See [architecture.md](architecture.md#market-state) for the meaning of each
`Global_Variables` member.

### `Matching_Engine` (`matching_engine.h`)

| Method | Description |
|---|---|
| `void checker(Global_Variables &m)` | Lock the market, then match every queued order and rest any limit remainder. Does nothing if the queue is empty. |
| `void buy(Order &o, Global_Variables &m, bool &stop)` | One matching step for a buy order against the best ask. Sets `stop` when no ask is acceptable. Called by `checker`; the caller must hold the lock. |
| `void sell(Order &o, Global_Variables &m, bool &stop)` | Mirror of `buy` against the best bid. |

### `Order_Book` (`order_book.h`)

| Method | Description |
|---|---|
| `void printer(Global_Variables &m)` | Clear the terminal and print asks then bids. Prints nothing if both sides are empty. |
| `void sell(Global_Variables &m)` | Print the header and the ask side. |
| `void buy(Global_Variables &m)` | Print the bid side. |

### Agents (`Agents.h`)

| Class | Notes |
|---|---|
| `Agent_Base` | Abstract. Provides `execute()`, `loop(int n)`, `infinite_loop(int ms)`; subclasses override `execute_agent()` |
| `manual` | `trade(int price, int shares, Side side, OrderType type, Global_Variables &m)` queues one order and matches it immediately |
| `market_maker`, `noise_trader`, `trend_follower`, `whale` | Automated agents, see [agents.md](agents.md) |

### Example

```cpp
#include "Agents.h"
#include "order_book.h"

int main() {
    Global_Variables market;
    market.rng.seed(42);                 // reproducible run

    market_maker maker(market);
    noise_trader noise(market);
    manual me(market);

    maker.loop(50);
    noise.loop(200);
    me.trade(0, 25, Side::Buy, OrderType::Market, market);

    Order_Book().printer(market);
}
```

## Python API

Build the project (see the [README](../README.md#quick-start)), then import the module
from the build directory:

```python
import sys
sys.path.insert(0, "build")
import orderbook_wrapper as ob
```

### Classes

| Python name | C++ type | Members |
|---|---|---|
| `Side` | `Side` | `Side.Buy`, `Side.Sell` |
| `OrderType` | `OrderType` | `OrderType.Limit`, `OrderType.Market` |
| `Order` | `Order` | `side`, `price`, `shares`, `order_type`, `order_id` |
| `PriceLevel` | `PriceLevel` | `total_shares`, `orders` |
| `GlobalVariables` | `Global_Variables` | `starting_price`, `price_history`, `count`, `buyMap`, `sellMap` |
| `Matching_Engine` | `Matching_Engine` | `checker(market)`, `buy(...)`, `sell(...)` |
| `Order_Book` | `Order_Book` | `printer(market)`, `sell(market)`, `buy(market)` |
| `AgentBase` | `Agent_Base` | `execute()`, `loop(n)`, `infinite_loop(ms)` |
| `Manual` | `manual` | `trade(price, shares, side, order_type, market)`, `execute_agent()` |
| `MarketMaker`, `NoiseTrader`, `TrendFollower`, `Whale` | agents | `execute_agent()` plus the `AgentBase` methods |

All agent constructors take a `GlobalVariables` instance and keep it alive for as long
as the agent exists.

### Reading the book

`buyMap` and `sellMap` come back as Python `dict`s from price to `PriceLevel`, ordered
like their C++ maps (best price first):

```python
best_bid = next(iter(market.buyMap), None)
best_ask = next(iter(market.sellMap), None)
spread = best_ask - best_bid if best_bid is not None and best_ask is not None else None

for price, level in market.sellMap.items():
    print(price, level.total_shares, [o.order_id for o in level.orders])
```

### Copy semantics

`price_history`, `buyMap`, `sellMap`, and `PriceLevel.orders` are converted to Python
containers each time they are accessed. The result is a **copy**:

```python
market.price_history.append(123)       # modifies a temporary copy; no effect
market.price_history = [100, 101]      # assignment replaces the C++ vector; works
```

Use `Manual.trade` or the agents to change the book. Do not edit the copies.

### Notes

- `seed` and `rng` are not exposed to Python, so Python simulations are seeded from the
  clock and are not reproducible.
- `infinite_loop` never returns and holds the interpreter, so it is not useful from Python.
- `Order_Book.printer` writes ANSI escape codes to clear the terminal.
