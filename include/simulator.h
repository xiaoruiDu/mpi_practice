#ifndef _SIMULATOR_H
#define _SIMULATOR_H
#include "agent.h"
#include <vector>
#include <map>
#include "id_provider.h"
#include "world.h"
#include "ball.h"
#include <mpi.h>



using namespace std;


#define OBSERVE_NODE 100

struct comm_simulator
{
    int rank;
    int size;
};


struct migrate_ball_S{

    Ball *migrate_ball;
    int partition_id;

};

struct position
{
    float pos_x;
    float pos_y;
};


class Simulator
{
    public:
        Simulator( map<string,map<string,float>> config,MPI_Comm communicator_in, int partition_id=0);
        ~Simulator();
        void get_id_provider();
        void next_step();
        void ball_collide_check();
        void commit_states();
        void print_state();
        void add_agents(vector<agent_S> agents_list,bool ignore_remote_agent, vector<agent_S> migrate_list=default_migrate_list);
        void add_agents_new(Ball *agents_ball, bool ignore_remote_agent);

        comm_simulator get_communicator_info(MPI_Comm communicator_in);
        
        map<int, migrate_ball_S> migrate_agents();
        // void print_agent_S(map<int,agent_S> agents);
        void print_agent_S_ball(map<int, Ball> agents);
        
        // map<int,agent_S> agents;
        map<int, Ball> agents;
        //
        map<int, migrate_ball_S> agents_corresponding_to_previous_status;


        

    private:

        MPI_Comm communicator;
        comm_simulator comm_info;
        bool ignore_remote_agent;
        static const vector<agent_S> default_migrate_list;
        vector<agent_S> migrate_ball_list;

        IDProvider *idprovider;
        id_package _id_provider;
        World *world;
       
        Partition *my_partition;
        int step;

        vector<position> previous_pos;
        vector<position> target_pos;
        vector<position> migrate_ball_pos_list;
        float ball_diameter;



        


};



#endif //_SIMULATOR_H










