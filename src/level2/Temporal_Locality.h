//
// Created by ben on 3/30/24.
//

#ifndef B_OOKSIM2_TEMPORAL_LOCALITY_H
#define B_OOKSIM2_TEMPORAL_LOCALITY_H

#include "../RandomGenerator.h"
#include <map>

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

#endif //B_OOKSIM2_TEMPORAL_LOCALITY_H
