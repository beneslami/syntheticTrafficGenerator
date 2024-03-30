//
// Created by Ben on 8/23/21.
//

#include "MarkovChain.h"
#include "RandomGenerator.h"

MarkovChain::MarkovChain() {

}

MarkovChain::~MarkovChain() {

}

void MarkovChain::set_state(int state, RandomGenerator::CustomDistribution *dist) {
    states.insert(std::pair<int, RandomGenerator::CustomDistribution*>(state, dist));
    if(initial_flag == 0){
        this->current_state = state;
        initial_flag = 1;
    }

}

void MarkovChain::show() {
    std::map<int, RandomGenerator::CustomDistribution*>::iterator it;
    for(it = states.begin(); it != states.end(); ++it){
        std::cout << "b: " << it->first << std::endl;
        it->second->show_cdf();
    }
}

int MarkovChain::get_next_state() {
    std::map<int, RandomGenerator::CustomDistribution*>::iterator it = states.find(this->current_state);
    int next_state;
    if(it == states.end()){ // Detects the absorbing state
        next_state = -1;
    }
    else if(it->second->size() == 1 && it->first == it->second->Generate()){ // Detects the absorbing self-loop state
        next_state = -1;
    }
    else {
        next_state = it->second->Generate();
        this->current_state = next_state;
    }
    return next_state;
}

int MarkovChain::get_current_state() {
    return this->current_state;
}

int MarkovChain::trace_back_state(int state) {
    std::map<int, RandomGenerator::CustomDistribution*>::iterator it;
    for(it = this->states.begin(); it != this->states.end(); ++it){
        if(it->second->is_in(state) && it->second->size() == 1){
            trace_back_state(it->first);
        }
        else{
            return it->first;
        }
    }
    return -1;
}