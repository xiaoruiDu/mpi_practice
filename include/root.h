#ifndef _ROOT_H
#define _ROOT_H
#include <string.h>
#include <string>
#include <iostream>
#include <mpi.h>
#include "controller.h"

#include <iostream>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <algorithm>
#include <numeric>


using namespace std;


enum msg_type {
                SUB_NODE_BLOCK, \
                SUB_NODE_CURRENT_STATUS,\
                ALIGN_THE_SIMULATION_TABLE,\
                ALREADY_ROLLBACK,\
                UPDATE_SIMULATION_TABLE,\
                PAUSE_NODE_FINISH_DISTRIBUTING,\
                NEIGHBOR_MIGRATE,\
                SUB_NODE_FINISH_SIM
            };



class Root{
    public:
        Root();
        Root(MPI_Comm communicator_in);
        comm get_communicator_info(MPI_Comm communicator_in);
        int get_msg_type(string msg);
        string get_complete_msg_according_to_receiving(int receive_msgtype);

        void process_msg(string msg);
        void process_msg_SUB_NODE_BLOCK(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_SUB_NODE_CURRENT_STATUS(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_ALIGN_THE_SIMULATION_TABLE(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_ALREADY_ROLLBACK(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_UPDATE_SIMULATION_TABLE(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_PAUSE_NODE_FINISH_DISTRIBUTING(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_NEIGHBOR_MIGRATE(MPI_Status status_s, string send_back_to_subnodes_msg);
        void process_msg_SUB_NODE_FINISH_SIM(MPI_Status status_s, string send_back_to_subnodes_msg);
        
        void run();
    
    private:
        bool flag=true;
        bool receive_msg_from_only_one_node_flag = true;
        
        int pause_node = -1; 
        int comm_size;
        int comm_rank;
        int simulation_step_table[100]={0}; // need to modify it;

        vector<int> already_rollback_counter;
        vector<int> count_simulation_step_receive;
        vector<int> count_pause_nodes_finish_migrate;
        vector<int> count_ending;
        MPI_Comm communicator;

        int nodes_amount_except_pause_node;
        
        

};




#endif //_ROOT_H