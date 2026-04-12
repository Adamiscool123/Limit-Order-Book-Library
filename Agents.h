#ifndef AGENTS_H
#define AGENTS_H

#include "variables.h"

class Agent{
protected:
    Global_Variables& variable;
public: 
    Agent(Global_Variables& m) : variable(m) {}

    virtual ~Agent() = default;

    virtual void agent() = 0;

    // Thread loop for trader
    void loop(int timer);
};

class market_maker : public Agent{
public: 
    market_maker(Global_Variables& m) : Agent(m) {}

    void agent() override;
};

class noise_trader : public Agent{
public: 
    noise_trader(Global_Variables& m) : Agent(m) {}

    void agent() override;
};

class trend_follower : public Agent{
public: 
    trend_follower(Global_Variables& m) : Agent(m) {}

    void agent() override;
};

class whale : public Agent{
public: 
    whale(Global_Variables& m) : Agent(m) {}

    void agent() override;
};

#endif