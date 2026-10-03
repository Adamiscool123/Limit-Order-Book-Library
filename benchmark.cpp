#include <iostream>
#include <benchmark/benchmark.h>
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"
#include "variables.h"

static void Market_Maker_Test(benchmark::State &state)
{
    Global_Variables v;

    v.seed = 12345;
    v.rng.seed(v.seed);

    const int count = state.range(0);

    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }

    for (auto _ : state)
    {
        market_maker t1(v);

        t1.execute();
    }
}

static void Whale_Test(benchmark::State &state)
{
    Global_Variables v;

    v.seed = 12345;
    v.rng.seed(v.seed);

    const int count = state.range(0);

    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }

    for (auto _ : state)
    {
        whale t1(v);

        t1.execute();
    }
}

static void Trend_Follower_Test(benchmark::State &state)
{
    Global_Variables v;

    v.seed = 12345;
    v.rng.seed(v.seed);

    const int count = state.range(0);

    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }

    for (auto _ : state)
    {
        trend_follower t1(v);

        t1.execute();
    }
}

static void Noise_Trader_Test(benchmark::State &state)
{
    Global_Variables v;

    v.seed = 12345;
    v.rng.seed(v.seed);

    const int count = state.range(0);

    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }

    for (auto _ : state)
    {
        noise_trader t1(v);

        t1.execute();
    }
}

// 2. Register the benchmark
BENCHMARK(Noise_Trader_Test)->Arg(10);
BENCHMARK(Noise_Trader_Test)->Arg(100);
BENCHMARK(Noise_Trader_Test)->Arg(1000);
BENCHMARK(Noise_Trader_Test)->Arg(10000);
BENCHMARK(Whale_Test)->Arg(10);
BENCHMARK(Whale_Test)->Arg(100);
BENCHMARK(Whale_Test)->Arg(1000);
BENCHMARK(Whale_Test)->Arg(10000);
BENCHMARK(Market_Maker_Test)->Arg(10);
BENCHMARK(Market_Maker_Test)->Arg(100);
BENCHMARK(Market_Maker_Test)->Arg(1000);
BENCHMARK(Market_Maker_Test)->Arg(10000);
BENCHMARK(Trend_Follower_Test)->Arg(10);
BENCHMARK(Trend_Follower_Test)->Arg(100);
BENCHMARK(Trend_Follower_Test)->Arg(1000);
BENCHMARK(Trend_Follower_Test)->Arg(10000);

// 3. Generate the main() function
BENCHMARK_MAIN();