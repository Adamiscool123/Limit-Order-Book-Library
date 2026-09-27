#include "variables.h"
#include "order_book.h"
#include "matching_engine.h"

void Matching_Engine::checker(Global_Variables &variables)
{
    {
        // Lock the queue
        std::lock_guard<std::mutex> lock(variables.market_mutex);

        if (variables.TradingQueue.empty())
        {
            return;
        }

        while (!variables.TradingQueue.empty())
        {
            // Make the order struct
            Order trader = variables.TradingQueue.front();

            // Create the order book for printing
            Order_Book print_order_book;

            // Pop out the front.

            variables.TradingQueue.pop();
            /*
                While loop that checks if the traders wants to sell and buy,
                then depending on that they match that order with an opposite order so buyer with seller and seller with buyer.

                For buyer it finds the cheapest seller (buyers want the best price) and gets matched,

                if they have more shares than the seller then the engine will repeat this while loop until all their shares are gone.

                If they have the same shares than the seller then the engine will simply match the 2 orders and go on to the next trader.

                If the buyer has less shares than the seller then the buyer will still get matched but the seller will still be in the queue
                until all their shares are gone
            */

            bool break_loop = false;

            while (trader.shares > 0 && break_loop == false)
            {
                if (trader.side == 0)
                { // buy
                    buy(trader, variables, break_loop);
                }
                else
                { // sell
                    sell(trader, variables, break_loop);
                }
            }

            if (trader.shares > 0)
            {
                if (trader.side == 0)
                {
                    variables.buyMap[trader.price].orders.push_back(trader);
                    variables.buyMap[trader.price].total_shares += trader.shares;
                }
                else
                {
                    variables.sellMap[trader.price].orders.push_back(trader);
                    variables.sellMap[trader.price].total_shares += trader.shares;
                }
            }
        }
    }
}

void Matching_Engine::buy(Order &trader, Global_Variables &variables, bool &break_loop)
{
    // A buy can trade only if its price reaches the cheapest available sell price.
    if (!variables.sellMap.empty() && trader.price >= variables.sellMap.begin()->first)
    {

        // sellMap is ordered from lowest to highest, so begin() is the best ask.
        auto best_sell_level = variables.sellMap.begin();

        // Orders at the same price are FIFO, so match the oldest sell order first.
        Order &best_sell_order = best_sell_level->second.orders.front();

        // The buyer is larger: fully fill the resting sell order. The buyer still
        // has shares left, so checker() will loop and try the next sell order.
        if (trader.shares > best_sell_order.shares)
        {
            variables.price_history.push_back(best_sell_level->first);
            int matched = best_sell_order.shares;
            trader.shares -= matched;
            best_sell_level->second.total_shares -= matched;
            best_sell_level->second.orders.pop_front();

            if (best_sell_level->second.orders.empty())
            {
                variables.sellMap.erase(best_sell_level);
            }
        }
        // Both orders have the same quantity, so both are completely filled.
        else if (trader.shares == best_sell_order.shares)
        {
            variables.price_history.push_back(best_sell_level->first);
            int matched = best_sell_order.shares;
            best_sell_level->second.total_shares -= matched;
            trader.shares = 0;
            best_sell_level->second.orders.pop_front();

            if (best_sell_level->second.orders.empty())
            {
                variables.sellMap.erase(best_sell_level);
            }
        }
        // The buyer is smaller: fully fill it and leave the reduced sell order
        // at the front of its price level.
        else
        {
            variables.price_history.push_back(best_sell_level->first);
            int matched = trader.shares;
            best_sell_order.shares -= matched;
            best_sell_level->second.total_shares -= matched;
            trader.shares = 0;
        }
    }
    else
    {
        // There is no sell order at an acceptable price. Stop matching so the
        // buyer's remaining shares can rest in the buy book.
        break_loop = true;
    }
}

void Matching_Engine::sell(Order &trader, Global_Variables &variables, bool &break_loop)
{
    // A sell can trade only if its price reaches the highest available buy price.
    if (!variables.buyMap.empty() && trader.price <= variables.buyMap.begin()->first)
    {

        // buyMap is ordered from highest to lowest, so begin() is the best bid.
        auto best_buy_level = variables.buyMap.begin();

        // Orders at the same price are FIFO, so match the oldest buy order first.
        Order &best_buy_order = best_buy_level->second.orders.front();

        // The seller is larger: fully fill the resting buy order. The seller still
        // has shares left, so checker() will loop and try the next buy order.
        if (trader.shares > best_buy_order.shares)
        {
            variables.price_history.push_back(best_buy_level->first);
            int matched = best_buy_order.shares;
            trader.shares -= matched;
            best_buy_level->second.total_shares -= matched;
            best_buy_level->second.orders.pop_front();

            if (best_buy_level->second.orders.empty())
            {
                variables.buyMap.erase(best_buy_level);
            }
        }
        // Both orders have the same quantity, so both are completely filled.
        else if (trader.shares == best_buy_order.shares)
        {
            variables.price_history.push_back(best_buy_level->first);
            int matched = best_buy_order.shares;
            best_buy_level->second.total_shares -= matched;
            trader.shares = 0;
            best_buy_level->second.orders.pop_front();

            if (best_buy_level->second.orders.empty())
            {
                variables.buyMap.erase(best_buy_level);
            }
        }
        // The seller is smaller: fully fill it and leave the reduced buy order
        // at the front of its price level.
        else
        {
            variables.price_history.push_back(best_buy_level->first);
            int matched = trader.shares;
            best_buy_order.shares -= matched;
            best_buy_level->second.total_shares -= matched;
            trader.shares = 0;
        }
    }
    else
    {
        // There is no buy order at an acceptable price. Stop matching so the
        // seller's remaining shares can rest in the sell book.
        break_loop = true;
    }
}
