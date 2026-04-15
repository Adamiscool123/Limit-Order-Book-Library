#include "variables.h"
#include "order_book.h"

void Order_Book::printer(Global_Variables& m){
    std::cout << "\033[2J\033[1;1H";

    if (m.sell.empty() && m.buy.empty()) {
        return; 
    }

    sell(m);

    buy(m);
}

void Order_Book::sell(Global_Variables& m){
    std::vector<Global_Variables::order_complete> sell_orders;

    std::vector<int> prices_sell;

    bool found_sell = false;

    for(int i = 0; i < m.sell.size(); i++){
        Global_Variables::order_complete t1;
        
        t1.price = m.sell.at(i).price; 

        t1.shares = m.sell.at(i).shares;

        for(int j = 0; j < m.sell.size(); j++){
            if(m.sell.at(i).price == m.sell.at(j).price && i != j){
                t1.shares += m.sell.at(j).shares;
            }
        }

        for(const int n : prices_sell){
            if(m.sell.at(i).price == n){
                found_sell = true;
            }
        }

        if(found_sell == false){
            sell_orders.push_back(t1);
            prices_sell.push_back(t1.price);
        }

        found_sell = false;
    }

    std::cout << "       Order Book" << std::endl;

    for (int i = 0; i < sell_orders.size(); i++) {
        
        std::cout << "SELL:   $" << sell_orders.at(i).price << " | " << sell_orders.at(i).shares << std::endl;
    }

    std::cout << "--------------------------------------" << std::endl;
}

void Order_Book::buy(Global_Variables& m){
    std::vector<Global_Variables::order_complete> buy_orders;

    std::vector<int> prices_buy;

    bool found_buy = false;

    for(int i = 0; i < m.buy.size(); i++){
        Global_Variables::order_complete t1;
        
        t1.price = m.buy.at(i).price; 

        t1.shares = m.buy.at(i).shares;

        for(int j = 0; j < m.buy.size(); j++){
            if(m.buy.at(i).price == m.buy.at(j).price && i != j){
                t1.shares += m.buy.at(j).shares;
            }
        }

        for(const int n : prices_buy){
            if(m.buy.at(i).price == n){
                found_buy = true;
            }
        }

        if(found_buy == false){
            buy_orders.push_back(t1);
            prices_buy.push_back(t1.price);
        }

        found_buy = false;
    }

    for (int i = 0; i < buy_orders.size(); i++) {
        std::cout << "BUY:   $" << buy_orders.at(i).price << " | " << buy_orders.at(i).shares << std::endl;
    }
}