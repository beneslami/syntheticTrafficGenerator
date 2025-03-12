//
// Created by ben on 3/30/24.
//

#include "Traffic_Model.h"
#include "nlohmann/json.hpp"
#include <sstream>

using json = nlohmann::json;

int Traffic_Model::return_kernel_num(std::string path){
    std::queue<string> res;
    int pos = 0;
    while(pos < int(path.size())){
        pos = path.find("/");
        res.push(path.substr(0,pos));
        path.erase(0,pos + 1); // 3 is the length of the delimiter, "/"
    }
    while(res.size() != 2){
        res.pop();
    }
    std::string result = res.front();
    return atoi(result.c_str());
}

Traffic_Model::Traffic_Model(std::string path, std::string output_trace_name) {
    std::cout << "Initializing level2 traffic model " << std::endl;
    this->traffic_model_path = path;
    this->trace_file_path = output_trace_name;
    this->request_temporal_locality = new class Temporal_Locality();
    this->spatial_locality = new class Spatial_Locality();
    //TODO: later, add the reply temporal locality
}

Traffic_Model::~Traffic_Model(){
    delete request_temporal_locality;
    delete spatial_locality;
}

void Traffic_Model::init(int iteration) {
    std::ostringstream str;
    str << iteration;
    outTrace.open(this->trace_file_path + "trace" + str.str() + ".txt", std::ios::out);
}

