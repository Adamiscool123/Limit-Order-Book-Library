#ifndef VARIBLES_H
#define VARIBLES_H

#include <iostream>
#include <random>
#include <chrono>
#include <queue>
#include <map>
#include <list>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <stdio.h>
#include <algorithm>
#include <functional>

// Trader order
struct Order{
    int side;
    int price;
    int shares;
    int order_type;
    int order_id;
    long long timestamp;
    bool traded = false;
};

class Global_Variables{
public:

    struct order_complete{
        int price;

        int shares = 0;
    };

    // For tracking time
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rng{seed};

    // Starting price for stock
    int starting_price = 100;

    // Trading queue for orders which then gets either pushed to the buy or the sell section depending on the traders decision 
    std::queue<Order> TradingQueue;

    // Orders for buy and sell
    std::vector <Order> buy;
    std::vector <Order> sell;

    // After an order is executed the first one gets put into price history.
    std::vector <int> price_history;

    // To lock the Queue so that orders don't run at the same time causing a crash error
    std::mutex market_mutex;

    int count = 0;
};

#endif