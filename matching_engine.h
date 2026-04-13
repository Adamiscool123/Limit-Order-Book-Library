#ifndef MATCHING_ENGINE_H
#define MATCHING_ENGINE_H

#include "variables.h"

class Matching_Engine {
protected:
    bool break_loop;
public:

    void checker(Global_Variables& m);

    void buy(Order trader, Global_Variables& m, bool& break_loop);

    void sell(Order trader, Global_Variables& m, bool& break_loop);
};

#endif