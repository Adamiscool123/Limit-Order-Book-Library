#include "order_book.h"
#include "Agents.h"
#include "matching_engine.h"
#include "varibles.h"

void start(Global_Varibles& m){
    // We do this because we need to set the starting price to the history so that the traders when they call price_history it is not empty
    m.price_history.push_back(m.starting_price);

    Agents self;

    // Here we make the thread objects and call the thread loops
    std::thread noise(&Agents::noise_trader_loop, &self, std::ref(m));
    std::thread market_maker(&Agents::market_maker_loop, &self, std::ref(m));
    std::thread whale(&Agents::whale_loop, &self, std::ref(m));
    std::thread trend_follower(&Agents::trend_follower_loop, &self, std::ref(m));

    /*
        Lastly, we apply the .join() method which ensures the main program waits for the threads to finish their job
        so if you were to exit the program the thread would first finish and then exit saving the data.

        I didn't use .detach() since what .detach() does is it leaves the threads running jobs in the background (like a ghost)
        so if you were to exit the program the thread will just stop mid job and all data will be lost
    */
    noise.join();
    market_maker.join();
    whale.join();
    trend_follower.join();
}

int main(void){
    Global_Varibles market;
    
    start(market);
    
    return 0;
}