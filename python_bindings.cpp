#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "order_book.h"
#include "matching_engine.h"
#include "Agents.h"
#include "variables.h"

namespace py = pybind11;

// Removed the mod_gil_not_used flag for standard compatibility
PYBIND11_MODULE(orderbook_wrapper, m) {
    m.doc() = "Limit Order Book Python API";

    // 1. Wrap the Order Struct (Essential because Matching_Engine uses it)
    py::class_<Order>(m, "Order")
        .def(py::init<>())
        .def_readwrite("side", &Order::side)
        .def_readwrite("price", &Order::price)
        .def_readwrite("shares", &Order::shares)
        .def_readwrite("order_type", &Order::order_type)
        .def_readwrite("order_id", &Order::order_id)
        .def_readwrite("traded", &Order::traded);

    // 2. Wrap Global_Variables
    py::class_<Global_Variables> gv(m, "GlobalVariables");

    gv.def(py::init<>())
      .def_readwrite("starting_price", &Global_Variables::starting_price)
      .def_readwrite("buy", &Global_Variables::buy)
      .def_readwrite("sell", &Global_Variables::sell)
      .def_readwrite("price_history", &Global_Variables::price_history)
      .def_readwrite("count", &Global_Variables::count);

    // 3. Wrap Nested Struct
    py::class_<Global_Variables::order_complete>(gv, "OrderComplete")
        .def(py::init<>())
        .def_readwrite("price", &Global_Variables::order_complete::price)
        .def_readwrite("shares", &Global_Variables::order_complete::shares);

    // 4. Wrap Matching_Engine
    py::class_<Matching_Engine>(m, "Matching_Engine")
        .def(py::init<>())
        .def("checker", &Matching_Engine::checker)
        .def("buy", &Matching_Engine::buy)
        .def("sell", &Matching_Engine::sell)
        .def("sort_buy", &Matching_Engine::sort_buy)
        .def("sort_sell", &Matching_Engine::sort_sell);

    // 5. Wrap Order_Book
    py::class_<Order_Book>(m, "Order_Book")
        .def(py::init<>())
        .def("printer", &Order_Book::printer)
        .def("sell", &Order_Book::sell)
        .def("buy", &Order_Book::buy);

    // 1. Wrap the Base Class
    py::class_<Agent_Base>(m, "AgentBase")
        .def("infinite_loop", &Agent_Base::infinite_loop)
        .def("execute", &Agent_Base::execute)
        .def("loop", &Agent_Base::loop);

    py::class_<manual, Agent_Base>(m, "Manual")
        .def(py::init<Global_Variables&>())
        .def("trade", &manual::trade)
        .def("execute_agent", &manual::execute_agent);

    // 3. Wrap 'market_maker'
    py::class_<market_maker, Agent_Base>(m, "MarketMaker")
        .def(py::init<Global_Variables&>())
        .def("execute_agent", &market_maker::execute_agent);

    // 4. Wrap 'noise_trader'
    py::class_<noise_trader, Agent_Base>(m, "NoiseTrader")
        .def(py::init<Global_Variables&>())
        .def("execute_agent", &noise_trader::execute_agent);

    // 5. Wrap 'trend_follower'
    py::class_<trend_follower, Agent_Base>(m, "TrendFollower")
        .def(py::init<Global_Variables&>())
        .def("execute_agent", &trend_follower::execute_agent);

    // 6. Wrap 'whale'
    py::class_<whale, Agent_Base>(m, "Whale")
        .def(py::init<Global_Variables&>())
        .def("execute_agent", &whale::execute_agent);
}