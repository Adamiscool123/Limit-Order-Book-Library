# Trading agents

Agents generate order flow for the simulation. Each agent holds a reference to one
`Global_Variables` market and decides what to submit from the **last traded price**,
`price_history.back()`. Before any trade has happened, agents quote around
`starting_price` (100).

All randomness comes from the market's `rng`, so seeding it makes a simulation fully
reproducible:

```cpp
Global_Variables market;
market.rng.seed(12345);
```

## Running an agent

Every agent derives from `Agent_Base`, which provides three drivers. Each one calls the
agent's `execute_agent()` (which queues zero, one, or two orders) and then
`Matching_Engine::checker` to match them.

| Method | Behaviour |
|---|---|
| `execute()` | Run once |
| `loop(n)` | Run `n` times |
| `infinite_loop(ms)` | Run forever, sleeping `ms` milliseconds between runs. It never returns, so call it on a dedicated thread. From Python it blocks the interpreter. |

## Agent summary

| Agent | Python class | Orders per run | Side | Price | Size |
|---|---|---|---|---|---|
| Manual | `Manual` | 0 (use `trade`) | You choose | You choose | You choose |
| Market maker | `MarketMaker` | 2 | One bid and one ask | last − 1 / last + 1 | 1–10 each |
| Noise trader | `NoiseTrader` | 1 | Random | Wide random band | 1–10 |
| Trend follower | `TrendFollower` | 0 or 1 | With the trend | Near last | 10–30 |
| Whale | `Whale` | 1 | Random | Limit near last, or market | 100–200 |

## Manual

`execute_agent()` does nothing. Place orders directly instead:

```cpp
manual me(market);
me.trade(100, 10, Side::Buy, OrderType::Limit, market);
```

`trade` rejects orders with zero or negative shares, or a negative price, by printing
`Invalid` and returning. Otherwise it queues the order and runs the matching engine
immediately. The price of a market order is ignored.

## Market maker

Provides liquidity on both sides by quoting one tick around the last trade.

- **Bid:** `last − 1`, 1–10 shares
- **Ask:** `last + 1`, 1–10 shares
- **Before the first trade:** bid uniformly in 95–99, ask uniformly in 101–105

Its own quotes never cross each other. It only trades when another agent's order
reaches its prices.

## Noise trader

Random background flow that makes the price move.

- Side: buy or sell with equal probability; 1–10 shares
- **Buy price:** uniform in `[0.8 × last, last + 2]`
- **Sell price:** uniform in `[last − 2, 1.2 × last]`
- **Before the first trade:** buy in 95–101, sell in 99–105

Because each band reaches 2 ticks past the last price, some noise orders cross the
spread and trade, and that is what moves the price.

## Trend follower

A momentum trader that compares the last price with the price 4 trades earlier
(`price_history[size − 5]`).

| Condition | Action |
|---|---|
| Fewer than 5 trades in history | Submit nothing |
| Last price higher | Buy 10–30 shares at a price in `[last − 2, last]` |
| Last price lower | Sell 10–30 shares at a price in `[last, last + 2]` |
| Prices equal | Submit nothing |

The flat-price rule stops it from pushing the price in either direction when there is
no trend.

## Whale

A large participant that can move the market on its own.

- Side: buy or sell with equal probability; 100–200 shares
- Order type: limit or market with equal probability
- **Limit buy:** price in `[last − 10, last]`
- **Limit sell:** price in `[last, last + 10]`
- **Before the first trade:** limit buy in 95–101, limit sell in 99–105
- **Market order:** takes the best available prices on the opposite side until filled
  or until that side is empty; any remainder is discarded

The market orders matter: the whale's limit orders never bid above or offer below the
last price, so without market orders it would rarely move the price.

## Writing a new agent

Derive from `Agent_Base`, override `execute_agent()`, and push orders while holding the
mutex:

```cpp
class my_agent : public Agent_Base {
public:
    my_agent(Global_Variables &m) : Agent_Base(m) {}

    void execute_agent() override {
        Order o;
        o.side = Side::Buy;
        o.order_type = OrderType::Limit;
        o.price = variable.price_history.empty() ? variable.starting_price
                                                 : variable.price_history.back();
        o.shares = 5;
        o.order_id = variable.count++;

        std::lock_guard<std::mutex> lock(variable.market_mutex);
        variable.TradingQueue.push(o);
    }
};
```

`execute()`, `loop()`, and `infinite_loop()` then work without further changes. To use
the new agent from Python, register it in [python_bindings.cpp](../python_bindings.cpp)
the same way as the existing agents.
