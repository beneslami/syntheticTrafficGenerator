//
// Created by ben on 3/31/24.
//

#include "Core_Model.h"

Core_Model::Core_Model(int id){
    this->id = id;
}

Core_Model::~Core_Model(){

}

int Core_Model::get_id(){
    return this->id;
}

void Core_Model::set_destination_dist(RandomGenerator::CustomDistribution *dist){
    this->destination_dist = dist;
}
void Core_Model::set_processing_delay(RandomGenerator::CustomDistribution *dist){
    this->processing_delay_dist = dist;
}

int Core_Model::get_destination(){
    return this->destination_dist->Generate();
}

int Core_Model::get_processing_delay(){
    return this->processing_delay_dist->Generate();
}