void Traffic_Model::reinint() {
    outTrace.close();
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

void Traffic_Model::show_temporal_model(std::string type) {
    if(type == "request" || type == "req") {
        request_temporal_locality->show_model();
    }
    else if(type == "reply" || type == "response" || type == "rep" || type == "resp") {
        reply_temporal_locality->show_model();
    }
}

void Traffic_Model::show_spatial_model(std::string type) {
    if(type == "request" || type == "req") {
        spatial_locality->show_model();
    }
    else if(type == "reply" || type == "response" || type == "rep" || type == "resp") {

    }
}

Spatial_Locality *Traffic_Model::get_spatial_locality(){
    return this->spatial_locality;
}

int Traffic_Model::get_cycle() {
    return this->cycle;
}

int Traffic_Model::get_byte_granularity() {
    return this->byte_granularity;
}

void Traffic_Model::read_model_file() {
    std::ifstream modelFile(traffic_model_path);
    json data = json::parse(modelFile);
    this->byte_granularity = 1024;
    for(json::iterator it = data.begin(); it != data.end(); ++it){ // Either temporal or spatial
        if(it.key() == "cycle"){
            this->cycle = atoi(it.value().dump().c_str());
        }
        else if(it.key() == "temporal"){
            for(json::iterator it2 = it->begin(); it2 != it->end(); ++it2){ // iat, burst duration and burst volume
                std::map<int, int>temp;
                if(it2.key() == "iat") {
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        temp.insert(std::pair<int, int>(atoi(it3.key().c_str()), atoi(it3.value().dump().c_str())));
                    }
                    RandomGenerator::CustomDistribution *iat_dist = new RandomGenerator::CustomDistribution(temp);
                    request_temporal_locality->set_iat(iat_dist);
                    temp.clear();
                }
                else if(it2.key() == "duration") {
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        temp.insert(std::pair<int, int>(atoi(it3.key().c_str()), atoi(it3.value().dump().c_str())));
                    }
                    RandomGenerator::CustomDistribution *duration_dist = new RandomGenerator::CustomDistribution(temp);
                    request_temporal_locality->set_burst_duration(duration_dist);
                    temp.clear();
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
        else if(it.key() == "spatial"){
            for (json::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
                if(it2.key() == "request_window"){
                    std::map<int, int>request_window;
                    for(json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3){
                        request_window.insert(std::pair<int, int>(atoi(it3.key().c_str()), atoi(it3.value().dump().c_str())));
                    }
                    RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(request_window);
                    this->spatial_locality->set_request_window(dist);
                }
                else if(it2.key() == "source"){
                    std::map<int, int>source_dist;
                    for(json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3){
                        source_dist.insert(std::pair<int, int>(atoi(it3.key().c_str()), atoi(it3.value().dump().c_str())));
                    }
                    RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(source_dist);
                    this->spatial_locality->set_source_distribution(dist);
                }
                else if(it2.key() == "chip"){
                    for (json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3) {
                        int chip = atoi(it3.key().c_str());
                        Core_Model *core = new Core_Model(chip);
                        for(json::iterator it4 = it3->begin(); it4 != it3->end(); ++it4){
                            MarkovChain *markov = new MarkovChain();
                            if(it4.key() == "destination"){
                                std::map<int, RandomGenerator::CustomDistribution*>dest_markov;
                                for(json::iterator it5 = it4->begin(); it5 != it4->end(); ++it5){
                                    std::map<int, int>dest_dist;
                                    for(json::iterator it6 = it5->begin(); it6 != it5->end(); ++it6) {
                                        dest_dist.insert(std::pair<int, int>(atoi(it6.key().c_str()),atoi(it6.value().dump().c_str())));
                                    }
                                    RandomGenerator::CustomDistribution *temp_dist = new RandomGenerator::CustomDistribution(dest_dist);
                                    dest_markov.insert(std::pair<int, RandomGenerator::CustomDistribution*>(atoi(it5.key().c_str()), temp_dist));
                                }
                                markov->set_state(dest_markov);
                                core->set_destination_dist(markov);
                            }
                            else if(it4.key() == "request_packet"){
                                std::map<int, RandomGenerator::CustomDistribution*>packet_dist;
                                for(json::iterator it5 = it4->begin(); it5 != it4->end(); ++it5){
                                    int dest_chip = atoi(it5.key().c_str());
                                    std::map<int, int>packet_temp;
                                    for(json::iterator it6 = it5->begin(); it6 != it5->end(); ++it6){
                                        packet_temp.insert(std::pair<int, int>(atoi(it6.key().c_str()), atoi(it6.value().dump().c_str())));
                                    }
                                    RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(packet_temp);
                                    packet_dist.insert(std::pair<int, RandomGenerator::CustomDistribution*>(dest_chip, dist));
                                }
                                core->set_packet_type_dist(packet_dist, "req");
                            }
                            else if(it4.key() == "reply_packet"){
                                std::map<int, RandomGenerator::CustomDistribution*>packet_dist;
                                for(json::iterator it5 = it4->begin(); it5 != it4->end(); ++it5){
                                    int dest_chip = atoi(it5.key().c_str());
                                    std::map<int, int>packet_temp;
                                    for(json::iterator it6 = it5->begin(); it6 != it5->end(); ++it6){
                                        packet_temp.insert(std::pair<int, int>(atoi(it6.key().c_str()), atoi(it6.value().dump().c_str())));
                                    }
                                    RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(packet_temp);
                                    packet_dist.insert(std::pair<int, RandomGenerator::CustomDistribution*>(dest_chip, dist));
                                }
                                core->set_packet_type_dist(packet_dist, "rep");
                            }
                            else if(it4.key() == "latency"){
                                std::map<int, int>latency_dist;
                                for(json::iterator it5 = it4->begin(); it5 != it4->end(); ++it5){
                                    latency_dist.insert(std::pair<int, int>(atoi(it5.key().c_str()), atoi(it5.value().dump().c_str())));
                                }
                                RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(latency_dist);
                                core->set_processing_delay(dist);
                            }
                        }
                        if(core->return_smallest_packet() <= this->byte_granularity){
                            this->byte_granularity = core->return_smallest_packet();
                        }
                        spatial_locality->add_core_instance(core);
                    }
                }
                else if(it2.key() == "reply_window"){
                    std::map<int, int>reply_window;
                    for(json::iterator it3 = it2->begin(); it3 != it2->end(); ++it3){
                        reply_window.insert(std::pair<int, int>(atoi(it3.key().c_str()), atoi(it3.value().dump().c_str())));
                    }
                    RandomGenerator::CustomDistribution *dist = new RandomGenerator::CustomDistribution(reply_window);
                    this->spatial_locality->set_reply_window(dist);
                }
            }
        }
    }
}