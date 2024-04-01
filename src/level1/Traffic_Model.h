//
// Created by ben on 3/30/24.
//

#ifndef B_OOKSIM2_TRAFFIC_MODEL_H
#define B_OOKSIM2_TRAFFIC_MODEL_H


#include "../RandomGenerator.h"
#include "../trafficmanager.hpp"
#include "Spatial_Locality.h"
#include "Temporal_Locality.h"
#include <fstream>
#include <vector>
#include <map>

class Traffic_Model {
private:
    int cycle;
    std::string traffic_model_path;
    class Temporal_Locality *request_temporal_locality;
    class Temporal_Locality *reply_temporal_locality;
    class Spatial_Locality *spatial_locality;
public:
    std::ofstream outTrace;
    Traffic_Model(std::string, std::string);
    ~Traffic_Model();
    void read_model_file();
    int generate_off_cycle(std::string);
    int generate_burst_duration(std::string);
    int generate_burst_volume(std::string, int);
    void show_model(std::string);
    Spatial_Locality *get_spatial_locality();
    int get_cycle();
};


#endif //B_OOKSIM2_TRAFFIC_MODEL_H
