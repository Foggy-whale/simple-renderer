#pragma once
#include <random>

class Random {
private:
    std::mt19937 rng;
public:
    Random() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {}
    int operator()() { return rng(); }
    int operator()(int n) { return std::uniform_int_distribution<int>(0, n - 1)(rng); }
};