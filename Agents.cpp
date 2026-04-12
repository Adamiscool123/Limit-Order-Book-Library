#include "variables.h"
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"

using namespace std::chrono;

void Agent::loop(int time){
    while(true) {
        this->agent();
        matching_engine(variable);
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }    
}

void market_maker::agent(){
    int c = 0;

    while(c != 2){
        Order trader;

        trader.agent_id = "Market Trader";

        trader.order_id = std::to_string(variable.counter);

        trader.order_type = 0;

        variable.counter++;

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

void noise_trader::agent(){

    Order trader;

    trader.agent_id = "Noise Trader";

    trader.order_id = std::to_string(variable.counter);

    trader.order_type = 0;

    variable.counter++;

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

void trend_follower::agent(){
    Order trader;

    trader.agent_id = "Trend Trader";

    trader.order_id = std::to_string(variable.counter);

    trader.order_type = 0;

    variable.counter++;

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

void whale::agent(){
    Order trader;

    trader.agent_id = "Whale Trader";

    trader.order_id = std::to_string(variable.counter);

    trader.order_type = 0;

    variable.counter++;

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