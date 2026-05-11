#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "order_book.h"
#include "matching_engine.h"
#include "Agents.h"
#include "variables.h"

namespace py = pybind11;

PYBIND11_MODULE(orderbook_wrapper, m) {
    m.doc() = "Limit Order Book Python API";

    py::class_<Order>(m, "Order")
        .def(py::init<>())
        .def_readwrite("side", &Order::side)
        .def_readwrite("price", &Order::price)
        .def_readwrite("shares", &Order::shares)
        .def_readwrite("order_type", &Order::order_type)
        .def_readwrite("order_id", &Order::order_id)
        .def_readwrite("traded", &Order::traded);

    py::class_<Global_Variables> gv(m, "GlobalVariables");
    gv.def(py::init<>())
    .def_readwrite("starting_price", &Global_Variables::starting_price)
    .def_readwrite("price_history", &Global_Variables::price_history)
    .def_readwrite("count", &Global_Variables::count)
    .def_readwrite("buyMap", &Global_Variables::buyMap)
    .def_readwrite("sellMap", &Global_Variables::sellMap);

    py::class_<Matching_Engine>(m, "Matching_Engine")
        .def(py::init<>())
        .def("checker", &Matching_Engine::checker)
        .def("buy", &Matching_Engine::buy)
        .def("sell", &Matching_Engine::sell);

    py::class_<Order_Book>(m, "Order_Book")
        .def(py::init<>())
        .def("printer", &Order_Book::printer)
        .def("sell", &Order_Book::sell)
        .def("buy", &Order_Book::buy);

    py::class_<Agent_Base>(m, "AgentBase")
        .def("infinite_loop", &Agent_Base::infinite_loop)
        .def("execute", &Agent_Base::execute)
        .def("loop", &Agent_Base::loop);

    py::class_<manual, Agent_Base>(m, "Manual")
        .def(py::init<Global_Variables&>(), py::keep_alive<1, 2>())
        .def("trade", &manual::trade)
        .def("execute_agent", &manual::execute_agent);

    py::class_<market_maker, Agent_Base>(m, "MarketMaker")
        .def(py::init<Global_Variables&>(), py::keep_alive<1, 2>())
        .def("execute_agent", &market_maker::execute_agent);

    py::class_<noise_trader, Agent_Base>(m, "NoiseTrader")
        .def(py::init<Global_Variables&>(), py::keep_alive<1, 2>())
        .def("execute_agent", &noise_trader::execute_agent);

    py::class_<trend_follower, Agent_Base>(m, "TrendFollower")
        .def(py::init<Global_Variables&>(), py::keep_alive<1, 2>())
        .def("execute_agent", &trend_follower::execute_agent);

    py::class_<whale, Agent_Base>(m, "Whale")
        .def(py::init<Global_Variables&>(), py::keep_alive<1, 2>())
        .def("execute_agent", &whale::execute_agent);

    py::class_<PriceLevel>(m, "PriceLevel")
        .def(py::init<>())
        .def_readwrite("total_shares", &PriceLevel::total_shares)
        .def_readwrite("orders", &PriceLevel::orders);
    

}