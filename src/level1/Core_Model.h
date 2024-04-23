//
// Created by ben on 3/31/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_CORE_MODEL_H
#define SYNTHETICTRAFFICGENERATOR_CORE_MODEL_H

#include "../RandomGenerator.h"
#include <string>

class Core_Model {
private:
    int id;
    RandomGenerator::CustomDistribution *destination_dist;
    std::map<int, RandomGenerator::CustomDistribution*>request_packet_type_dist;
    std::map<int, RandomGenerator::CustomDistribution*>reply_packet_type_dist;
    RandomGenerator::CustomDistribution *processing_delay_dist;
public:
    Core_Model(int);
    ~Core_Model();
    int get_id();
    void set_destination_dist(RandomGenerator::CustomDistribution*);
    void set_processing_delay(RandomGenerator::CustomDistribution*);
    void set_packet_type_dist(std::map<int, RandomGenerator::CustomDistribution*>, std::string);
    int generate_destination();
    int generate_processing_delay();
    int generate_request_packet_type(int);
    int generate_reply_packet_type(int);
    void show_model();
};


#endif
