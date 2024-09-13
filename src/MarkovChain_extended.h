//
// Created by ben on 9/11/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_MARKOVCHAIN_EXTENDED_H
#define SYNTHETICTRAFFICGENERATOR_MARKOVCHAIN_EXTENDED_H

#include <string>
#include <map>
#include "RandomGenerator.h"

class MarkovChain_extended {
private:
    int id;
    int history;
    std::map<std::string, RandomGenerator::CustomDistribution*>states;
    std::string current_state;
    void shift_window(int);
public:
    MarkovChain_extended(int, int);
    ~MarkovChain_extended();
    void set_state(std::map<std::string, RandomGenerator::CustomDistribution*>dist);
    void show();
    int generate_next();
};


#endif