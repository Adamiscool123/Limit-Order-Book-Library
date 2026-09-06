#include <iostream>
#include <benchmark/benchmark.h>
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"
#include "variables.h"

static void Test_Market_All_Agents(benchmark::State &state)
{
    Global_Variables v;
    for (auto _ : state)
    {

        market_maker t1(v);

        noise_trader t2(v);

        trend_follower t3(v);

        whale t4(v);

        t1.execute_agent();

        t2.execute_agent();

        t3.execute_agent();

        t4.execute_agent();
    }
}

static void Market_Maker_Test(benchmark::State &state)
{
    Global_Variables v;

    for (int i = 0; i < 100000; i++)
    {
        noise_trader t1(v);

        t1.execute_agent();
    }

    for (auto _ : state)
    {
        market_maker t1(v);

        t1.execute_agent();
    }
}

static void Whale_Test(benchmark::State &state)
{
    Global_Variables v;

    for (int i = 0; i < 100000; i++)
    {
        noise_trader t1(v);

        t1.execute_agent();
    }

    for (auto _ : state)
    {
        whale t1(v);

        t1.execute_agent();
    }
}

static void Trend_Follower_Test(benchmark::State &state)
{
    Global_Variables v;

    for (int i = 0; i < 100000; i++)
    {
        noise_trader t1(v);

        t1.execute_agent();
    }

    for (auto _ : state)
    {
        trend_follower t1(v);

        t1.execute_agent();
    }
}

static void Noise_Trader_Test(benchmark::State &state)
{
    Global_Variables v;

    for (int i = 0; i < 100000; i++)
    {
        noise_trader t1(v);

        t1.execute_agent();
    }

    for (auto _ : state)
    {
        noise_trader t1(v);

        t1.execute_agent();
    }
}

// 2. Register the benchmark
BENCHMARK(Noise_Trader_Test);
BENCHMARK(Whale_Test);
BENCHMARK(Market_Maker_Test);
BENCHMARK(Trend_Follower_Test);

// 3. Generate the main() function
BENCHMARK_MAIN();