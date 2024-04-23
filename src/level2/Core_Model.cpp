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

void Core_Model::set_packet_type_dist(std::map<int, RandomGenerator::CustomDistribution*>dist, std::string type) {
    if(type == "request" || type == "req") {
        this->request_packet_type_dist = dist;
    }
    else if(type == "reply" || type == "rep"){
        this->reply_packet_type_dist = dist;
    }
}

int Core_Model::generate_destination(){
    return this->destination_dist->Generate();
}

int Core_Model::generate_processing_delay(){
    return this->processing_delay_dist->Generate();
}

int Core_Model::generate_request_packet_type(int chip) {
    return this->request_packet_type_dist[chip]->Generate();
}

int Core_Model::generate_reply_packet_type(int chip) {
    return this->reply_packet_type_dist[chip]->Generate();
}

void Core_Model::show_model() {
    std::cout << "----- chip " << this->id << std::endl;
    std::cout << "destination distribution:\n";
    this->destination_dist->show_cdf();
    std::cout << "\n";
    std::cout << "packet type distribution:\n";
    std::map<int, RandomGenerator::CustomDistribution*>::iterator it;
    for(it = this->request_packet_type_dist.begin(); it != this->request_packet_type_dist.end(); ++it){
        std::cout << "destination " << it->first << "\n";
        it->second->show_cdf();
    }
    std::cout << "\n";
    std::cout << "access latency distribution\n";
    this->processing_delay_dist->show_cdf();
}