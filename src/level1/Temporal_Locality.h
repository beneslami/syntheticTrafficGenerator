//
// Created by ben on 3/31/24.
//

#ifndef SYNTHETICTRAFFICGENERATOR_TEMPORAL_LOCALITY_H
#define SYNTHETICTRAFFICGENERATOR_TEMPORAL_LOCALITY_H

#include "../RandomGenerator.h"
#include <vector>

class Temporal_Locality {
private:
    RandomGenerator::CustomDistribution *iat;
    RandomGenerator::CustomDistribution *burst_duration;
    std::map<int, RandomGenerator::CustomDistribution *>burst_volume;
public:
    Temporal_Locality();
    ~Temporal_Locality();
    void set_iat(RandomGenerator::CustomDistribution*);
    int generate_iat();
    void set_burst_duration(RandomGenerator::CustomDistribution*);
    int generate_burst_duration();
    void set_burst_volume(std::map<int, RandomGenerator::CustomDistribution *>);
    int generate_burst_volume(int);
    void show_model();
};


#endif //SYNTHETICTRAFFICGENERATOR_TEMPORAL_LOCALITY_H
