# Limit Order Book Library

A C++ limit order book and matching engine project built to simulate basic market microstructure concepts such as bid/ask books, order matching, partial fills, and agent-driven order flow.

## Overview

This project implements a basic limit order book (LOB) with a matching engine and multiple trading agent types. The goal of the project is to model how exchange order books work and to explore data structures, matching logic, and market simulation in C++.

The codebase is split into separate components for:
- order book management
- matching engine logic
- shared/global market state
- manual and automated trading agents

## Features

Current supported functionality:
- Buy and sell orders
- Limit order submission
- Basic matching engine
- Partial fills
- Price-priority matching
- FIFO handling for equal-priced orders
- Manual order entry
- Agent-based order generation
- Price history tracking

Agent types currently included:
- Manual trader
- Market maker
- Noise trader
- Trend follower
- Whale

## Why I built this

I built this project to learn more about:
- market microstructure
- exchange matching engines
- order book design
- C++ systems programming
- algorithmic trading infrastructure

This project started as a learning project and is being extended into a more realistic exchange-style simulator.

## Project Structure

```text
.
├── Agents.cpp
├── Agents.h
├── matching_engine.cpp
├── matching_engine.h
├── order_book.cpp
├── order_book.h
├── variables.h
├── CMakeLists.txt
└── README.md
