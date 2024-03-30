//
// Created by ben on 3/30/24.
//

#include "Multi_GPU.h"
#include "globals.hpp"
#include "trafficmanager.hpp"


Multi_GPU::Multi_GPU(){

}

Multi_GPU::~Multi_GPU(){

}

void Multi_GPU::set_link_frequency(std::string topo, int nvlink){
    if(topo == "ring"){
        switch (nvlink) {
            case 4:
                this->icnt_freq_chLet = 5625.0;
                break;
            case 3:
                this->icnt_freq_chLet = 3750.0;
                break;
            case 2:
                this->icnt_freq_chLet = 1875.0;
                break;
            case 1:
                this->icnt_freq_chLet = 1000.0;
                break;
        }
    }
    else if(topo == "mesh" || topo == "torus"){
        switch (nvlink) {
            case 4:
                this->icnt_freq_chLet = 2812.0;
                break;
            case 3:
                this->icnt_freq_chLet = 1875.0;
                break;
            case 2:
                this->icnt_freq_chLet = 937.0;
                break;
            case 1:
                this->icnt_freq_chLet = 500.0;
                break;
        }
    }
    else if(topo == "fly"){
        switch (nvlink) {
            case 4:
                this->icnt_freq_chLet = 11250.0;
                break;
            case 3:
                this->icnt_freq_chLet = 7500.0;
                break;
            case 2:
                this->icnt_freq_chLet = 3750.0;
                break;
            case 1:
                this->icnt_freq_chLet = 2000.0;
                break;
            default:
                std::cout << "error in frequency\n";
                this->icnt_freq_chLet = -1;
        }
    }
    this->IcntToCoreRatio = (double)(this->icnt_freq_chLet)/(this->core_freq);
}

double Multi_GPU::get_frequnecy_ratio(){
    return IcntToCoreRatio;
}

void Multi_GPU::init(BookSimConfig const & config){
    this->_ejection_buffer.resize(trafficManager->get_num_subnets());
    this->_boundary_buffer.resize(trafficManager->get_num_subnets());
    this->_round_robin_turn.resize(trafficManager->get_num_subnets());
    this->_ejected_flit_queue.resize(trafficManager->get_num_subnets());

    for(int i = 0 ; i < trafficManager->get_num_subnets(); i++){
        this->_boundary_buffer[i].resize(trafficManager->get_num_nodes());
        this->_round_robin_turn[i].resize(trafficManager->get_num_nodes());
        this->_ejected_flit_queue[i].resize(trafficManager->get_num_nodes());
        this->_ejection_buffer[i].resize(trafficManager->get_num_nodes());
    }

    for(int i = 0 ; i < trafficManager->get_num_subnets(); i++) {
        for (int j = 0; j < trafficManager->get_num_nodes(); j++) {
            this->_boundary_buffer[i][j].resize(trafficManager->get_num_vcs());
            this->_ejection_buffer[i][j].resize(trafficManager->get_num_vcs());
        }
    }
    _pending_reply.resize(trafficManager->get_num_nodes());
    this->pending_reply_capacity = config.GetInt("pending_buffer_size");
    this->ejection_buffer_capacity = config.GetInt("ejection_buffer_size");
    this->boundary_buffer_capacity = config.GetInt("boundary_buffer_size");
    this->processing_buffer_capacity = config.GetInt("processing_buffer_size");
    this->input_buffer_capacity = config.GetInt("input_buffer_size");
    reinit_clock_domains();
    init_clock_domains();
}

void Multi_GPU::init_clock_domains() {
    core_freq = core_freq MhZ;
    icnt_freq = icnt_freq MhZ;
    icnt_freq_chLet = icnt_freq_chLet MhZ;
    l2_freq = l2_freq MhZ;
    dram_freq = dram_freq MhZ;
    core_period = 1 / core_freq;
    icnt_period = 1 / icnt_freq;
    icnt_chLet_period = 1 / icnt_freq_chLet;
    dram_period = 1 / dram_freq;
    l2_period = 1 / l2_freq;
}

void Multi_GPU::reinit_clock_domains(){
    core_time = 0;
    dram_time = 0;
    icnt_time = 0;
    icnt_chLet_time = 0;
    l2_time = 0;
}

int Multi_GPU::next_clock_domain() {
    double smallest = min4(core_time, icnt_time, icnt_chLet_time, dram_time); //20220820 Shiqing
    int mask = 0x00;
    if (l2_time <= smallest) {
        smallest = l2_time;
        mask |= L2;
        l2_time += l2_period;
    }
    if (icnt_time <= smallest) {
        mask |= ICNT;
        icnt_time += icnt_period;
    }
    if (icnt_chLet_time <= smallest) {
        mask |= ICNT_chLet;
        icnt_chLet_time += icnt_chLet_period;
    }
    if (dram_time <= smallest) {
        mask |= DRAM;
        dram_time += dram_period;
    }
    if (core_time <= smallest) {
        mask |= CORE;
        core_time += core_period;
    }
    return mask;
}
