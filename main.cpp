/* erin li
4/29/2022
source: https://en.cppreference.com/w/cpp/chrono
https://en.cppreference.com/w/cpp/chrono
https://riptutorial.com/cplusplus/example/1676/initializing-a-std--vector
github : https://github.com/ErinLi1998/erinpro2
*/
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cmath>
#include <queue>
#include <chrono>


//#include <cstdint>
//#include "process.h"
//#include "processor.h"


class Processor{
public:
    Processor(int id, int64_t speed, int memory)
        :processor_id(id), speed(speed), memory(memory){
        pre_exec_time = 0;
    }
    int processor_id;
    int64_t speed;
    int64_t memory;
    int64_t pre_exec_time;
};
class Memory {
public:
    Memory(int id)
            :memory_id(id){
        this->pre_exec_time = 0;
    }
    int memory_id;
    int64_t pre_exec_time;
};

class Process {
public:
    Process(int id, int64_t start, int64_t cost, int memory)
        :process_id(id), arrive_time(start), service_time(cost), require_memory(memory){
        this->last_exec_time = arrive_time;
        this->memory_ptr= nullptr;
    }
    int process_id;
    int64_t arrive_time;
    int64_t service_time;
    int64_t require_memory;
    int64_t last_exec_time;
    int memory_head_id;
    char* memory_ptr;
};

bool cmp_arrive( Process& p1,Process& p2)
{
    return p1.arrive_time < p2.arrive_time;
}

const int64_t INF=0x3f3f3f3f3f3f3f3f;

// Fifo
int doFIFO(std::vector<Process>& processes, std::vector<Processor>& processors){
    // sort by arrive time
    int64_t size = processes.size();
    for(Process& p : processes){
        // find a processor to exec
        int64_t pre_time = -1;
        Processor* select_p = nullptr;
        for(Processor& pr : processors){
            if(pr.memory >= p.require_memory){
                if(pre_time == -1 || pre_time > pr.pre_exec_time){
                    pre_time = pr.pre_exec_time;
                    select_p = &pr;
                }
            }
        }
        if(select_p == nullptr){
            continue;
        }
        int64_t start_time = std::max(p.arrive_time, select_p->pre_exec_time);
        // exec p
        p.last_exec_time = start_time + p.service_time;
        select_p->pre_exec_time = start_time + p.service_time;
    }
    return 0;
}

int doSC1(std::vector<Process>& processes, std::vector<Processor>& processors){
    // sort by arrive time
    int64_t size = processes.size();
    std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
    for(Process& p : processes){
        // find a processor to exec
        int64_t pre_time = -1;
        Processor* select_p = nullptr;
        for(Processor& pr : processors){
            if(pr.memory >= p.require_memory){
                if(pre_time == -1 || pre_time > pr.pre_exec_time){
                    pre_time = pr.pre_exec_time;
                    select_p = &pr;
                }
            }
        }
        if(select_p == nullptr){
            continue;
        }
        //alloc memory for p
        p.memory_ptr=(char*) malloc(p.require_memory*1000);
        int64_t start_time = std::max(p.arrive_time, select_p->pre_exec_time);
        // exec p
        p.last_exec_time = start_time + p.service_time;
        select_p->pre_exec_time = start_time + p.service_time;
    }
    //free memory
    for(Process& p : processes){
        free(p.memory_ptr);
    }
    std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
    std::chrono::duration<double,std::ratio<1,1000000000>> duration_ns=std::chrono::duration<double,std::ratio<1,1000000000>>(t2-t1);
    return duration_ns.count();
}
std::vector<Memory> init_memory_list(int memory_size){
    std::vector<Memory> memory_list;
    for(int i=0;i<memory_size;++i){
        memory_list.emplace_back(Memory(i));
    }
    return memory_list;
}
int my_alloc_first_fit(std::vector<Memory>& memory_list,int require_size,int alloc_time,int service_time){
    int find=0;//0-not find,-1-find,>0-hole size
    int now_head=-1;
    int64_t size = memory_list.size();
    for(int i=0;i<size;++i){
        if(alloc_time>=memory_list[i].pre_exec_time){
            find+=1;//find 1KB free
        }else{
            find=0;//be broken
            now_head=-1;
        }
        if(find==1){
            now_head=i;//save the head
        }
        if(find>=require_size){
            find=-1;//fit the require
            break;
        }
    }
    if(find==-1){
        for(int i=now_head;i<=now_head+require_size;++i){
            memory_list[i].pre_exec_time=alloc_time+service_time;//refresh each block time
        }
        return now_head;
    }else{
        return -1;
    }

}

