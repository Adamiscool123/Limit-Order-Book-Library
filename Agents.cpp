#include "variables.h"
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"

using namespace std::chrono;

void Agent_Base::infinite_loop(int time)
{
    while (true)
    {
        Matching_Engine Engine;
        this->execute_agent();
        Engine.checker(variable);
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
}

void Agent_Base::execute()
{
    Matching_Engine Engine;
    this->execute_agent();
    Engine.checker(variable);
}

void Agent_Base::loop(int times)
{
    for (int i = 0; i < times; i++)
    {
        Matching_Engine Engine;
        this->execute_agent();
        Engine.checker(variable);
    }
}

void manual::execute_agent() {}

void manual::trade(int price, int shares, Side side, OrderType order_type, Global_Variables &m)
{
    Order trader;
    Matching_Engine Engine;

    if ((shares < 0) || (price < 0) || (shares == 0))
    {
        std::cout << "Invalid";

        return;
    }

    trader.order_type = order_type;
    trader.price = price;
    trader.shares = shares;
    trader.side = side;
    trader.order_id = m.count;

    m.count++;

    {
        // locks queue
        std::lock_guard<std::mutex> lock(m.market_mutex);
        m.TradingQueue.push(trader);
    } // auto unlocks queue here.

    Engine.checker(m);
}

void market_maker::execute_agent()
{
    int c = 0;

    while (c != 2)
    {
        Order trader;

        trader.order_type = OrderType::Limit;

        trader.order_id = variable.count;

        variable.count++;

        if (c == 0)
        {
            trader.side = Side::Buy;

            if (variable.price_history.empty())
            {
                std::uniform_int_distribution<int> price_dist(variable.starting_price - 5, variable.starting_price - 1);
                trader.price = price_dist(variable.rng);
            }
            else
            {
                trader.price = variable.price_history.back() - 1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(variable.rng);
        }
        else
        {
            trader.side = Side::Sell;

            if (variable.price_history.empty())
            {
                std::uniform_int_distribution<int> price_dist(variable.starting_price + 1, variable.starting_price + 5);
                trader.price = price_dist(variable.rng);
            }
            else
            {
                trader.price = variable.price_history.back() + 1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(variable.rng);
        }

        /*
            steady_clock::now() - What time is it?
            moment.time_since_epoch() - How long since the clock's starting point?
            duration_cast<nanoseconds> - Express it in nanoseconds
            in_ns.count() - Give it in plain number
        */

        std::lock_guard<std::mutex> lock(variable.market_mutex);

        variable.TradingQueue.push(trader);

        c++;
    }
}

void noise_trader::execute_agent()
{

    Order trader;

    trader.order_type = OrderType::Limit;

    trader.order_id = variable.count;

    variable.count++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(variable.rng);

    if (buy_or_sell == 0)
    {
        trader.side = Side::Buy;
    }
    else
    {
        trader.side = Side::Sell;
    }

    if (trader.side == Side::Buy)
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 5, variable.starting_price + 1);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back() - (0.2 * variable.price_history.back()), variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(variable.rng);
    }
    else
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 1, variable.starting_price + 5);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back(), (variable.price_history.back() + (0.2 * variable.price_history.back())));

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(variable.rng);
    }

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);
}

void trend_follower::execute_agent()
{
    Order trader;

    trader.order_type = OrderType::Limit;

    trader.order_id = variable.count;

    variable.count++;

    if (variable.price_history.size() < 5)
    {
        return;
    }

    if (variable.price_history.back() > variable.price_history.at(variable.price_history.size() - 5))
    {
        trader.side = Side::Buy;
    }
    else
    {
        trader.side = Side::Sell;
    }

    if (trader.side == Side::Buy)
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 5, variable.starting_price + 1);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back() - 2, variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }
    else
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 1, variable.starting_price + 5);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back(), variable.price_history.back() + 2);

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);
}

void whale::execute_agent()
{
    Order trader;

    trader.order_type = OrderType::Limit;

    trader.order_id = variable.count;

    variable.count++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(variable.rng);

    if (buy_or_sell == 0)
    {
        trader.side = Side::Buy;
    }
    else
    {
        trader.side = Side::Sell;
    }

    if (trader.side == Side::Buy)
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 5, variable.starting_price + 1);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back() - 10, variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }
    else
    {

        if (variable.price_history.empty())
        {
            std::uniform_int_distribution<int> price_dist(variable.starting_price - 1, variable.starting_price + 5);
            trader.price = price_dist(variable.rng);
        }
        else
        {
            std::uniform_int_distribution<int> size(variable.price_history.back(), variable.price_history.back() + 10);

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);
}