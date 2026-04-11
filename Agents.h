#ifndef AGENTS_H
#define AGENTS_H

#include "varibles.h"

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
        void noise_trader_loop(Global_Varibles& m);

        void whale_loop(Global_Varibles& m);

        void trend_follower_loop(Global_Varibles& m);

        void market_maker_loop(Global_Varibles& m);

        /*
            The market maker helps with making a lot of market trades to add more volatility to the market. 
            What it basically does here is take the current price - 1 when buying and when selling current price + 1
        */
        void market_maker(Global_Varibles& m);

        /*
            The noise trader makes limit orders where they trade at price of 20% under when buying and 20% over when selling,
            they also help with adding volatility to the market
        */

        void noise_trader(Global_Varibles& m);

        /*
            The trend follower is a trader that uses a strategy where it checks the price from 5 trades ago and compares it with the current price
            If the price before is higher than the current price or the current price is lower than the price before then he goes bullish so he sets a buy limit order
            If the price before is lower than the current price or the current price is higher than the price before then he goes bearish so he sets a sell limit order
            The bot typically trades current price - 2 when buying and current price + 2 when selling
        */
        void trend_follower(Global_Varibles& m);

        /*
            The whale is one of those big institutional traders that sends orders with large quantities adding a lot of volatility to the market
            The whale trades current price - 1 when buying and current price + 1 when selling
            It would send about 100 - 200 shares per trade 
        */

        void whale(Global_Varibles& m);

};

#endif