//
// Created by ben on 3/31/24.
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