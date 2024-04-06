//
// Created by ben on 3/30/24.
//

#ifndef BOOKSIM2_MULTI_GPU_H
#define BOOKSIM2_MULTI_GPU_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <cmath>
#include <sstream>
#include <algorithm>
#include "flit.hpp"
#include "networks/network.hpp"
#include "booksim_config.hpp"
#include "trafficmanager.hpp"
#include "stats.hpp"

#if LEVEL == leve1
#include "level1/Traffic_Model.h"
#elif LEVEL == level2
#include "level2/Traffic_Model.h"
#elif LEVEL == level3
#include "level3/Traffic_Model.h"
#endif

#define  CORE  0x01
#define  L2    0x02
#define  DRAM  0x04
#define  ICNT  0x08
#define  ICNT_chLet 0x10
#define MhZ *1000000
#define gs_min2(a,b) (((a)<(b))?(a):(b))
#define min3(x,y,z) (((x)<(y) && (x)<(z))?(x):(gs_min2((y),(z))))

typedef struct mem_fetch{
    int src;
    int dest;
    Flit::FlitType type;
    int size;
    int id;
    int timestamp;
    int processing_time;
    int interval;
}mem_fetch;

class Multi_GPU {
private:
    int gpu_cycle;
    static int id_counter;
    double core_freq = 1132.0;
    double icnt_freq = 3277.0;
    double icnt_freq_chLet;             // Ring : 5625 for NVLink4 , 3750 for NVLink3, 1875 for NVLink2, 1000 for NVLink1
    double dram_freq = 8000.0;         // Torus: 2812 for NVLink4, 1875 for NVLink3, 937 for NVLink2, 500 for NVLink1
    double l2_freq = 4096.0;           // Mesh:  2812 for NVLink4, 1875 for NVLink3, 937 for NVLink2, 500 for NVLink1

    double core_period;
    double icnt_period;
    double icnt_chLet_period;
    double dram_period;
    double l2_period;

    double core_time;
    double icnt_time;
    double icnt_chLet_time;
    double dram_time;
    double l2_time;

    int subnet;
    int nodes;
    int vcs;

    std::vector<int>sum_throughput;
    int sum_tot_throughput;
    int iteration;

    int burst_duration;
    int burst_volume;
    int iat;

    long unsigned int pending_reply_capacity;
    long unsigned int ejection_buffer_capacity;
    long unsigned int boundary_buffer_capacity;
    long unsigned int input_buffer_capacity;
    long unsigned int processing_buffer_capacity;
    class _BoundaryBufferItem {
    public:
        _BoundaryBufferItem():_packet_n(0) {}
        inline unsigned Size(void) const { return _buffer.size(); }
        inline bool HasPacket() const { return _packet_n; }
        void* PopPacket();
        void* TopPacket() const;
        void PushFlitData(void* data,bool is_tail);

    private:
        queue<void *> _buffer;
        queue<bool> _tail_flag;
        int _packet_n;

    };
    typedef queue<Flit*> _EjectionBufferItem;

    std::vector<Stats*>throughput_per_chip;
    Stats *total_throughput;
    std::vector<int>byteArray;
    std::vector<queue<mem_fetch *> > _pending_reply;
    std::vector<vector<vector<_BoundaryBufferItem> > > _boundary_buffer;
    std::vector<vector<vector<_EjectionBufferItem> > > _ejection_buffer;
    std::vector<vector<queue<Flit* > > > _ejected_flit_queue;
    std::vector<std::vector<int> >_round_robin_turn;
    std::map<int, std::vector<mem_fetch*> >_processing_queue;
    double IcntToCoreRatio;
    void byte_spread_within_burst(int, int);
public:
    Multi_GPU();
    ~Multi_GPU();
    void set_link_frequency(std::string, int);
    double get_frequnecy_ratio();
    void init(BookSimConfig const & config);
    void run();
    void icnt_push(int, int, mem_fetch*);
    mem_fetch *generate_packet(int, int, int, int);
    mem_fetch *icnt_pop(int, int);
    void reset();
    void processing_queue_pop(int);
    void WriteOutBuffer(int, int, Flit*);
    void boundaryBufferTransfer(int, int);
    void process_request(int, mem_fetch*);
    void set_Pending_Reply_capacity(int);
    void set_boundary_buffer_capacity(int);
    void pending_reply_push(int, mem_fetch*);
    void pending_reply_pop(int);
    int get_received_queue_occupancy(int, int);
    bool boundary_buffer_isEmpty(int, int);
    bool boundary_buffer_isFull(int, int);
    bool drain_queues();
    bool pending_reply_isFull(int);
    bool pending_reply_isEmpty(int);
    void init_clock_domains();
    int next_clock_domain();
    int get_gpu_cycle();
    void reinit_clock_domains();
    void update_throughput();
    void add_throughput(int, int);
    void print_throughput(ostream &os = cout);
    void print_overall_throughput(ostream &os = cout);
    int get_input_buffer_capacity(){ return input_buffer_capacity; }
    int get_ejection_buffer_capacity(){ return ejection_buffer_capacity; }
    Flit *GetEjectedFlit(int , int);
    mem_fetch *pending_reply_front(int);
    inline double min4(double a, double b, double c, double d)
    {
        double smallest = min3(a,b,c);
        if (d < smallest)
            return d;
        else
            return smallest;
    }
};


#endif //BOOKSIM2_MULTI_GPU_H
