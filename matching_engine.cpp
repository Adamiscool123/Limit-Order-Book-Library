#include "variables.h"
#include "order_book.h"
#include "matching_engine.h"

void Matching_Engine::checker(Global_Variables& variables){
    {
    // Lock the queue
    std::lock_guard<std::mutex> lock(variables.market_mutex);

    if (variables.TradingQueue.empty()) {
        return;
    }

    Order trader = variables.TradingQueue.front();

    Order_Book print_order_book;

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

    while (trader.shares > 0 && break_loop == false) {
        if (trader.side == 0) {  // buy
            buy(trader, variables, break_loop);
        }
        else {  // sell
            sell(trader, variables, break_loop);
        }
    }

    if (trader.shares > 0) {
        if (trader.side == 0) {
            variables.buyMap[trader.price].orders.push_back(trader);
            variables.buyMap[trader.price].total_shares += trader.shares;
        } else {
            variables.sellMap[trader.price].orders.push_back(trader);
            variables.sellMap[trader.price].total_shares += trader.shares;
        }
    }

    print_order_book.printer(variables);

    }
}

void Matching_Engine::buy(Order& trader, Global_Variables& variables, bool& break_loop){
    if (!variables.sellMap.empty() && trader.price >= variables.sellMap.begin()->first) {

        auto best_sell_level = variables.sellMap.begin();

        Order& best_sell_order = best_sell_level->second.orders.front();

        if (trader.shares > best_sell_order.shares) {
            trader.shares -= best_sell_order.shares;
            variables.price_history.push_back(best_sell_level->first);
            best_sell_level->second.orders.pop_front();
        }
        else if (trader.shares == best_sell_order.shares) {
            variables.price_history.push_back(best_sell_level->first);
            trader.shares = 0;
            best_sell_level->second.orders.pop_front();
        }
        else {
            variables.price_history.push_back(best_sell_level->first);
            best_sell_order.shares -= trader.shares;
            trader.shares = 0;
        }
    }
    else {
        break_loop = true;
    }
}

void Matching_Engine::sell(Order& trader, Global_Variables& variables, bool& break_loop){
    if (!variables.buyMap.empty() && trader.price <= variables.buyMap.begin()->first) {

        auto best_buy_level = variables.buyMap.begin();

        Order& best_buy_order = best_buy_level->second.orders.front();

        if (trader.shares > best_buy_order.shares) {
            trader.shares -= best_buy_order.shares;
            variables.price_history.push_back(best_buy_level->first);
            best_buy_level->second.orders.pop_front();
        }
        else if (trader.shares == best_buy_order.shares) {
            variables.price_history.push_back(best_buy_level->first);
            trader.shares = 0;
            best_buy_level->second.orders.pop_front();
        }
        else {
            variables.price_history.push_back(best_buy_level->first);
            best_buy_order.shares -= trader.shares;
            trader.shares = 0;
        }
    }
    else {
        break_loop = true;
    }
}