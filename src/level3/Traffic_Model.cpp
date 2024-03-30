//
// Created by ben on 3/30/24.
//

#include "Traffic_Model.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

Traffic_Model::Traffic_Model(std::string path) {
    this->traffic_model_path = path;
    this->request_temporal_locality = new class Temporal_Locality();
    //TODO: later, add the reply temporal locality
}

Traffic_Model::~Traffic_Model(){
    delete request_temporal_locality;
}

int Traffic_Model::generate_off_cycle(std::string subnet) {
    if(subnet == "request" || subnet == "req") {
        return request_temporal_locality->generate_iat();
    }
    else if(subnet == "reply" || subnet == "response" || subnet == "rep" || subnet == "resp") {
        return reply_temporal_locality->generate_iat();
    }
    return -1;
}

int Traffic_Model::generate_burst_duration(std::string subnet) {
    if(subnet == "request" || subnet == "req") {
        return request_temporal_locality->generate_burst_duration();
    }
    else if(subnet == "reply" || subnet == "response" || subnet == "rep" || subnet == "resp") {
        return reply_temporal_locality->generate_burst_duration();
    }
    return -1;
}

int Traffic_Model::generate_burst_volume(std::string subnet, int duration) {
    if(subnet == "request" || subnet == "req") {
        return request_temporal_locality->generate_burst_volume(duration);
    }
    else if(subnet == "reply" || subnet == "response" || subnet == "rep" || subnet == "resp") {
        return reply_temporal_locality->generate_burst_volume(duration);
    }
    return -1;
}

void Traffic_Model::show_model(std::string type) {
    if(type == "request" || type == "req") {
        request_temporal_locality->show_model();
    }
    else if(type == "reply" || type == "response" || type == "rep" || type == "resp") {
        reply_temporal_locality->show_model();
    }
}

int Traffic_Model::get_cycle() {
    return this->cycle;
}

void Traffic_Model::read_model_file() {
    std::ifstream modelFile(traffic_model_path);
    json data = json::parse(modelFile);
    for(json::iterator it = data.begin(); it != data.end(); ++it){ // Either temporal or spatial
        if(it.key() == "cycle"){
            this->cycle = atoi(it.value().dump().c_str());
        }
        else if(it.key() == "temporal"){
            for(json::iterator it2 = it->begin(); it2 != it->end(); ++it2){ // iat, burst duration and burst volume
                std::map<int, int>temp;
                if(it2.key() == "iat") {
                    MarkovChain *iat_markov = new MarkovChain();
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        std::map<int, RandomGenerator::CustomDistribution*>state;
                        int st = atoi(it3.key().c_str());
                        for(json::iterator it4 = it3->begin(); it4 != it3->end(); ++it4){
                            temp.insert(std::pair<int, int>(atoi(it4.key().c_str()), atoi(it4.value().dump().c_str())));
                        }
                        RandomGenerator::CustomDistribution *iat_dist = new RandomGenerator::CustomDistribution(temp);
                        iat_markov->set_state(st, iat_dist);
                        temp.clear();
                    }
                    request_temporal_locality->set_iat(iat_markov);
                }
                else if(it2.key() == "duration") {
                    MarkovChain *duration_markov = new MarkovChain();
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        std::map<int, RandomGenerator::CustomDistribution*>state;
                        int st = atoi(it3.key().c_str());
                        for(json::iterator it4 = it3->begin(); it4 != it3->end(); ++it4){
                            temp.insert(std::pair<int, int>(atoi(it4.key().c_str()), atoi(it4.value().dump().c_str())));
                        }
                        RandomGenerator::CustomDistribution *duration_dist = new RandomGenerator::CustomDistribution(temp);
                        duration_markov->set_state(st, duration_dist);
                        temp.clear();
                    }
                    request_temporal_locality->set_burst_duration(duration_markov);
                }
                else if (it2.key() == "volume") {
                    std::map<int, std::map<int, int> > vol_temp;
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        std::map<int, int>inner_temp;
                        for (json::iterator it4 = it3->begin(); it4 != it3->end(); ++it4) {
                            inner_temp.insert(std::pair<int, int>(atoi(it4.key().c_str()), atoi(it4.value().dump().c_str())));
                        }
                        vol_temp.insert(std::pair<int, std::map<int, int> >(atoi(it3.key().c_str()), inner_temp));
                    }
                    std::map < int, std::map < int, int > > ::iterator
                    it_temp;
                    std::map < int, RandomGenerator::CustomDistribution * > volume_dist;
                    for (it_temp = vol_temp.begin(); it_temp != vol_temp.end(); ++it_temp) {
                        RandomGenerator::CustomDistribution *vol_dist = new RandomGenerator::CustomDistribution(
                                it_temp->second);
                        volume_dist.insert(
                                std::pair<int, RandomGenerator::CustomDistribution *>(it_temp->first, vol_dist));
                    }
                    request_temporal_locality->set_burst_volume(volume_dist);
                }
            }
        }
    }
}
