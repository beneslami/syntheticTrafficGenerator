//
// Created by Ben on 8/23/21.
//

#ifndef MARKOVCHAIN_H
#define MARKOVCHAIN_H
#include "globals.hpp"
#include "RandomGenerator.h"
#include <map>

class MarkovChain {
private:
    std::map<int, RandomGenerator::CustomDistribution*>states;
    int current_state = -1;
    int trace_back_state(int);
public:
    MarkovChain();
    ~MarkovChain();
    void set_state(std::map<int, RandomGenerator::CustomDistribution*>dist);
    void show();
    int generate_next();
};


#endif
