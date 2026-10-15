# Testing

The correctness suite in [test.cpp](../test.cpp) drives the engine through
`manual::trade` and checks the resulting book, queue, and trade history with `assert`.
Every scenario runs on a fresh `Global_Variables`, so the tests are independent and
deterministic.

## Running

```powershell
cmake --build build --target orderbook_tests
ctest --test-dir build --output-on-failure
```

The `orderbook_tests` target compiles with `-UNDEBUG` (`/UNDEBUG` on MSVC), so the
assertions stay active even in a Release build. A failed assertion aborts the program
and CTest reports the test as failed.

## Scenarios

| Test | Orders submitted | Verifies |
|---|---|---|
| `empty_book` | Buy 5 @ 100 | A non-crossing order rests as a single order in a new bid level |
| `non_crossing_limit_book` | Buy 5 @ 99, Sell 7 @ 101 | Orders that do not cross both rest; no trades |
| `crossing_limit_book` | Buy 5 @ 100, Sell 5 @ 99 | Equal sizes fill completely at the resting price (100); book empties |
| `trader_shares_bigger_than_resting_shares` | Sell 4 @ 100, Buy 10 @ 101 | Resting ask fully filled; the buyer's remaining 6 rest at 101 |
| `multiple_price_levels` | Sells 3 @ 100, 4 @ 101, 10 @ 103; Buy 10 @ 102 | Sweeps the two acceptable levels in price order, stops at 103, rests 3 @ 102 |
| `FIFO_Priority` | Sells 3 @ 100 then 7 @ 100; Buy 5 @ 100 | The older order fills first; the newer order is left with 5 shares |
| `market_order_remainder` | Sell 4 @ 100, Market buy 10 | Market buy takes the available 4; the unfilled 6 are discarded, not rested |
| `incoming_seller_smaller_than_resting_buyer` | Buy 10 @ 100, Sell 4 @ 99 | Partial fill of the resting bid, which keeps its place with 6 shares |
| `market_sell_remainder` | Buy 4 @ 100, Market sell 10 | Mirror of `market_order_remainder` for the sell side |

Together these cover the three fill cases (incoming larger, equal, smaller) on both
sides, price priority across levels, time priority within a level, and the
different handling of limit and market remainders.

## Not yet covered

- Input validation in `manual::trade` (zero/negative shares, negative price)
- Market orders against an empty book
- Agent behaviour with a fixed seed (for example, that a seeded simulation reproduces
  the same `price_history`)
- Consistency checks between `PriceLevel::total_shares` and the sum of its orders after
  long random simulations
