//
// Created by ben on 3/31/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_COREMODEL_H
#define SYNTHETICTRAFFICGENERATOR_COREMODEL_H

#include "../RandomGenerator.h"
#include "Core_Model.h"
#include <vector>

class Spatial_Locality {
private:
    std::vector<Core_Model*>coreArray;
    RandomGenerator::CustomDistribution *source_distribution;
public:
    Spatial_Locality();
    ~Spatial_Locality();
    void add_core_instance(Core_Model*);
    Core_Model *get_core_instance(int);
    void set_source_distribution(RandomGenerator::CustomDistribution*);
    int generate_source();
};


#endif
