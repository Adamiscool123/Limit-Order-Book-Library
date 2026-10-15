#include <iostream>
#include <benchmark/benchmark.h>
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"
#include "variables.h"

// Every benchmark uses the same seed so each measured batch starts from an identical market.
const unsigned benchmark_seed = 12345;

void order_book_creation(Global_Variables &v, int count)
{
    for (int i = 0; i < count; i++)
    {
        noise_trader t1(v);

        t1.execute();
    }
}

// Counts every order resting in the book (both sides).
long long resting_orders(const Global_Variables &v)
{
    long long total = 0;

    for (const auto &[price, level] : v.buyMap)
    {
        total += level.orders.size();
    }

    for (const auto &[price, level] : v.sellMap)
    {
        total += level.orders.size();
    }

    return total;
}

// Adds counters that show what a batch actually did, so a fast result can't hide a batch that never traded.
void report_counters(benchmark::State &state, long long executions, long long fills, long long resting)
{
    state.SetItemsProcessed(executions);

    // Averaged over iterations, so these are per batch.
    state.counters["fills"] = benchmark::Counter(fills, benchmark::Counter::kAvgIterations);
    state.counters["resting"] = benchmark::Counter(resting, benchmark::Counter::kAvgIterations);
}

/*
    Times n executions of one agent type on a book built from `setup` noise-trader submissions.

    range(0) = setup submissions
    range(1) = timed agent executions
*/
template <class Agent>
void agent_fixed_book(benchmark::State &state)
{
    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    long long fills = 0;

    long long resting = 0;

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            // Make new order book.
            Global_Variables v;

            v.seed = benchmark_seed;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            Agent t1(v);

            const size_t fills_before = v.price_history.size();

            state.ResumeTiming();

            t1.loop(submitted_orders);

            state.PauseTiming();

            fills += v.price_history.size() - fills_before;

            resting += resting_orders(v);
        }

        // need to do this otherwise PauseTiming would be invalid (Can't pause time on something alr paused)
        state.ResumeTiming();
    }

    report_counters(state, state.iterations() * submitted_orders, fills, resting);
}

static void Noise_Trader_Test_Fixed_Book(benchmark::State &state)
{
    agent_fixed_book<noise_trader>(state);
}

static void Whale_Test_Fixed_Book(benchmark::State &state)
{
    agent_fixed_book<whale>(state);
}

static void Market_Maker_Test_Fixed_Book(benchmark::State &state)
{
    agent_fixed_book<market_maker>(state);
}

static void Trend_Follower_Test_Fixed_Book(benchmark::State &state)
{
    agent_fixed_book<trend_follower>(state);
}

/*
    All four automated agents trading in the same market. One round runs each agent once,
    so the market maker's quotes get hit by the others and the trend follower sees real trends.

    range(0) = setup submissions
    range(1) = timed rounds (4 agent executions each)
*/
static void Mixed_Market_Test_Fixed_Book(benchmark::State &state)
{
    const int count = state.range(0);

    const int rounds = state.range(1);

    long long fills = 0;

    long long resting = 0;

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            Global_Variables v;

            v.seed = benchmark_seed;

            v.rng.seed(v.seed);

            order_book_creation(v, count);

            market_maker maker(v);
            noise_trader noise(v);
            trend_follower trend(v);
            whale big(v);

            const size_t fills_before = v.price_history.size();

            state.ResumeTiming();

            for (int i = 0; i < rounds; i++)
            {
                maker.execute();
                noise.execute();
                trend.execute();
                big.execute();
            }

            state.PauseTiming();

            fills += v.price_history.size() - fills_before;

            resting += resting_orders(v);
        }

        state.ResumeTiming();
    }

    report_counters(state, state.iterations() * rounds * 4, fills, resting);
}

