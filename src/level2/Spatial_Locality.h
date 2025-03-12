//
// Created by ben on 3/31/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_SPATIAL_LOCALITY_H
#define SYNTHETICTRAFFICGENERATOR_SPATIAL_LOCALITY_H

#include "../RandomGenerator.h"
#include "Core_Model.h"
#include <vector>

class Spatial_Locality {
private:
    std::vector<Core_Model*>coreArray;
    RandomGenerator::CustomDistribution *source_distribution;
    RandomGenerator::CustomDistribution *reply_window;
    RandomGenerator::CustomDistribution *request_window;
public:
    Spatial_Locality();
    ~Spatial_Locality();
    void add_core_instance(Core_Model*);
    Core_Model *get_core_instance(int);
    void set_reply_window(RandomGenerator::CustomDistribution*);
    void set_request_window(RandomGenerator::CustomDistribution*);
    void set_source_distribution(RandomGenerator::CustomDistribution*);
    int generate_reply_window();
    int generate_request_window();
    int generate_source();
    void show_model();
};


#endif
