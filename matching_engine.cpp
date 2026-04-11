#include "varibles.h"
#include "order_book.h"
#include "matching_engine.h"

void matching_engine(Global_Varibles& m){
        {
        // Lock the queue
        std::lock_guard<std::mutex> lock(m.market_mutex);

        if (m.TradingQueue.empty()) {
            return;
        }

        Global_Varibles::Order trader = m.TradingQueue.front();
        m.TradingQueue.pop();
        /*
            While loop that checks if the traders wants to sell and buy, 
            then depending on that they match that order with an opposite order so buyer with seller and seller with buyer.

            For buyer it finds the cheapest seller (buyers want the best price) and gets matched, 
            
            if they have more shares than the seller then the engine will repeat this while loop until all their shares are gone.
            
            If they have the same shares than the seller then the engine will simply match the 2 orders and go on to the next trader.
            
            If the buyer has less shares than the seller then the buyer will still get matched but the seller will still be in the queue
            until all their shares are gone
        */
        while (trader.shares > 0) {
            if (trader.side == 0) {  // buy
                if (!m.sell.empty() && trader.price >= m.sell.front().price) {
                    if (trader.shares > m.sell.front().shares) {
                        trader.shares -= m.sell.front().shares;
                        m.price_history.push_back(m.sell.front().price);
                        m.sell.erase(m.sell.begin());
                    }
                    else if (trader.shares == m.sell.front().shares) {
                        m.price_history.push_back(m.sell.front().price);
                        trader.shares = 0;
                        m.sell.erase(m.sell.begin());
                    }
                    else {
                        m.price_history.push_back(m.sell.front().price);
                        m.sell.front().shares -= trader.shares;
                        trader.shares = 0;
                    }
                }
                else {
                    m.buy.push_back(trader);
                    break;
                }
            }
            else {  // sell
                if (!m.buy.empty() && trader.price <= m.buy.front().price) {
                    if (trader.shares > m.buy.front().shares) {
                        trader.shares -= m.buy.front().shares;
                        m.price_history.push_back(m.buy.front().price);
                        m.buy.erase(m.buy.begin());
                    }
                    else if (trader.shares == m.buy.front().shares) {
                        m.price_history.push_back(m.buy.front().price);
                        trader.shares = 0;
                        m.buy.erase(m.buy.begin());
                    }
                    else {
                        m.price_history.push_back(m.buy.front().price);
                        m.buy.front().shares -= trader.shares;
                        trader.shares = 0;
                    }
                }
                else {
                    m.sell.push_back(trader);
                    break;
                }
            }
        }

        // Sort the buy vector from largest to smallest (Descending)
        std::sort(m.buy.begin(), m.buy.end(), [](const Global_Varibles::Order& a, const Global_Varibles::Order& b) {
            return a.price > b.price;
        });

        // Sort the sell vector from smallest to largest (Ascending)
        std::sort(m.sell.begin(), m.sell.end(), [](const Global_Varibles::Order& a, const Global_Varibles::Order& b) {
            return a.price < b.price;
        });

    }

    order_book(m);
}