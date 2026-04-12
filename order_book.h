#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "variables.h"

// Simple function that prints out the Limit Order Book
void order_book(Global_Variables& m);

class Order_Book{
public:
    void printer(Global_Variables& m);

    void sell(Global_Variables& m);

    void buy(Global_Variables& m);
};

#endif