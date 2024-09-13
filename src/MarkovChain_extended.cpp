//
// Created by ben on 9/11/24.
//

#include "MarkovChain_extended.h"
#include <cassert>

MarkovChain_extended::MarkovChain_extended(int core_id, int history){
    this->id = core_id;
    this->history = history;
}

MarkovChain_extended::~MarkovChain_extended(){

}

void MarkovChain_extended::set_state(std::map<std::string, RandomGenerator::CustomDistribution*>dist){
    this->states = dist;
    std::map<std::string, RandomGenerator::CustomDistribution*>::iterator it = this->states.begin();
    this->current_state = it->first;
}

void MarkovChain_extended::show(){

}

int MarkovChain_extended::generate_next(){
    int result = -1;
    std::map<std::string, RandomGenerator::CustomDistribution*>::iterator it;
    for(it = this->states.begin(); it != this->states.end(); ++it){
        if(it->first == this->current_state){
            result = it->second->Generate();
            shift_window(result);
            break;
        }
    }
    return result;
}

void MarkovChain_extended::shift_window(int new_value) {
    std::string::iterator it;
    std::string temp = "";
    bool flag = 0;
    for(it = this->current_state.begin(); it != this->current_state.end(); ++it){
        if(flag == 0){
            flag = 1;
        }
        else{
            temp += *it;
        }
    }
    temp += std::to_string(new_value);
    assert(int(temp.length()) == this->history);
    this->current_state = temp;
}