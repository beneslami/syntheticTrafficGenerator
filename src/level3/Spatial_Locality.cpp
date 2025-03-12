//
// Created by ben on 9/11/24.
//

#include "Spatial_Locality.h"
#include <cassert>

Spatial_Locality::Spatial_Locality(){

}

Spatial_Locality::~Spatial_Locality(){

}

void Spatial_Locality::add_core_instance(Core_Model *core){
    this->coreArray.push_back(core);
}

Core_Model *Spatial_Locality::get_core_instance(int id) {
    Core_Model *core = this->coreArray[id];
    assert(core->get_id() == id);
    return core;
}

void Spatial_Locality::set_source_distribution(RandomGenerator::CustomDistribution *dist){
    this->source_distribution = dist;
}

int Spatial_Locality::generate_source(){
    return this->source_distribution->Generate();
}

void Spatial_Locality::set_reply_window(RandomGenerator::CustomDistribution *dist){
    this->reply_window = dist;
}

void Spatial_Locality::set_request_window(RandomGenerator::CustomDistribution *dist) {
    this->request_window = dist;
}

int Spatial_Locality::generate_reply_window() {
    return this->reply_window->Generate();
}

int Spatial_Locality::generate_request_window() {
    return this->request_window->Generate();
}

void Spatial_Locality::show_model(){
    std::cout << "source distribution:\n";
    this->source_distribution->show_cdf();
    std::vector<Core_Model*>::iterator it;
    for(it = this->coreArray.begin(); it != this->coreArray.end(); ++it){
        Core_Model *core = *it;
        core->show_model();
    }
}