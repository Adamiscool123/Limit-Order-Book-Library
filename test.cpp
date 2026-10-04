#include <cassert>
#include <iostream>

#include "Agents.h"
#include "variables.h"

// Checks that the sellMap and TradingQueue is empty when inserting trade to buyMap in an empty LBO.
void empty_book(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 5, 0, 0, v);

    // Strict must be true otherwise program stops with assertion failure.
    assert(v.TradingQueue.empty());
    assert(v.sellMap.empty());
    assert(v.buyMap.size() == 1);
    assert(v.buyMap.count(100) == 1);
    assert(v.buyMap.at(100).total_shares == 5);
    assert(v.buyMap.at(100).orders.size() == 1);
    assert(v.buyMap.at(100).orders.front().shares == 5);

    std::cout << "Empty book insertion test passed." << std::endl;
}

// Checks that $99 for buyer dosen't cross $101 for seller.
void non_crossing_limit_book(Global_Variables &v)
{
    manual trader(v);

    trader.trade(99, 5, 0, 0, v);  // Limit buy
    trader.trade(101, 7, 1, 0, v); // Limit sell

    assert(v.buyMap.count(99) == 1);
    assert(v.sellMap.count(101) == 1);
    assert(v.buyMap.at(99).total_shares == 5);
    assert(v.sellMap.at(101).total_shares == 7);
    assert(v.price_history.empty());

    std::cout << "Non crossing limit order test passed." << std::endl;
}

// Checks that $100 buyer crosses $99 seller.
void crossing_limit_book(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 5, 0, 0, v); // Resting buy
    trader.trade(99, 5, 1, 0, v);  // Incoming sell

    assert(v.buyMap.empty());
    assert(v.sellMap.empty());
    assert(v.TradingQueue.empty());
    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Crossing limit order test passed." << std::endl;
}

// Checks if everything is well when limit sell $100 and incoming buy is $101 so trader share > resting limit sell.
void trader_shares_bigger_than_resting_shares(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 4, 1, 0, v);  // Resting limit sell
    trader.trade(101, 10, 0, 0, v); // Larger incoming limit buy

    assert(v.TradingQueue.empty());
    assert(v.sellMap.empty());

    assert(v.buyMap.size() == 1);
    assert(v.buyMap.count(101) == 1);
    assert(v.buyMap.at(101).total_shares == 6);
    assert(v.buyMap.at(101).orders.size() == 1);
    assert(v.buyMap.at(101).orders.front().shares == 6);
    assert(v.buyMap.at(101).orders.front().price == 101);

    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Trader shares bigger than resting shares test passed." << std::endl;
}

// Checks if multiple price levels and best price priority works.
void multiple_price_levels(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 3, 1, 0, v);  // Limit sell: 3 shares at $100
    trader.trade(101, 4, 1, 0, v);  // Limit sell: 4 shares at $101
    trader.trade(103, 10, 1, 0, v); // Limit sell: 10 shares at $103
    trader.trade(102, 10, 0, 0, v); // Limit buy: 10 shares at $102

    assert(v.TradingQueue.empty());

    assert(v.sellMap.size() == 1);
    assert(v.sellMap.count(103) == 1);
    assert(v.sellMap.at(103).total_shares == 10);

    assert(v.buyMap.size() == 1);
    assert(v.buyMap.count(102) == 1);
    assert(v.buyMap.at(102).total_shares == 3);
    assert(v.buyMap.at(102).orders.front().shares == 3);

    assert(v.price_history.size() == 2);
    assert(v.price_history.at(0) == 100);
    assert(v.price_history.at(1) == 101);

    std::cout << "Multiple price levels and best priority works." << std::endl;
}

// Checks if FIFO priority works.
void FIFO_Priority(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 3, 1, 0, v); // First resting sell, order_id 0
    trader.trade(100, 7, 1, 0, v); // Second resting sell, order_id 1
    trader.trade(100, 5, 0, 0, v); // Incoming buy

    assert(v.buyMap.empty());

    assert(v.sellMap.size() == 1);
    assert(v.sellMap.count(100) == 1);
    assert(v.sellMap.at(100).total_shares == 5);
    assert(v.sellMap.at(100).orders.size() == 1);

    assert(v.sellMap.at(100).orders.front().order_id == 1);
    assert(v.sellMap.at(100).orders.front().shares == 5);

    assert(v.price_history.size() == 2);
    assert(v.price_history.at(0) == 100);
    assert(v.price_history.at(1) == 100);

    std::cout << "First In First Out Priority test passed." << std::endl;
}

// Checks what happens if limit sell shares in the LBO has less than the traders shares.
void market_order_remainder(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 4, 1, 0, v); // Resting limit sell
    trader.trade(0, 10, 0, 1, v);  // Market buy; price is ignored

    assert(v.TradingQueue.empty());
    assert(v.buyMap.empty());
    assert(v.sellMap.empty());

    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Market Order Remainder test passed." << std::endl;
}

// Checks what happens if the incoming seller shares is smaller than the resting buyer shares.
void incoming_seller_smaller_than_resting_buyer(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 10, 0, 0, v); // Resting buy
    trader.trade(99, 4, 1, 0, v);   // Smaller incoming sell

    assert(v.TradingQueue.empty());
    assert(v.sellMap.empty());

    assert(v.buyMap.size() == 1);
    assert(v.buyMap.count(100) == 1);
    assert(v.buyMap.at(100).total_shares == 6);
    assert(v.buyMap.at(100).orders.size() == 1);
    assert(v.buyMap.at(100).orders.front().shares == 6);

    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Incoming seller smaller than resting buyer test passed." << std::endl;
}

// Checks what happens if market buyer shares is less than incoming seller shares.
void market_sell_remainder(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 4, 0, 0, v); // Resting limit buy
    trader.trade(0, 10, 1, 1, v);  // Oversized market sell

    assert(v.TradingQueue.empty());
    assert(v.buyMap.empty());
    assert(v.sellMap.empty());

    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Market sell remainder test passed." << std::endl;
}

int main(void)
{

    {
        Global_Variables v;
        empty_book(v);
    }

    {
        Global_Variables v;
        non_crossing_limit_book(v);
    }

    {
        Global_Variables v;
        crossing_limit_book(v);
    }

    {
        Global_Variables v;
        trader_shares_bigger_than_resting_shares(v);
    }

    {
        Global_Variables v;
        multiple_price_levels(v);
    }

    {
        Global_Variables v;
        FIFO_Priority(v);
    }

    {
        Global_Variables v;
        market_order_remainder(v);
    }

    {
        Global_Variables v;
        incoming_seller_smaller_than_resting_buyer(v);
    }

    return 0;
}