#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "simulator.h"
#include <map>
#include "mpi.h"
#include <string.h>
#include <string>
#include <unistd.h>


using namespace std;

#define MSG_LENGTH 8



/***
 
serialized message interpretation:

0000000:

position 0,1:                       who send this message
position 2,3:                       who will receive the message
(if position 2,3 == "--"")          all node will receive this msg except the sender.

position 1,2,3:                     the message type


ex:0001001

00:      rank 0 send the message;
01:      rank 1 receive the message;

//root node will process the data with msg type below;

001：     continue the self simulation;
002:      I am gonna block myself; (sent it to root)
003:      I send my current status back (send it to root)
004:      I already alligned with the simulation table,
          (make sure there is only one node bloked in the simulation table)
          send the newest step to root after 004

005:      tell root that rollback is already finished.
006:      update simulation_step_table.
007:      pause_nodes already finish distributing the agent to other nodes.
008:      tell root I already finished the simulation.


ex:00--009

00:      rank 0 send the message;
01:      all node will receive message from node 0;


//sub node will process the data with msg type below;

999:     root requires to block the sub nodes progress.
998:     root requires to get the current sub nodes status,and send simulation_table back to all nodes
997:     root send new simulation table to root nodes(process self_block=true nodes)
996:     allow nodes to migrate agent(set distribute=true)  
995:     sub nodes distribute agents to neigbors and add nodes from neighbors into its list
994:     all sub nodes are at the same simulation step and are already for next step
993:     the simulation finished successfully.

***/

struct comm
{
    int rank;
    int size;
};

struct agent_mpi_structure{
    
    int agent_id = 0;
    int diameter = 0;
    float state_next_pos_x = 0;
    float state_next_pos_y = 0;
    float state_next_traj_x = 0;
    float state_next_traj_y = 0;
    float pos_x=0;
    float pos_y=0;
    float traj_x=0;
    float traj_y=0;
};




class SimulationController{

    public:
        SimulationController(map<string,map<string,float>> config_in,
                            Simulator *sim_in, 
                            MPI_Comm communicator_in,
                            int steps_n_out_in=100
                            );
        
        ~SimulationController();

        void bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator);

        void run();
        void process_message(string received_msg);
        void all_done();
        void next_step();
        void migrate_agents_op(map<int, migrate_ball_S> agent_to_move_in);
         void migrate_agents_op_neighbors(map<int, migrate_ball_S> agent_to_move_in);

        comm get_communicator_info(MPI_Comm communicator_in);

        MPI_Request *send_msg_to_itself(string type,float true_data, MPI_Request *request);
        void send_msg_to_root(string type,float true_data);
        
        void process_msg_from_itself(MPI_Request *request,string msg_type);
        void process_msg_from_root(string msg_type);
        void process_999_msg_from_root();
        void process_998_msg_from_root();
        void process_997_msg_from_root();
        void process_996_msg_from_root();
        void process_994_msg_from_root();
        void process_993_msg_from_root();
        void process_992_msg_from_root();
        
        void process_msg_from_neighbor_nodes(MPI_Status *status_s,string msg_type);
        void process_0000995_msg_from_neighbor(MPI_Status *status_s);
        void process_1111995_msg_from_neighbor(MPI_Status *status_s);

        void process_msg_from_other_all_node(MPI_Request *request);
        Ball* reconstruct_ball_agent(agent_mpi_structure agent_true_data);

        string generate_msg_type(int sender_rank,int receiver_rank,string msg);
        vector<map<int, Ball>> previous_status;
        vector<map<int, migrate_ball_S>> agents_waiting_to_move; //only 0 position save data

        // vector<map<int, migrate_ball_S>> previous_agents_waiting_to_move;

        bool update_simtable_before_rollback = false;

        int previouse_pause_step = -1; // the pause_node step
        bool self_blocked = false;

        int pause_node = -1; //keep to know who pause the simulation
        bool send_end_msg = true;
        





    
    private:
        map<string,map<string,float>> config;
        Simulator *simulator;
        MPI_Comm communicator;
        int step;
        bool done;
        map<int,int> progress_states;
        int steps_n;


        bool no_migrate;
        // self.previous_status = []  //public
        bool distribute;
        // self.agent_waiting_to_move = [0]  //public
        bool update_simulation_table;
        // self.previouse_pause_step = 0 //public

        MPI_Request request;
        int flag_set_wait_request=1;
        int neighbor_counter;

        comm comm_info;




};




#endif //_CONTROLLER_H