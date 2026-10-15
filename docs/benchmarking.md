# Benchmarking

[benchmark.cpp](../benchmark.cpp) uses [Google Benchmark](https://github.com/google/benchmark)
to measure the matching engine on its own, a mixed market of agents, and each agent type
separately. This page explains what is measured, how to reproduce the results, and what
the numbers do and do not mean.

## Benchmarks

| Benchmark | Timed work | One item = |
|---|---|---|
| `Matching_Engine_Test_Fixed_Book` | Queue insertion and matching of a **pre-generated** order stream. No RNG or agent logic is timed. | one order |
| `Mixed_Market_Test_Fixed_Book` | Market maker, noise trader, trend follower, and whale taking turns in the same market | one agent execution (4 per round) |
| `Noise_Trader_Test_Fixed_Book` | One noise trader on its own | one agent execution |
| `Whale_Test_Fixed_Book` | One whale on its own | one agent execution |
| `Market_Maker_Test_Fixed_Book` | One market maker on its own | one agent execution (2 orders) |
| `Trend_Follower_Test_Fixed_Book` | One trend follower on its own | one agent execution |

The engine benchmark is the closest measure of the library itself. Its order stream is
generated once, before timing starts, from a fixed seed: buy/sell with equal
probability, 1–10 shares, 10% market orders, and limit prices of $95–$101 for buys and
$99–$105 for sells, so the two sides overlap and a large share of orders trade. Each
order follows the same path as `manual::trade`: lock, push onto the queue, run
`Matching_Engine::checker`.

The agent benchmarks time the full agent path: decision logic and random draws, pushing
under the mutex, and matching.

## Method

For every measured iteration:

1. Timing is paused.
2. A new `Global_Variables` market is created with its RNG seeded to `12345`.
3. The book is built from `setup` submissions: noise-trader executions for the agent
   benchmarks, and the first `setup` orders of the stream for the engine benchmark.
4. Timing resumes, the timed batch runs, and timing pauses again.
5. Counters are recorded and the market is destroyed, still outside the timed region.

Every iteration therefore starts from exactly the same book. Benchmark names encode the
two parameters:

```text
<Benchmark>/<setup>/<timed items>/iterations:1/repeats:20/real_time
```

Each benchmark runs over `setup ∈ {10, 100, 1000, 10000}` × `batch ∈ {1000, 10000}`.
The settings are fixed in code (`fixed_book_args`), so running the executable without
flags gives correct results:

| Setting | Reason |
|---|---|
| One batch per repetition (`Iterations(1)`) | A batch already repeats the work 1,000–10,000 times. Letting Google Benchmark calibrate would rebuild the setup market thousands of times. |
| 20 repetitions, aggregates only | Reports mean, median, standard deviation, and coefficient of variation (cv) |
| Wall-clock time (`UseRealTime()`) | The Windows CPU clock ticks roughly every 15.6 ms, longer than a batch, so CPU time reads 0 |
| Batches of at least 1,000 | Pausing and resuming the timer costs around a microsecond and swamped smaller batches (cv 18–50%) |

### Counters

Every result also reports what the batch did, so a fast number can't hide a batch that
never traded:

| Counter | Meaning |
|---|---|
| `items_per_second` | Throughput, in the unit from the table above |
| `fills` | Fills during the timed batch (one per resting order matched) |
| `resting` | Orders left resting in the book at the end of the batch |

## Reproducing

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target benchmark_test
.\build\benchmark_test.exe
```

The project defaults to a Release build when no build type is given. An existing build
folder keeps whatever type it was configured with, so check `CMAKE_BUILD_TYPE` in
`build/CMakeCache.txt` before benchmarking. Google Benchmark prints a warning if its own
library was built in Debug mode.

To run only the headline cases, add `--benchmark_filter=Book/1000/10000`. For
machine-readable output, add `--benchmark_format=json`.

### Getting stable numbers

Laptop CPUs change clock speed with the power mode and move work between cores, and
both effects show up directly in the results. Before recording numbers:

1. Close other programs.
2. Plug in the charger and set Windows power mode to **Best performance**.
3. Pin the benchmark to one core at high priority, so it is not moved between fast and
   power-saving cores mid-run:

```powershell
cmd /c start /b /wait /high /affinity 1 .\build\benchmark_test.exe --benchmark_filter=Book/1000/10000
```

## Results

Release build (`-O3`), MinGW-w64 GCC 16.1.0, Windows 11, AMD Ryzen AI 9 HX 370,
recorded 2026-10-15. Best performance power mode, pinned to one core at high priority.
setup = 1,000, batch = 10,000; median of 20 repetitions.

| Benchmark | Time per item | Items / s | cv | Fills | Resting after |
|---|---:|---:|---:|---:|---:|
| Matching engine | 63 ns per order | 15.9 M | 3.9 % | 4,489 | 5,565 |
| Mixed market | 93 ns per execution | 10.8 M | 0.9 % | 41,596 | 4,967 |
| Noise trader | 78 ns | 12.8 M | 6.7 % | 1,809 | 8,801 |
| Whale | 73 ns | 13.7 M | 18.5 % | 10,321 | 80 |
| Market maker | 109 ns (54 ns per order) | 9.2 M | 7.0 % | 0 | 20,803 |
| Trend follower | 16 ns | 62 M | 0.5 % | 0 | 803 |

The fills and resting counts are identical in every run, because the inputs are
deterministic.

### Reading the results

- **Matching engine** is the headline figure: about 63 ns to accept and match one
  order, including the mutex, the queue, and `std::map`/`std::list` node allocation.
- **Mixed market** is the most realistic simulation: every agent's orders interact, so
  it produces about 4 fills per round (41,596 in total, against 1,809 for the noise
  trader alone).
- **Whale** spends its time matching. Its market orders drain the book down to 80 resting
  orders, so later market orders often find one side empty.
- **Market maker** on its own never trades (`fills = 0`). Its quotes sit one tick either
  side of the last price, nobody else is trading, so the price never moves. This
  measures resting-order insertion only.
- **Trend follower** on its own submits nothing: the seeded book's recent trades are
  flat, so it sees no trend and `resting` is unchanged. This measures its decision
  check only. Its real behaviour is covered by the mixed-market benchmark.

### Variability

Run conditions matter more than anything in the code. On the same laptop, with the same
build and the same pinned command:

| Power mode | Matching engine | Mixed market | Trend follower |
|---|---:|---:|---:|
| Balanced | 946 µs (cv 2.8 %) | 5,434 µs | 238 µs |
| Best performance | 628 µs (cv 3.9 %) | 3,714 µs | 160 µs |

Without pinning to a core, the cv on the engine and mixed-market benchmarks reached
29% and 56%. Compare numbers only across runs made under the same conditions.

### Earlier results

Figures recorded on 2026-10-03 (169–332 ns per agent execution) predate several
agent-behaviour changes and the removal of per-order clock reads from the order path.
They used a different benchmark set and are not comparable.

## Limitations

- Throughput only. Per-order latency percentiles (p50/p99) would show tail behaviour,
  but Windows' high-resolution clock (about 100 ns) is too coarse to time single
  ~80 ns operations reliably.
- Single-threaded. There is no benchmark of several agents submitting concurrently;
  see [architecture.md](architecture.md#concurrency-model) for why.
- The engine's order stream uses a fixed price band around $100, so the book does not
  drift the way it does in the agent simulations.
