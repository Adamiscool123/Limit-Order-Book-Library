#ifndef AGENTS_H
#define AGENTS_H

#include "variables.h"

class Agent{
public: 
    virtual ~Agent() = default;

    virtual void agent(Global_Variables& m) = 0;
};

class market_maker : public Agent{
public: 
    void agent(Global_Variables& m) override;
};

class noise_trader : public Agent{
public: 
    void agent(Global_Variables& m) override;
};

class trend_follower : public Agent{
public: 
    void agent(Global_Variables& m) override;
};

class whale : public Agent{
public: 
    void agent(Global_Variables& m) override;
};

class Agents {

    public:
        /*
            Here we have the thread loops so it can run simultaneously since you 
            firstly, need a lot of volatility otherwise it will be a slow dead market. 
            Secondly, in the real world you don't wait for this trader to go then you go, they sometimes trade at the same time.
            Lastly, speed.  

            Structure:

            An infinite while loop.

            Inside it you have (this -> <typeof trader>) which calls the noise trader function.

            Then you have the matching_engine() function which calls the engine to match the trader's order with another

            Lastly, you have std::this_thread::sleep_for(std::chrono::<milliseconds or seconds>(<time>)) 
            which sets how frequent the trader will place their order so if it was milliseconds and time 500 then the trader would send their orders
            every 500 milliseconds

        */

        // Thread loop for trader
        void loop(Global_Variables& m, int time, std::function<void(Global_Variables& m)> trader);
};

#endif