//
// Created by ben on 3/31/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_CORE_MODEL_H
#define SYNTHETICTRAFFICGENERATOR_CORE_MODEL_H

#include "../RandomGenerator.h"

class Core_Model {
private:
    int id;
    RandomGenerator::CustomDistribution *destination_dist;
    RandomGenerator::CustomDistribution *processing_delay_dist;
public:
    Core_Model(int);
    ~Core_Model();
    int get_id();
    void set_destination_dist(RandomGenerator::CustomDistribution*);
    void set_processing_delay(RandomGenerator::CustomDistribution*);
    int get_destination();
    int get_processing_delay();
};


#endif