/*
    Pre-generates a deterministic stream of orders around $100 so the matching-engine benchmark
    doesn't time any random number generation or agent logic.

    90% limit orders: buys at $95-$101, sells at $99-$105, so the price bands overlap and many of them trade.
    10% market orders.
*/
std::vector<Order> make_order_stream(int count)
{
    std::mt19937 rng(benchmark_seed);

    std::uniform_int_distribution<int> coin(0, 1);
    std::uniform_int_distribution<int> percent(0, 99);
    std::uniform_int_distribution<int> buy_price(95, 101);
    std::uniform_int_distribution<int> sell_price(99, 105);
    std::uniform_int_distribution<int> share(1, 10);

    std::vector<Order> orders;

    orders.reserve(count);

    for (int i = 0; i < count; i++)
    {
        Order trader;

        trader.order_id = i;

        trader.side = (coin(rng) == 0) ? Side::Buy : Side::Sell;

        trader.shares = share(rng);

        if (percent(rng) < 10)
        {
            trader.order_type = OrderType::Market;

            trader.price = 0;
        }
        else
        {
            trader.order_type = OrderType::Limit;

            trader.price = (trader.side == Side::Buy) ? buy_price(rng) : sell_price(rng);
        }

        orders.push_back(trader);
    }

    return orders;
}

// Same path as manual::trade: lock, push onto the queue, then match.
void submit(Global_Variables &v, Matching_Engine &engine, const Order &trader)
{
    {
        std::lock_guard<std::mutex> lock(v.market_mutex);

        v.TradingQueue.push(trader);
    }

    engine.checker(v);
}

/*
    Times only queue insertion and matching for a pre-generated order stream.

    range(0) = setup orders (the first part of the stream)
    range(1) = timed orders (the rest of the stream)
*/
static void Matching_Engine_Test_Fixed_Book(benchmark::State &state)
{
    const int count = state.range(0);

    const int submitted_orders = state.range(1);

    // Generated once, before any timing starts.
    const std::vector<Order> orders = make_order_stream(count + submitted_orders);

    Matching_Engine engine;

    long long fills = 0;

    long long resting = 0;

    for (auto _ : state)
    {
        state.PauseTiming();

        {
            Global_Variables v;

            for (int i = 0; i < count; i++)
            {
                submit(v, engine, orders[i]);
            }

            const size_t fills_before = v.price_history.size();

            state.ResumeTiming();

            for (int i = count; i < count + submitted_orders; i++)
            {
                submit(v, engine, orders[i]);
            }

            state.PauseTiming();

            fills += v.price_history.size() - fills_before;

            resting += resting_orders(v);
        }

        state.ResumeTiming();
    }

    report_counters(state, state.iterations() * submitted_orders, fills, resting);
}

/*
    Shared settings for every benchmark:

    - Setup sizes 10 to 10,000, timed batches of 1,000 and 10,000. Smaller batches were dominated by
      the cost of pausing/resuming the timer.
    - One batch per repetition. Letting Google Benchmark calibrate would rebuild the setup market
      thousands of times, and a batch already repeats the work many times.
    - Wall-clock time. Windows' CPU clock is too coarse for a single batch and reads 0.
*/
void fixed_book_args(benchmark::Benchmark *b)
{
    b->ArgsProduct({{10, 100, 1000, 10000}, {1000, 10000}})
        ->Iterations(1)
        ->Repetitions(20)
        ->ReportAggregatesOnly(true)
        ->UseRealTime()
        ->Unit(benchmark::kMicrosecond);
}

BENCHMARK(Matching_Engine_Test_Fixed_Book)->Apply(fixed_book_args);
BENCHMARK(Mixed_Market_Test_Fixed_Book)->Apply(fixed_book_args);
BENCHMARK(Noise_Trader_Test_Fixed_Book)->Apply(fixed_book_args);
BENCHMARK(Whale_Test_Fixed_Book)->Apply(fixed_book_args);
BENCHMARK(Market_Maker_Test_Fixed_Book)->Apply(fixed_book_args);
BENCHMARK(Trend_Follower_Test_Fixed_Book)->Apply(fixed_book_args);

// Generate the main() function
BENCHMARK_MAIN();
