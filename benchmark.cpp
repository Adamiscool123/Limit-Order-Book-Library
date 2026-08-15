#include <iostream>
#include <benchmark/benchmark.h>
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"
#include "variables.h"

static void BM_StringCreation(benchmark::State &state)
{
    for (auto _ : state)
    {
        
    }
}

// 2. Register the benchmark
BENCHMARK(BM_StringCreation);

// 3. Generate the main() function
BENCHMARK_MAIN();