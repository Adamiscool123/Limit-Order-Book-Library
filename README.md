# Limit Order Book Library

A C++ limit order book and matching engine project built to simulate basic market microstructure concepts such as bid/ask books, order matching, partial fills, FIFO priority, and agent-driven order flow.

## Overview

This project implements a limit order book with a matching engine and multiple trading agent types. The goal is to model how exchange-style order books work while exploring data structures, matching logic, market simulation, and performance in C++.

The codebase is split into separate components for:

- order book management
- matching engine logic
- shared/global market state
- manual and automated trading agents
- benchmark/testing utilities

## Features

Current supported functionality:

- Buy and sell orders
- Limit order submission
- Basic matching engine
- Partial fills
- FIFO handling for equal-priced orders
- Manual order entry
- Agent-based order generation
- CSV latency benchmark output

Agent types currently included:

- Manual trader
- Market maker
- Noise trader
- Trend follower
- Whale

## Benchmarking and Testing

I added a small benchmarking harness to measure order submission latency under different order book scenarios. The benchmark records per-order latency in nanoseconds and exports the results to CSV files for analysis.

The benchmark currently tests three scenarios:

1. **Existing price level orders**  
   Orders are submitted at price levels that already exist in the book.

2. **New price level orders**  
   Orders are submitted outside the existing bid/ask ranges, creating new price levels.

3. **Crossing orders**  
   Orders are submitted aggressively enough to cross the spread and trigger matching.

Each benchmark records:

- latency in nanoseconds
- order price
- order quantity
- side: buy or sell
- current bid-side book length
- current ask-side book length
- whether the order crossed the book
- whether the order created a new price level

The benchmark functions generate `existing.csv`, `new.csv`, and `cross.csv` from the C++ test harness. The harness uses `std::chrono::high_resolution_clock` for timing and writes each measured order event to CSV. :contentReference[oaicite:0]{index=0}

## Benchmark Results

Current benchmark results:

| Scenario | Mean Latency | Median | 90% | 95% | 99% | 99.5% | Max |
|---|---:|---:|---:|---:|---:|---:|---:|
| Existing price level | 349.617 ns | 300 ns | 400 ns | 500 ns | 700 ns | 800 ns | 167,000 ns |
| New price level | 254.433 ns | 200 ns | 300 ns | 300 ns | 400 ns | 500 ns | 89,100 ns |
| Crossing order | 209.795 ns | 200 ns | 300 ns | 300 ns | 400 ns | 400 ns | 170,900 ns |

These results suggest that the typical order processing latency is in the low hundreds of nanoseconds for the tested scenarios, with occasional large outliers likely caused by system scheduling, cache effects, memory allocation, or OS noise.

## Why I Built This

I built this project to learn more about:

- market microstructure
- exchange matching engines
- order book design
- C++ systems programming
- algorithmic trading infrastructure
- latency measurement and benchmarking

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
