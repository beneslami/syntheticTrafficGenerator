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
    int current_state;
    int initial_flag = 0;
    int trace_back_state(int);
public:
    MarkovChain();
    ~MarkovChain();
    void set_state(int state, RandomGenerator::CustomDistribution*);
    void show();
    int get_next_state();
    int get_current_state();
    void set_next_state(int);
};


#endif
