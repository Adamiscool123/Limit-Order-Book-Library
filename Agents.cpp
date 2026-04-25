#include "variables.h"
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"

/*
    Buy = 0

    Sell = 1

    Limit Order = 0

    Marekt Order = 1

    Print: Yes = 0

    Print : No = 1

*/

using namespace std::chrono;

void Agent_Base::infinite_loop(int time ){
    while(true) {
        Matching_Engine Engine;
        this->execute_agent();
        Engine.checker(variable);
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }    
}

void Agent_Base::execute(){
    Matching_Engine Engine;
    this->execute_agent();
    Engine.checker(variable);
}

void Agent_Base::loop(int times ){
    for(int i = 0; i < times; i++){
        Matching_Engine Engine;
        this->execute_agent();
        Engine.checker(variable);       
    }
}

void manual::execute_agent(){}

void manual::trade(int price, int shares, int buy_sell, int limit_market_order, Global_Variables& m){
    Order trader;
    Matching_Engine Engine;

    trader.order_type = limit_market_order;
    trader.price = price;
    trader.shares = shares;
    trader.side = buy_sell;
    trader.order_id = m.count;

    m.count++;

    auto start = steady_clock::now();
    auto end = steady_clock::now();
    long long time = duration_cast<nanoseconds>(end - start).count();
    trader.timestamp = time;

    {
        std::lock_guard<std::mutex> lock(variable.market_mutex);
        variable.TradingQueue.push(trader);
    }

    Engine.checker(variable);
}

void market_maker::execute_agent(){
    int c = 0;

    while(c != 2){
        Order trader;

        trader.order_type = 0;

        trader.order_id = variable.count;

        variable.count++;

        if(c == 0){
            trader.side = 0;

            if(variable.price_history.empty()){
                trader.price = variable.starting_price-1;
            }
            else{
                trader.price = variable.price_history.back()-1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(variable.rng);
        }
        else{
            trader.side = 1;

            if(variable.price_history.empty()){
                trader.price = variable.starting_price+1;
            }
            else{
                trader.price = variable.price_history.back()+1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(variable.rng);
        }

        // 1. Record start/end for Task A
        auto start = steady_clock::now();
        // ... code for A ...
        auto end = steady_clock::now();

        // 3. Convert to long long integers (nanoseconds)
        long long time = duration_cast<nanoseconds>(end - start).count();

        trader.timestamp = time;

        std::lock_guard<std::mutex> lock(variable.market_mutex);

        variable.TradingQueue.push(trader);   
        
        c++;
    }       
}

void noise_trader::execute_agent(){

    Order trader;

    trader.order_type = 0;

    trader.order_id = variable.count;

    variable.count++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(variable.rng);

    trader.side = buy_or_sell;

    if(trader.side == 0){

        if(variable.price_history.empty()){
            trader.price = variable.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back()-(0.2*variable.price_history.back()), variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(variable.rng);
    }
    else{

        if(variable.price_history.empty()){
            trader.price = variable.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back(), (variable.price_history.back()+(0.2*variable.price_history.back())));

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(variable.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);
}

void trend_follower::execute_agent(){
    Order trader;

    trader.order_type = 0;

    trader.order_id = variable.count;

    variable.count++;

    if(variable.price_history.size() < 5){
        return;
    }

    int buy_or_sell;

    if(variable.price_history.back() > variable.price_history.at(variable.price_history.size()-5)){
        buy_or_sell = 0;

        trader.side = buy_or_sell;
    }
    else{
        buy_or_sell = 1;

        trader.side = buy_or_sell;
    }

    if(trader.side == 0){

        if(variable.price_history.empty()){
            trader.price = variable.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back()-2, variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }
    else{

        if(variable.price_history.empty()){
            trader.price = variable.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back(), variable.price_history.back()+2);

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);                
}

void whale::execute_agent(){
    Order trader;

    trader.order_type = 0;

    trader.order_id = variable.count;

    variable.count++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(variable.rng);

    trader.side = buy_or_sell;

    if(trader.side == 0){

        if(variable.price_history.empty()){
            trader.price = variable.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back()-10, variable.price_history.back());

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }
    else{

        if(variable.price_history.empty()){
            trader.price = variable.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(variable.price_history.back(), variable.price_history.back()+10);

            trader.price = size(variable.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(variable.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(variable.market_mutex);

    variable.TradingQueue.push(trader);            
}