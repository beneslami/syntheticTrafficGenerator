//
// Created by ben on 3/31/24.
//

#include "Temporal_Locality.h"

Temporal_Locality::Temporal_Locality() {

}

Temporal_Locality::~Temporal_Locality() {

}

void Temporal_Locality::set_iat(RandomGenerator::CustomDistribution *dist) {
    this->iat = dist;
}

void Temporal_Locality::set_burst_duration(RandomGenerator::CustomDistribution *dist) {
    this->burst_duration = dist;
}

void Temporal_Locality::set_burst_volume(std::map<int, RandomGenerator::CustomDistribution *>dist) {
    this->burst_volume = dist;
}

int Temporal_Locality::generate_iat() {
    return this->iat->Generate();
}

int Temporal_Locality::generate_burst_duration() {
    return this->burst_duration->Generate();
}

int Temporal_Locality::generate_burst_volume(int duration) {
    return burst_volume[duration]->Generate();
}

void Temporal_Locality::show_model(){
    std::cout << "----Inter Arrival Time----\n";
    this->iat->show_cdf();
    std::cout << "----Burst Duration----\n";
    this->burst_duration->show_cdf();
    std::map<int, RandomGenerator::CustomDistribution*>::iterator it;
    for(it = burst_volume.begin(); it != burst_volume.end(); ++it) {
        std::cout << "----Burst volume " << it->first << "----\n";
        it->second->show_cdf();
    }
}