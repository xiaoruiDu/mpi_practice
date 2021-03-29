#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <map>
#include "include/agent.h"
#include "include/parsejson.h"
#include "include/simulator.h"
#include "include/controller.h"
#include "include/root.h"
#include <unistd.h>
#include <stdlib.h>
#include "mpi.h"

using namespace std;
int main()
{

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    
    MPI_Comm comm = MPI_COMM_WORLD;
    // Get the number of processes
    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    if(comm_size < 3)
    {
        printf("This application is meant to be run with over 3 MPI processes.\n");
        MPI_Abort(comm, EXIT_FAILURE);
    }

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    
    vector<agent_S> agent_list;
    map<string,float>  world_config;
    int simulation_step = 100;

    world_config["x_min"] = 0.0f;
    world_config["y_min"] = 0.0f;
    world_config["x_max"] = 100.0f;
    world_config["y_max"] = 100.0f;
    world_config["grid_width"] = 2.0f;
    world_config["grid_height"] = 2.0f;
    world_config["comm_size"] = (float)comm_size;  
                                                    
    map<string,map<string,float>> sim_config;
    sim_config["world_config"] = world_config;

    
    // individual processor 
    // if(rank == 0)
    if(rank != comm_size-1)
    {
        // Position *pos;
        ParseJson *parser;
        Simulator *sim;
        SimulationController *my_controller;
        
        parser = new ParseJson();
        // set up simulator instance for my partition
        sim = new Simulator(sim_config,comm, rank);
        // add some agents... printf("reconstruct_ball----start\n");
        agent_list = parser->get_agent_json_data("agent.json");

        sim->add_agents(agent_list,true);

        cout <<"rank/processors: " << rank <<"/"<< comm_size \
         << "--->agent_size:" << sim->agents.size()<<endl;
         
        // sim->print_agent_S_ball(sim->agents);

        // set up a distributed controller
        my_controller = new SimulationController(sim_config,sim,comm,simulation_step);
        my_controller->run();

    }
    // root processor 
    else
    {   
        Root *root = new Root(comm);
        printf("Process %d \n", rank);
        root->run();
    }

    // Finalize the MPI environment.
    MPI_Finalize();



}






