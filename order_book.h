#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "variables.h"

class Order_Book{
public:
    void printer(Global_Variables& m);

    void sell(Global_Variables& m);

    void buy(Global_Variables& m);
};

#endif