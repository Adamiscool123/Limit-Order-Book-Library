#include "variables.h"
#include "order_book.h"
#include "matching_engine.h"

void Matching_Engine::sort_buy(Order& trader, Global_Variables& m){
    auto it = std::upper_bound(m.buy.begin(), m.buy.end(), trader,
    [](const Order& a, const Order& b){
        return a.price > b.price;
    });

    m.buy.insert(it, trader);
}

void Matching_Engine::sort_sell(Order& trader, Global_Variables& m){
    auto it = std::upper_bound(m.sell.begin(), m.sell.end(), trader,
    [](const Order& a, const Order& b){
        return a.price < b.price;
    });

    m.sell.insert(it, trader);
}

void Matching_Engine::checker(Global_Variables& variables, int print){
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
            buy(trader, variables, break_loop, print);
        }
        else {  // sell
            sell(trader, variables, break_loop, print);
        }
    }

    if (trader.shares > 0) {
        if (trader.side == 0) {
            sort_buy(trader, variables);
        } else {
            sort_sell(trader, variables);
        }
    }

    print_order_book.printer(variables, print);

    }
}

void Matching_Engine::buy(Order& trader, Global_Variables& variables, bool& break_loop, int print){
    if (!variables.sell.empty() && trader.price >= variables.sell.front().price) {
        if (trader.shares > variables.sell.front().shares) {
            trader.shares -= variables.sell.front().shares;
            variables.price_history.push_back(variables.sell.front().price);
            variables.sell.erase(variables.sell.begin());
        }
        else if (trader.shares == variables.sell.front().shares) {
            variables.price_history.push_back(variables.sell.front().price);
            trader.shares = 0;
            variables.sell.erase(variables.sell.begin());
        }
        else {
            variables.price_history.push_back(variables.sell.front().price);
            variables.sell.front().shares -= trader.shares;
            trader.shares = 0;
        }
    }
    else {
        break_loop = true;
    }
}

void Matching_Engine::sell(Order& trader, Global_Variables& variables, bool& break_loop, int print){
    if (!variables.buy.empty() && trader.price <= variables.buy.front().price) {
        if (trader.shares > variables.buy.front().shares) {
            trader.shares -= variables.buy.front().shares;
            variables.price_history.push_back(variables.buy.front().price);
            variables.buy.erase(variables.buy.begin());
        }
        else if (trader.shares == variables.buy.front().shares) {
            variables.price_history.push_back(variables.buy.front().price);
            trader.shares = 0;
            variables.buy.erase(variables.buy.begin());
        }
        else {
            variables.price_history.push_back(variables.buy.front().price);
            variables.buy.front().shares -= trader.shares;
            trader.shares = 0;
        }
    }
    else {
        break_loop = true;
    }
}