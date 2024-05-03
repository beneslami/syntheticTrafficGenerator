//
// Created by ben on 3/30/24.
//

#include "Multi_GPU.h"
#include "globals.hpp"
#include "trafficmanager.hpp"


int Multi_GPU::id_counter = 0;

Multi_GPU::Multi_GPU(){

}

Multi_GPU::~Multi_GPU(){

}

void Multi_GPU::update_throughput() {
    for(int i = 0; i < this->nodes; ++i){
        sum_throughput[i] += this->throughput_per_chip[i]->Average();
    }
    sum_tot_throughput += this->total_throughput->Average();
    iteration++;
}

void Multi_GPU::reset() {
    this->_ejection_buffer.resize(subnet);
    this->_boundary_buffer.resize(subnet);
    this->_round_robin_turn.resize(subnet);
    this->_ejected_flit_queue.resize(subnet);

    for(int i = 0 ; i < subnet; i++){
        this->_boundary_buffer[i].resize(nodes);
        this->_ejection_buffer[i].resize(nodes);
        this->_round_robin_turn[i].resize(nodes);
        this->_ejected_flit_queue[i].resize(nodes);

    }

    for(int i = 0 ; i < subnet; i++) {
        for (int j = 0; j < nodes; j++) {
            this->_boundary_buffer[i][j].resize(vcs);
            this->_ejection_buffer[i][j].resize(vcs);
        }
    }
    _pending_reply.resize(nodes);

    byteArray.clear();
    total_throughput->Clear();
    this->gpu_cycle = 0;

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
    this->reinit_clock_domains();
    this->init_clock_domains();
    this->IcntToCoreRatio = (double)(this->icnt_freq_chLet)/(this->core_freq);
}

double Multi_GPU::get_frequnecy_ratio(){
    return IcntToCoreRatio;
}

