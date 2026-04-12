#ifndef AGENTS_H
#define AGENTS_H

#include "variables.h"

class Agent_Base{
protected:
    Global_Variables& variable;
public: 
    Agent_Base(Global_Variables& m) : variable(m) {}

    virtual ~Agent_Base() = default;

    virtual void execute_agent() = 0;

    // Thread loop for trader
    void loop(int timer);
};

class market_maker : public Agent_Base{
public: 
    market_maker(Global_Variables& m) : Agent_Base(m) {}

    void execute_agent() override;
};

class noise_trader : public Agent_Base{
public: 
    noise_trader(Global_Variables& m) : Agent_Base(m) {}

    void execute_agent() override;
};

class trend_follower : public Agent_Base{
public: 
    trend_follower(Global_Variables& m) : Agent_Base(m) {}

    void execute_agent() override;
};

class whale : public Agent_Base{
public: 
    whale(Global_Variables& m) : Agent_Base(m) {}

    void execute_agent() override;
};

#endif