int my_alloc_best_fit(std::vector<Memory>& memory_list,int require_size,int alloc_time,int service_time){
    int now_hole_size=0;
    int now_head=-1;


    int64_t size = memory_list.size();
    int best_hole_size=size+1;
    int best_head=-1;
    for(int i=0;i<size;++i){
        if(alloc_time>=memory_list[i].pre_exec_time){
            now_hole_size+=1;
        }else{
            if(now_hole_size>=require_size&&now_hole_size<best_hole_size){
                best_head=now_head;
                best_hole_size=now_hole_size;
            }
            now_hole_size=0;
            now_head=-1;
        }
        if(now_hole_size==1){
            now_head=i;
        }
    }

    if(best_head!=-1){
        for(int i=best_head;i<=best_head+require_size;++i){
            memory_list[i].pre_exec_time=alloc_time+service_time;//refresh time
        }
        return best_head;
    }else{
        if(now_hole_size>=require_size){//just one hole
            for(int i=now_head;i<=now_head+require_size;++i){
                memory_list[i].pre_exec_time=alloc_time+service_time;
            }
            return now_head;
        }
        return -1;
    }

}
int doSC2(std::vector<Process>& processes, std::vector<Processor>& processors,int alloc_type){
    std::vector<Memory> memory_list=init_memory_list(10000);
    std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
  }

int main()
{
    std::cout << "choose which of the four scenarios you wish to simulate: " << std::endl;
    std::cout << "1.SC1" << std::endl;
    std::cout << "2.SC2(Not finished yet)" << std::endl;
    std::cout << "3.SC3(Not finished yet)" << std::endl;
    std::cout << "4.SC4(Not finished yet)" << std::endl;
    int sc_type;
    std::cin >> sc_type;
    int allocate_type;
    if(sc_type!=1){
        std::cout << "choose which of the five allocate algorithms you wish to use" << std::endl;
        std::cout << "1.first-fit 2.best-fit" << std::endl;
        std::cin >> allocate_type;

    }
    std::cout << "which seed value you wish to use for the random number generator" << std::endl;
    int32_t seed;
    std::cin >> seed;

    int Ghz = 1000000000;

    // Choose a random values
    std::default_random_engine e1(seed);
    std::uniform_int_distribution<int64_t> uniform_service_time(200, 2000);
    std::normal_distribution<float> uniform_memory(200, 75);

    std::vector<Process> processes;
    int arr_time=0;
    for(int i = 0; i < 50; i ++){
        int64_t start = arr_time;
        arr_time+=100;
        int64_t cost = uniform_service_time(e1);
        int64_t memory = floor(uniform_memory(e1));
        processes.emplace_back(Process(i+1, start, cost, memory));//generate each process
    }
    std::vector<Processor> processors;
    int memory_size=10000;//10000KB=10MB
    processors.emplace_back(Processor(1, 15*Ghz,memory_size));
    processors.emplace_back(Processor(2, 15*Ghz,memory_size));
    processors.emplace_back(Processor(3, 15*Ghz,memory_size));
    processors.emplace_back(Processor(4, 15*Ghz,memory_size));
    int exec_time=-1;
    if(sc_type == 1){
        exec_time=doSC1(processes,processors);
        // all 3Ghz and 8GB
    }else if(sc_type == 2){

    }else if(sc_type == 3){

    }else{

    }
    // The list of processes in the order they were processed, and for each process
    // Its ID number,Its service time, Its memory requirements...
    std::cout << "Complete Time(ns): " <<exec_time<< std::endl;
    for(Process& p : processes){
        std::cout << "Process: " << p.process_id << "\tService time: " << p.service_time  << "\tMemory Requere: " << p.require_memory<<" KB ";
        if(sc_type==2){
            //memory allocate position
            std::cout <<"\tMemory Pos: "<<p.memory_head_id;
        }
        std::cout <<std::endl;
    }

}
