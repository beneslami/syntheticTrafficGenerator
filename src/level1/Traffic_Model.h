//
// Created by ben on 3/30/24.
//

#ifndef B_OOKSIM2_TRAFFIC_MODEL_H
#define B_OOKSIM2_TRAFFIC_MODEL_H


#include "../RandomGenerator.h"
#include <fstream>
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

class Traffic_Model {
private:
    std::string traffic_model_path;
    class Temporal_Locality *request_temporal_locality;
    class Temporal_Locality *reply_temporal_locality;
public:
    Traffic_Model(std::string);
    ~Traffic_Model();
    void read_model_file();
    int generate_off_cycle(std::string);
    int generate_burst_duration(std::string);
    int generate_burst_volume(std::string, int);
    void show_model(std::string);
};


#endif //B_OOKSIM2_TRAFFIC_MODEL_H
