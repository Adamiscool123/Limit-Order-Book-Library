#include "variables.h"
#include "order_book.h"

void Order_Book::printer(Global_Variables& m){
    std::cout << "\033[2J\033[1;1H";

    if (m.sellMap.empty() && m.buyMap.empty()) {
        return; 
    }

    sell(m);

    buy(m);
}

void Order_Book::sell(Global_Variables& m){
    std::cout << "       Order Book" << std::endl;

    for (const auto& [price, level] : m.sellMap) {
        int total_shares = 0;

        for (const auto& order : level.orders) {
            total_shares += order.shares;
        }

        if (total_shares > 0) {
            std::cout << "SELL:   $" << price << " | " << total_shares << std::endl;
        }
    }

    std::cout << "--------------------------------------" << std::endl;
}

void Order_Book::buy(Global_Variables& m){
    for (const auto& [price, level] : m.buyMap) {
        int total_shares = 0;

        for (const auto& order : level.orders) {
            total_shares += order.shares;
        }

        if (total_shares > 0) {
            std::cout << "BUY:   $" << price << " | " << total_shares << std::endl;
        }
    }
}