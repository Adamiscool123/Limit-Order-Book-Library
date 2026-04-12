#include "variables.h"
#include "Agents.h"
#include "matching_engine.h"
#include "order_book.h"

using namespace std::chrono;

void loop(Global_Variables& m, int time, std::function<void(Global_Variables& m)> trader){
    while(true) {
        trader(m);
        matching_engine(m);
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }    
}

void market_maker::agent(Global_Variables& m){
    int c = 0;

    while(c != 2){
        Order trader;

        trader.agent_id = "Market Trader";

        trader.order_id = std::to_string(m.counter);

        trader.order_type = 0;

        m.counter++;

        if(c == 0){
            trader.side = 0;


            if(m.price_history.empty()){
                trader.price = m.starting_price-1;
            }
            else{
                trader.price = m.price_history.back()-1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(m.rng);
        }
        else{
            trader.side = 1;

            if(m.price_history.empty()){
                trader.price = m.starting_price+1;
            }
            else{
                trader.price = m.price_history.back()+1;
            }

            std::uniform_int_distribution<int> share(1, 10);

            trader.shares = share(m.rng);
        }

        // 1. Record start/end for Task A
        auto start = steady_clock::now();
        // ... code for A ...
        auto end = steady_clock::now();

        // 3. Convert to long long integers (nanoseconds)
        long long time = duration_cast<nanoseconds>(end - start).count();

        trader.timestamp = time;

        std::lock_guard<std::mutex> lock(m.market_mutex);

        m.TradingQueue.push(trader);   
        
        c++;
    }       
}

void noise_trader::agent(Global_Variables& m){

    Order trader;

    trader.agent_id = "Noise Trader";

    trader.order_id = std::to_string(m.counter);

    trader.order_type = 0;

    m.counter++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(m.rng);

    trader.side = buy_or_sell;

    if(trader.side == 0){

        if(m.price_history.empty()){
            trader.price = m.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back()-(0.2*m.price_history.back()), m.price_history.back());

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(m.rng);
    }
    else{

        if(m.price_history.empty()){
            trader.price = m.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back(), (m.price_history.back()+(0.2*m.price_history.back())));

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(1, 10);

        trader.shares = share(m.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(m.market_mutex);

    m.TradingQueue.push(trader);
}

void trend_follower::agent(Global_Variables& m){
    Order trader;

    trader.agent_id = "Trend Trader";

    trader.order_id = std::to_string(m.counter);

    trader.order_type = 0;

    m.counter++;

    if(m.price_history.size() < 5){
        return;
    }

    int buy_or_sell;

    if(m.price_history.back() > m.price_history.at(m.price_history.size()-5)){
        buy_or_sell = 0;

        trader.side = buy_or_sell;
    }
    else{
        buy_or_sell = 1;

        trader.side = buy_or_sell;
    }

    if(trader.side == 0){

        if(m.price_history.empty()){
            trader.price = m.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back()-2, m.price_history.back());

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(m.rng);
    }
    else{

        if(m.price_history.empty()){
            trader.price = m.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back(), m.price_history.back()+2);

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(m.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(m.market_mutex);

    m.TradingQueue.push(trader);                
}

void whale::agent(Global_Variables& m){
    Order trader;

    trader.agent_id = "Whale Trader";

    trader.order_id = std::to_string(m.counter);

    trader.order_type = 0;

    m.counter++;

    std::uniform_int_distribution<int> dist(0, 1);

    int buy_or_sell = dist(m.rng);

    trader.side = buy_or_sell;

    if(trader.side == 0){

        if(m.price_history.empty()){
            trader.price = m.starting_price-1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back()-10, m.price_history.back());

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(m.rng);
    }
    else{

        if(m.price_history.empty()){
            trader.price = m.starting_price+1;
        }
        else{
            std::uniform_int_distribution<int> size(m.price_history.back(), m.price_history.back()+10);

            trader.price = size(m.rng);
        }

        std::uniform_int_distribution<int> share(100, 200);

        trader.shares = share(m.rng);
    }

    // 1. Record start/end for Task A
    auto start = steady_clock::now();
    // ... code for A ...
    auto end = steady_clock::now();

    // 3. Convert to long long integers (nanoseconds)
    long long time = duration_cast<nanoseconds>(end - start).count();

    trader.timestamp = time;

    std::lock_guard<std::mutex> lock(m.market_mutex);

    m.TradingQueue.push(trader);            
}