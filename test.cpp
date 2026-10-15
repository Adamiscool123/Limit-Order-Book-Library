#include <cassert>
#include <iostream>

#include "Agents.h"
#include "variables.h"

// Checks that the sellMap and TradingQueue is empty when inserting trade to buyMap in an empty LBO.
void empty_book(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 5, Side::Buy, OrderType::Limit, v);

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

    trader.trade(99, 5, Side::Buy, OrderType::Limit, v);   // Limit buy
    trader.trade(101, 7, Side::Sell, OrderType::Limit, v); // Limit sell

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

    trader.trade(100, 5, Side::Buy, OrderType::Limit, v); // Resting buy
    trader.trade(99, 5, Side::Sell, OrderType::Limit, v); // Incoming sell

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

    trader.trade(100, 4, Side::Sell, OrderType::Limit, v); // Resting limit sell
    trader.trade(101, 10, Side::Buy, OrderType::Limit, v); // Larger incoming limit buy

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

    trader.trade(100, 3, Side::Sell, OrderType::Limit, v);  // Limit sell: 3 shares at $100
    trader.trade(101, 4, Side::Sell, OrderType::Limit, v);  // Limit sell: 4 shares at $101
    trader.trade(103, 10, Side::Sell, OrderType::Limit, v); // Limit sell: 10 shares at $103
    trader.trade(102, 10, Side::Buy, OrderType::Limit, v);  // Limit buy: 10 shares at $102

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

    trader.trade(100, 3, Side::Sell, OrderType::Limit, v); // First resting sell, order_id 0
    trader.trade(100, 7, Side::Sell, OrderType::Limit, v); // Second resting sell, order_id 1
    trader.trade(100, 5, Side::Buy, OrderType::Limit, v);  // Incoming buy

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

    trader.trade(100, 4, Side::Sell, OrderType::Limit, v); // Resting limit sell
    trader.trade(0, 10, Side::Buy, OrderType::Market, v);  // Market buy; price is ignored

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

    trader.trade(100, 10, Side::Buy, OrderType::Limit, v); // Resting buy
    trader.trade(99, 4, Side::Sell, OrderType::Limit, v);  // Smaller incoming sell

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

    trader.trade(100, 4, Side::Buy, OrderType::Limit, v);  // Resting limit buy
    trader.trade(0, 10, Side::Sell, OrderType::Market, v); // Oversized market sell

    assert(v.TradingQueue.empty());
    assert(v.buyMap.empty());
    assert(v.sellMap.empty());

    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 100);

    std::cout << "Market sell remainder test passed." << std::endl;
}

// Checks that cancelling a resting order removes it and its price level from the book.
void cancel_resting_order(Global_Variables &v)
{
    manual trader(v);

    trader.trade(99, 5, Side::Buy, OrderType::Limit, v); // Resting buy, order_id 0

    assert(trader.cancel(0) == true);

    assert(v.buyMap.empty());
    assert(v.sellMap.empty());
    assert(v.price_history.empty());

    std::cout << "Cancel resting order test passed." << std::endl;
}

// Checks that orders which already filled, or never existed, can't be cancelled.
void cancel_filled_or_unknown_order(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 5, Side::Sell, OrderType::Limit, v); // Resting sell, order_id 0
    trader.trade(100, 5, Side::Buy, OrderType::Limit, v);  // Fills it completely, order_id 1

    assert(trader.cancel(0) == false);  // Resting order that already filled
    assert(trader.cancel(1) == false);  // Incoming order that filled immediately
    assert(trader.cancel(42) == false); // Never existed

    assert(v.buyMap.empty());
    assert(v.sellMap.empty());

    std::cout << "Cancel filled or unknown order test passed." << std::endl;
}

// Checks that the rest of a partially filled order can be cancelled.
void cancel_partially_filled_order(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 10, Side::Buy, OrderType::Limit, v); // Resting buy, order_id 0
    trader.trade(100, 4, Side::Sell, OrderType::Limit, v); // Fills 4 of its 10 shares

    assert(v.buyMap.at(100).total_shares == 6);

    assert(trader.cancel(0) == true);

    assert(v.buyMap.empty());
    assert(v.price_history.size() == 1);

    std::cout << "Cancel partially filled order test passed." << std::endl;
}

// Checks that cancelling the middle order at a price keeps FIFO priority for the others.
void cancel_middle_order_keeps_FIFO(Global_Variables &v)
{
    manual trader(v);

    trader.trade(100, 2, Side::Sell, OrderType::Limit, v); // order_id 0
    trader.trade(100, 3, Side::Sell, OrderType::Limit, v); // order_id 1
    trader.trade(100, 4, Side::Sell, OrderType::Limit, v); // order_id 2

    assert(trader.cancel(1) == true);

    assert(v.sellMap.at(100).total_shares == 6);
    assert(v.sellMap.at(100).orders.size() == 2);

    trader.trade(100, 5, Side::Buy, OrderType::Limit, v); // Takes 2 from order 0, then 3 from order 2

    assert(v.buyMap.empty());

    assert(v.sellMap.at(100).total_shares == 1);
    assert(v.sellMap.at(100).orders.size() == 1);
    assert(v.sellMap.at(100).orders.front().order_id == 2);
    assert(v.sellMap.at(100).orders.front().shares == 1);

    assert(v.price_history.size() == 2);

    std::cout << "Cancel middle order keeps FIFO test passed." << std::endl;
}

// Checks that cancelling the only order at the best price makes the next price the best price.
void cancel_best_price_level(Global_Variables &v)
{
    manual trader(v);

    trader.trade(99, 3, Side::Buy, OrderType::Limit, v); // Best bid, order_id 0
    trader.trade(98, 2, Side::Buy, OrderType::Limit, v); // Next bid, order_id 1

    assert(trader.cancel(0) == true);

    assert(v.buyMap.count(99) == 0);
    assert(v.buyMap.begin()->first == 98);

    trader.trade(95, 2, Side::Sell, OrderType::Limit, v); // Must trade at $98, not the cancelled $99

    assert(v.buyMap.empty());
    assert(v.sellMap.empty());
    assert(v.price_history.size() == 1);
    assert(v.price_history.back() == 98);

    std::cout << "Cancel best price level test passed." << std::endl;
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

    {
        Global_Variables v;
        market_sell_remainder(v);
    }

    {
        Global_Variables v;
        cancel_resting_order(v);
    }

    {
        Global_Variables v;
        cancel_filled_or_unknown_order(v);
    }

    {
        Global_Variables v;
        cancel_partially_filled_order(v);
    }

    {
        Global_Variables v;
        cancel_middle_order_keeps_FIFO(v);
    }

    {
        Global_Variables v;
        cancel_best_price_level(v);
    }

    return 0;
}