void Multi_GPU::init(BookSimConfig const & config){
    this->subnet = config.GetInt("subnets");
    this->nodes = pow(config.GetInt("k"), config.GetInt("n"));
    this->vcs = config.GetInt("num_vcs");
    this->_ejection_buffer.resize(subnet);
    this->_boundary_buffer.resize(subnet);
    this->_round_robin_turn.resize(subnet);
    this->_ejected_flit_queue.resize(subnet);

    for(int i = 0 ; i < subnet; i++){
        this->_boundary_buffer[i].resize(nodes);
        this->_round_robin_turn[i].resize(nodes);
        this->_ejected_flit_queue[i].resize(nodes);
        this->_ejection_buffer[i].resize(nodes);
    }

    for(int i = 0 ; i < subnet; i++) {
        for (int j = 0; j < nodes; j++) {
            this->_boundary_buffer[i][j].resize(vcs);
            this->_ejection_buffer[i][j].resize(vcs);
        }
    }
    _pending_reply.resize(nodes);
    this->throughput_per_chip.resize(this->nodes, 0);
    for(int i = 0; i < nodes; ++i){
        ostringstream s;;
        s << "chip_" << i;
        this->throughput_per_chip[i] = new Stats(trafficManager, s.str().c_str(),1.0, 1000);
    }
    this->total_throughput = new Stats(trafficManager, "total_throuhgput", 1.0, 1000);
    sum_throughput.resize(nodes, 0);
    this->sum_tot_throughput = 0;
    this->iteration = 0;
    this->pending_reply_capacity = config.GetInt("pending_buffer_size");
    this->ejection_buffer_capacity = config.GetInt("ejection_buffer_size");
    this->boundary_buffer_capacity = config.GetInt("boundary_buffer_size");
    this->processing_buffer_capacity = config.GetInt("processing_buffer_size");
    this->input_buffer_capacity = config.GetInt("input_buffer_size");
    this->gpu_cycle = 0;
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
    double smallest = min4(core_time, icnt_time, icnt_chLet_time, dram_time);
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

int Multi_GPU::get_gpu_cycle() {
    return this->gpu_cycle;
}

bool Multi_GPU::pending_reply_isFull(int chiplet){
    return (_pending_reply[chiplet].size() + 1 <= pending_reply_capacity) ? 0 : 1;
}

bool Multi_GPU::pending_reply_isEmpty(int chiplet){
    return (_pending_reply[chiplet].size() != 0) ? 0 : 1;
}

bool Multi_GPU::boundary_buffer_isEmpty(int subnet, int chiplet){
    int turn = _round_robin_turn[subnet][chiplet];
    return (_boundary_buffer[subnet][chiplet][turn].Size() != 0) ? 0 : 1;
}

bool Multi_GPU::boundary_buffer_isFull(int subnet, int chiplet){
    int turn = _round_robin_turn[subnet][chiplet];
    return (_boundary_buffer[subnet][chiplet][turn].Size() + 1 <= boundary_buffer_capacity) ? 0 : 1;
}

void Multi_GPU::WriteOutBuffer(int subnet, int dest, Flit* f) {
    int vc = f->vc;
    assert(_ejection_buffer[subnet][dest][vc].size() < ejection_buffer_capacity);
    _ejection_buffer[subnet][dest][vc].push(f);
}

void Multi_GPU::boundaryBufferTransfer(int subnet, int chiplet) {
    for(int vc = 0; vc < this->vcs; vc++) {
        if (!_ejection_buffer[subnet][chiplet][vc].empty() && _boundary_buffer[subnet][chiplet][vc].Size() < boundary_buffer_capacity) {
            Flit *f = _ejection_buffer[subnet][chiplet][vc].front();
            assert(f);
            _ejection_buffer[subnet][chiplet][vc].pop();
            _boundary_buffer[subnet][chiplet][vc].PushFlitData(f->data, f->tail);
            _ejected_flit_queue[subnet][chiplet].push(f); // for flow control
            if (f->head) {
                assert(f->dest == chiplet);
            }
        }
    }
}

void Multi_GPU::process_request(int input, mem_fetch *mf) {
    int chip = mf->dest;
    assert(input == chip);
    Core_Model *dest_chip = trafficModel->get_spatial_locality()->get_core_instance(chip);
    int processing_delay = dest_chip->generate_processing_delay();
    int size = -1;
    if(mf->type == Flit::READ_REQUEST){
        size = dest_chip->generate_reply_packet_type(mf->src);
        while(size == 8){
            size = dest_chip->generate_reply_packet_type(mf->src);
        }
    }
    else{
        size = 8;
    }
    mem_fetch *reply_mf = this->generate_packet(mf->dest, mf->src, size, 1);
    reply_mf->id = mf->id;
    reply_mf->processing_time = processing_delay;
    reply_mf->timestamp = this->gpu_cycle;
    this->_processing_queue[chip].push_back(reply_mf);
    delete mf;
    mf = NULL;
}

void Multi_GPU::set_Pending_Reply_capacity(int capacity){
    pending_reply_capacity = capacity;
}

void Multi_GPU::set_boundary_buffer_capacity(int chiplet){
    boundary_buffer_capacity = chiplet;
}

void Multi_GPU::pending_reply_push(int chiplet, mem_fetch *mf) {
    //assert(_pending_reply[chiplet].size() < pending_reply_capacity);
    _pending_reply[chiplet].push(mf);
}

void Multi_GPU::pending_reply_pop(int chiplet){
    assert(_pending_reply[chiplet].size() > 0);
    _pending_reply[chiplet].pop();
}

mem_fetch* Multi_GPU::pending_reply_front(int chiplet){
    assert(_pending_reply[chiplet].size() > 0);
    mem_fetch *mf = _pending_reply[chiplet].front();
    return mf;
}

void Multi_GPU::icnt_push(int src, int dest, mem_fetch *mf){
    int subnet = 0;
    if(mf->type == Flit::READ_REQUEST || mf->type == Flit::WRITE_REQUEST){
        subnet = 0;
    } else{
        subnet = 1;
    }
    trafficManager->_GeneratePacket(src, mf->size, dest, mf->type, subnet, 0, GetSimTime(), (void *) mf);
}

mem_fetch* Multi_GPU::icnt_pop(int subnet, int chiplet){
    mem_fetch *mf = NULL;
    int turn = _round_robin_turn[subnet][chiplet];
    for(int vc = 0; vc < this->vcs && (mf == NULL); vc++) {
        if (_boundary_buffer[subnet][chiplet][turn].HasPacket()) {
            mf = static_cast<mem_fetch *>(_boundary_buffer[subnet][chiplet][turn].PopPacket());
            mf->timestamp = gpu_cycle;
            assert(mf);
        }
        turn++;
        if (turn == this->vcs) {
            turn = 0;
        }
    }
    if (mf) {
        _round_robin_turn[subnet][chiplet] = turn;
    }
    return mf;
}

Flit* Multi_GPU::GetEjectedFlit(int subnet, int node){
    Flit* flit = NULL;
    if (!_ejected_flit_queue[subnet][node].empty()) {
        flit = _ejected_flit_queue[subnet][node].front();
        _ejected_flit_queue[subnet][node].pop();
    }
    return flit;
}

void Multi_GPU::processing_queue_pop(int chiplet){
    mem_fetch *return_mf = NULL;
    std::map<int, std::vector<mem_fetch*> >::iterator it = _processing_queue.find(chiplet);
    if(it != _processing_queue.end()){
        assert(chiplet == it->first);
        std::vector<mem_fetch*>::iterator it2;
        for(it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            return_mf = *it2;
            if (this->gpu_cycle - return_mf->timestamp >= return_mf->processing_time) {
                if(1/*TODO !pending_reply_isFull(chiplet)*/){
                    pending_reply_push(chiplet, return_mf);
                    this->_processing_queue[chiplet].erase(std::remove(this->_processing_queue[chiplet].begin(), this->_processing_queue[chiplet].end(), return_mf), this->_processing_queue[chiplet].end());
                    break;
                }
            }
        }
    }
}

int Multi_GPU::get_received_queue_occupancy(int subnet, int node){
    int occupancy = 0;
    for(int vc = 0; vc < this->vcs; vc++){
        occupancy += _boundary_buffer[subnet][node][vc].Size();
    }
    return occupancy;
}

void Multi_GPU::_BoundaryBufferItem::PushFlitData(void* data,bool is_tail){
    _buffer.push(data);
    _tail_flag.push(is_tail);
    if (is_tail) {
        _packet_n++;
    }
}

void* Multi_GPU::_BoundaryBufferItem::TopPacket() const{
    assert (_packet_n);
    void* data = NULL;
    void* temp_d = _buffer.front();
    while (data==NULL) {
        if (_tail_flag.front()) {
            data = _buffer.front();
        }
        assert(temp_d == _buffer.front()); //all flits must belong to the same packet
    }
    return data;
}

void* Multi_GPU::_BoundaryBufferItem::PopPacket(){
    assert (_packet_n);
    void * data = NULL;
    void * flit_data = _buffer.front();
    while (data == NULL) {
        assert(flit_data == _buffer.front()); //all flits must belong to the same packet
        if (_tail_flag.front()) {
            data = _buffer.front();
            _packet_n--;
        }
        _buffer.pop();
        _tail_flag.pop();
    }
    return data;
}

mem_fetch *Multi_GPU::generate_packet(int src, int dst, int size, int subnet){
    mem_fetch *mf = new mem_fetch;
    mf->src = src;
    mf->dest = dst;
    mf->size = size;
    if(size == 8){
        if(subnet == 1){
            mf->type = Flit::WRITE_REPLY;
        }
        else {
            mf->type = Flit::READ_REQUEST;
            mf->id = Multi_GPU::id_counter++;
        }
    }
    else{
        if(subnet == 1){
            mf->type = Flit::READ_REPLY;
        }
        else {
            mf->type = Flit::WRITE_REQUEST;
            mf->id = Multi_GPU::id_counter++;
        }
    }
    return mf;
}

void Multi_GPU::add_throughput(int chip, int size) {
    this->throughput_per_chip[chip]->AddSample(size);
    this->total_throughput->AddSample(size);
}

void Multi_GPU::print_throughput(ostream &os) {
    for(int i = 0; i < this->nodes; ++i) {
        os << "GPU " << i << "\n"
           << "average throughput = " << (double)this->throughput_per_chip[i]->Average() << std::endl
           << "max throughput = " << this->throughput_per_chip[i]->Max() << std::endl
           << "variance = " << (double)this->throughput_per_chip[i]->Variance() << std::endl << std::endl;
    }
    os << "total average throughput = " << (double)this->total_throughput->Average() << std::endl
       << "total variance throughput = " << (double)this->total_throughput->Variance() << std::endl << std::endl;
}

void Multi_GPU::print_overall_throughput(ostream &os){
    for(int i = 0; i < this->nodes; ++i) {
        os << "GPU " << i << " average throughput = " << this->sum_throughput[i] / (double)this->iteration << std::endl;
    }
    os << "total average throughput = " << this->sum_tot_throughput / (double)this->iteration << std::endl;
}

bool Multi_GPU::drain_queues() {
    bool flag = false;

    //while(!flag){
    int clock_mask = next_clock_domain();

    if(clock_mask & ICNT_chLet){
        for(int subnet = 0; subnet < this->subnet; ++subnet){
            for(int chip = 0; chip < this->nodes; ++chip){
                mem_fetch *mf = icnt_pop(subnet, chip);
                if(mf) {
                    if (mf->type == Flit::READ_REPLY || mf->type == Flit::WRITE_REPLY) {
                        trafficModel->outTrace << "reply received\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                               << mf->id
                                               << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: " << chip
                                               << "\tsize: "
                                               << mf->size << "\tq: " << get_received_queue_occupancy(subnet, chip)
                                               << std::endl;
                        this->throughput_per_chip[chip]->AddSample(mf->size);
                        this->total_throughput->AddSample(mf->size);
                        delete mf;
                        mf = NULL;
                    } else {
                        trafficModel->outTrace << "request received\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                               << mf->id
                                               << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: " << chip
                                               << "\tsize: " << mf->size << "\tq: "
                                               << get_received_queue_occupancy(subnet, chip) << std::endl;
                        this->throughput_per_chip[chip]->AddSample(mf->size);
                        this->total_throughput->AddSample(mf->size);
                        process_request(chip, mf);
                    }
                }
            }
        }
    }

    if(clock_mask & CORE){
        // pop from m_response_fifo. the packet is already in the chiplet
    }

    if(clock_mask & ICNT){
        // local transactions from LLC to cluster
    }

    if(clock_mask & ICNT_chLet){
        for(int chip = 0; chip < this->nodes; ++chip){
            if(!pending_reply_isEmpty(chip)){
                mem_fetch *mf = pending_reply_front(chip);
                if(mf){
                    if(1 /*TODO: check if the pacrtial packet is not full*/){
                        trafficModel->outTrace << "reply injected\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                               << mf->id
                                               << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: "
                                               << chip
                                               << "\tsize: " << mf->size << "\tq: "
                                               << trafficManager->get_partial_packet_occupancy(1, chip, 0) << std::endl;
                        icnt_push(mf->src, mf->dest, mf);
                        pending_reply_pop(chip);
                    }
                }
            }
        }
    }

    if(clock_mask & DRAM){
        // DRAM transactions happen here
    }

    if(clock_mask & L2){
        // pop remote request from the queue
        for(int input = 0; input < this->nodes; input++){
            processing_queue_pop(input);
        }
    }

    if(clock_mask & ICNT){
        // internal interconnect
    }

    if(clock_mask & ICNT_chLet){
        trafficManager->_Step();
    }

    if(clock_mask & CORE){
        //no request will be generated at this level.
        this->gpu_cycle++;
    }

    flag = true;
    for(int sub = 0; sub < this->subnet; sub++){
        for (int input = 0; input < this->nodes; input++){
            for(int vc = 0; vc < this->vcs; vc++){
                if (_boundary_buffer[sub][input][vc].HasPacket() &&
                        !_ejected_flit_queue[sub][input].empty() &&
                        !_ejection_buffer[sub][input][vc].empty() &&
                        !_processing_queue[input].empty() &&
                        !_pending_reply[input].empty() && trafficManager->check_if_any_packet_to_drain()) {
                    flag = false;
                }
            }
        }
    }
    //}
    return flag;
}

void Multi_GPU::run(){
    bool burst_state = 1;
    bool on_state = 0;
    bool off_state = 0;
    int begin_off_cycle = 0;
    int begin_on_cycle = 0;
    do{
        int clock_mask = next_clock_domain();

        if(clock_mask & ICNT_chLet){
            for(int subnet = 0; subnet < this->subnet; ++subnet){
                for(int chip = 0; chip < this->nodes; ++chip){
                    mem_fetch *mf = icnt_pop(subnet, chip);
                    if(mf) {
                        if (mf->type == Flit::READ_REPLY || mf->type == Flit::WRITE_REPLY) {
                            trafficModel->outTrace << "reply received\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                     << mf->id
                                     << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: " << chip
                                     << "\tsize: "
                                     << mf->size << "\tq: " << get_received_queue_occupancy(subnet, chip)
                                     << std::endl;
                            this->throughput_per_chip[chip]->AddSample(mf->size);
                            this->total_throughput->AddSample(mf->size);
                            delete mf;
                            mf = NULL;
                        } else {
                            trafficModel->outTrace << "request received\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                     << mf->id
                                     << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: " << chip
                                     << "\tsize: " << mf->size << "\tq: "
                                     << get_received_queue_occupancy(subnet, chip) << std::endl;
                            this->throughput_per_chip[chip]->AddSample(mf->size);
                            this->total_throughput->AddSample(mf->size);
                            process_request(chip, mf);
                        }
                    }
                }
            }
        }

        if(clock_mask & CORE){
            // pop from m_response_fifo. the packet is already in the chiplet
        }

        if(clock_mask & ICNT){
            // local transactions from LLC to cluster
        }

        if(clock_mask & ICNT_chLet){
            for(int chip = 0; chip < this->nodes; ++chip){
                if(!pending_reply_isEmpty(chip)){
                    mem_fetch *mf = pending_reply_front(chip);
                    if(mf){
                        if(1 /*TODO: check if the pacrtial packet is not full*/){
                            trafficModel->outTrace << "reply injected\tsrc: " << mf->src << "\tdst: " << mf->dest << "\tID: "
                                     << mf->id
                                     << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: "
                                     << chip
                                     << "\tsize: " << mf->size << "\tq: "
                                     << trafficManager->get_partial_packet_occupancy(1, chip, 0) << std::endl;
                            icnt_push(mf->src, mf->dest, mf);
                            pending_reply_pop(chip);
                        }
                    }
                }
            }
        }

        if(clock_mask & DRAM){
            // DRAM transactions happen here
        }

        if(clock_mask & L2){
            // pop remote request from the queue
            for(int input = 0; input < this->nodes; input++){
                processing_queue_pop(input);
            }
        }

        if(clock_mask & ICNT){
            // internal interconnect
        }

        if(clock_mask & ICNT_chLet){
            trafficManager->_Step();
        }

        if(clock_mask & CORE){
            if(burst_state == 1){
                if(on_state == 0){
                    this->burst_duration = trafficModel->generate_burst_duration("req");
                    this->burst_volume = trafficModel->generate_burst_volume("req", this->burst_duration);
                    this->byte_spread_within_burst(burst_duration, burst_volume);
                    begin_on_cycle = this->gpu_cycle;
                    on_state = 1;
                }
                if(on_state == 1){
                    assert(this->gpu_cycle >= begin_on_cycle);
                    int byte = this->byteArray[this->gpu_cycle - begin_on_cycle];
                    while(byte != 0) {
                        //spatial locality stuff is here
                        int src = trafficModel->get_spatial_locality()->generate_source();
                        int dst = trafficModel->get_spatial_locality()->get_core_instance(src)->generate_destination();
                        int byte_val = trafficModel->get_spatial_locality()->get_core_instance(src)->generate_request_packet_type(dst);
                        if(byte - byte_val >= 0) {
                            mem_fetch *mf = this->generate_packet(src, dst, byte_val, 0);
                            this->icnt_push(src, dst, mf);
                            trafficModel->outTrace << "request injected\tsrc: " << mf->src << "\tdst: " << mf->dest
                                                   << "\tID: " << mf->id
                                                   << "\ttype: " << mf->type << "\tcycle: " << gpu_cycle << "\tchip: "
                                                   << src << "\tsize: " << mf->size << "\tq: "
                                                   << trafficManager->get_partial_packet_occupancy(0, mf->src, 0)
                                                   << std::endl;
                            byte -= byte_val;
                        }
                    }

                    if(this->gpu_cycle - begin_on_cycle == this->burst_duration - 1){
                        on_state = 0;
                        burst_state = 0;
                        off_state = 0;
                        this->byteArray.clear();
                    }
                }
            }
            if(burst_state == 0){
                if(off_state == 0) {
                    begin_off_cycle = this->gpu_cycle;
                    this->iat = trafficModel->generate_off_cycle("req");
                    off_state = 1;
                }
                if(off_state == 1){
                    if(this->gpu_cycle - begin_off_cycle == this->iat - 1){
                        burst_state = 1;
                        off_state = 0;
                        on_state = 0;
                    }
                }
            }
            this->gpu_cycle++;
        }
    }while(this->gpu_cycle <= trafficModel->get_cycle());
}

void Multi_GPU::byte_spread_within_burst(int length, int volume) {
    this->byteArray.resize(length, 0);
    std::vector<int>::iterator it;
    it = this->byteArray.begin();
    while(volume > 0){
        if(it != this->byteArray.end()){
            *it += trafficModel->get_byte_granularity();
            ++it;
            volume -= trafficModel->get_byte_granularity();
        }
        else{
            it = this->byteArray.begin();
        }
    }
}