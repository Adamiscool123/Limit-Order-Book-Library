#include <iostream>
#include <benchmark/benchmark.h>
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"
#include "variables.h"

void order_book_creation(Global_Variables &v, int count)
{
    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }
}

static void Market_Maker_Test_Fixed_Book(benchmark::State &state)
{
    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            // Make new order book.
            Global_Variables v;

            v.seed = 12345;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            market_maker t1(v);

            state.ResumeTiming();

            t1.loop(submitted_orders);

            state.PauseTiming();
        }

        // need to do this otherwise PauseTiming would be invalid (Can't pause time on something alr paused)
        state.ResumeTiming();
    }
}

static void Whale_Test_Fixed_Book(benchmark::State &state)
{

    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            // Make new order book.
            Global_Variables v;

            v.seed = 12345;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            whale t1(v);

            state.ResumeTiming();

            t1.loop(submitted_orders);

            state.PauseTiming();
        }

        // need to do this otherwise PauseTiming would be invalid (Can't pause time on something alr paused)
        state.ResumeTiming();
    }
}

static void Trend_Follower_Test_Fixed_Book(benchmark::State &state)
{

    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            // Make new order book.
            Global_Variables v;

            v.seed = 12345;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            trend_follower t1(v);

            state.ResumeTiming();

            t1.loop(submitted_orders);

            state.PauseTiming();
        }

        // need to do this otherwise PauseTiming would be invalid (Can't pause time on something alr paused)
        state.ResumeTiming();
    }
}

static void Noise_Trader_Test_Fixed_Book(benchmark::State &state)
{

    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            // Make new order book.
            Global_Variables v;

            v.seed = 12345;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            noise_trader t1(v);

            state.ResumeTiming();

            t1.loop(submitted_orders);

            state.PauseTiming();
        }

        // need to do this otherwise PauseTiming would be invalid (Can't pause time on something alr paused)
        state.ResumeTiming();
    }
}

static void empty_book(benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming();

        {
            Global_Variables v;

            v.seed = 12345;

            v.rng.seed(v.seed);

            manual trader(v);

            state.ResumeTiming();

            trader.trade(100, 5, 0, 0, v);

            state.PauseTiming();
        }

        state.ResumeTiming();
    }
}

// 2. Register the benchmark
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10, 10});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10, 100});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10, 1000});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10, 10000});

BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({100, 10});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({100, 100});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({100, 1000});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({100, 10000});

BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({1000, 10});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({1000, 100});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({1000, 1000});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({1000, 10000});

BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10000, 10});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10000, 100});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10000, 1000});
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Args({10000, 10000});

BENCHMARK(Whale_Test_Fixed_Book)->Args({10, 10});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10, 100});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10, 1000});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10, 10000});

BENCHMARK(Whale_Test_Fixed_Book)->Args({100, 10});
BENCHMARK(Whale_Test_Fixed_Book)->Args({100, 100});
BENCHMARK(Whale_Test_Fixed_Book)->Args({100, 1000});
BENCHMARK(Whale_Test_Fixed_Book)->Args({100, 10000});

BENCHMARK(Whale_Test_Fixed_Book)->Args({1000, 10});
BENCHMARK(Whale_Test_Fixed_Book)->Args({1000, 100});
BENCHMARK(Whale_Test_Fixed_Book)->Args({1000, 1000});
BENCHMARK(Whale_Test_Fixed_Book)->Args({1000, 10000});

BENCHMARK(Whale_Test_Fixed_Book)->Args({10000, 10});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10000, 100});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10000, 1000});
BENCHMARK(Whale_Test_Fixed_Book)->Args({10000, 10000});

BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10, 10});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10, 100});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10, 1000});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10, 10000});

BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({100, 10});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({100, 100});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({100, 1000});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({100, 10000});

BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({1000, 10});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({1000, 100});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({1000, 1000});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({1000, 10000});

BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10000, 10});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10000, 100});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10000, 1000});
BENCHMARK(Market_Maker_Test_Fixed_Book)->Args({10000, 10000});

BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10, 10});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10, 100});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10, 1000});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10, 10000});

BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({100, 10});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({100, 100});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({100, 1000});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({100, 10000});

BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({1000, 10});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({1000, 100});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({1000, 1000});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({1000, 10000});

BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10000, 10});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10000, 100});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10000, 1000});
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Args({10000, 10000});

// 3. Generate the main() function
BENCHMARK_MAIN();