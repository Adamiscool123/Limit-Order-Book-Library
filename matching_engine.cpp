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
            buy(trader, variables);
        }
        else {  // sell
            sell(trader, variables);
        }
    }

    // Sort the buy vector from largest to smallest (Descending)
    std::sort(variables.buy.begin(), variables.buy.end(), [](const Order& a, const Order& b) {
        return a.price > b.price;
    });

    // Sort the sell vector from smallest to largest (Ascending)
    std::sort(variables.sell.begin(), variables.sell.end(), [](const Order& a, const Order& b) {
        return a.price < b.price;
    });

    order_book(variables);

    }
}

void Matching_Engine::buy(Order trader, Global_Variables& variables){
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
        variables.buy.push_back(trader);
        break_loop = true;
    }
}

void Matching_Engine::sell(Order trader, Global_Variables& variables){
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
        variables.sell.push_back(trader);
        break_loop = true;
    }
}