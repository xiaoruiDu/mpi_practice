#include "controller.h"


SimulationController::SimulationController(map<string,map<string,float>> config_in,
                                            Simulator *sim_in, 
                                            MPI_Comm communicator_in,
                                            int steps_n_out_in)
{

    simulator = sim_in;
    config = config_in;
    communicator = communicator_in;
    done = false;
    progress_states.clear();
    step = 1;
    steps_n = steps_n_out_in;
    

    no_migrate = true;
    distribute = false;
    update_simulation_table = false;
    previous_status.push_back(simulator->agents);
    agents_waiting_to_move.clear();
    comm_info = get_communicator_info(communicator);
    neighbor_counter = comm_info.size - 2;


}





void SimulationController::bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator)
{
    comm comm_info = get_communicator_info(communicator);
    if (comm_info.rank == root) {
        int i;
        for (i = 0; i < comm_info.size; i++) {
        if (i != comm_info.rank) {
            // 0: tag
            MPI_Send(data, count, datatype, i, 0, communicator);
        }
        }
    }
    else {
        // If we are a receiver process, receive the data from the root
        MPI_Recv(data, count, datatype, root, 0, communicator,
                MPI_STATUS_IGNORE);
  }
}



void SimulationController::run(){

    comm comm_info = get_communicator_info(communicator);

    // while(--count)
    while(!done)
    {   
        //send msg to itself; message_type + true_data
        MPI_Request request;
        MPI_Request *request_rt; 
        if(no_migrate)
        {
            request_rt = send_msg_to_itself("0000001",2, &request);
        }

        if(distribute)
        {   
            distribute = false;
            // migrate agent
            migrate_agents_op(agents_waiting_to_move.front());
            agents_waiting_to_move.clear();
        }

        process_msg_from_other_all_node(request_rt);

        if(step >= steps_n){
            
            no_migrate = false;
            if(send_end_msg)
            {
                // send_end_msg = false;
                string message_type = "008"; //I am done
                string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,message_type);
                char message_type_char[MSG_LENGTH];
                strcpy(message_type_char,complete_msg.c_str()); 
                MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);
           
            }

        }
    }
    

}



void SimulationController::process_message(string received_msg)
{   
    comm comm_info = get_communicator_info(communicator);

    return;
}

void SimulationController::all_done(){

    return;

}
void SimulationController::next_step(){
    
    comm comm_info = get_communicator_info(communicator);

    
    simulator->commit_states();
    
    if(comm_info.rank == 1)
    {
        cout << comm_info.rank<<"  step_n(total):" << steps_n << "---> current_step:" << step <<endl;
    }

    simulator->next_step();

    map<int, migrate_ball_S> migrate_agents = simulator->migrate_agents();
    //save the previous satus
    previous_status.push_back(simulator->agents);

    step += 1;


    if(migrate_agents.size() > 0)
    {   

        self_blocked = true;
        no_migrate = false; // stop simulation

        if(agents_waiting_to_move.size()>=1)
        {
            agents_waiting_to_move.clear();
        }
        agents_waiting_to_move.push_back(migrate_agents);
  
        
        if(previouse_pause_step == -1)
        {
            // send block info to root node (blocking send)
            string message_type = "002"; //I am gonna block myself.
            string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,message_type);
            //1- send msg type (type="002") tag=0
            char message_type_char[MSG_LENGTH];
            strcpy(message_type_char,complete_msg.c_str()); 
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);
        }
        // update simulation step table
        else if(step < previouse_pause_step && previouse_pause_step != -1)
        {  //other pause_node reset the previouse_pause_step 

            string message_type = "006"; //update simulation_step_table.
            string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,message_type);

            // cout << "complete_msg:" << complete_msg << endl;
            //1- send msg type (type="002") tag=0
            char message_type_char[MSG_LENGTH];
            strcpy(message_type_char,complete_msg.c_str()); 
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);
            
            previouse_pause_step = -1;
            
            printf("rank:%d <------update_simulation_table----> \n",comm_info.rank);
        }

        else if(step >= previouse_pause_step)
        {
            // catch up the pausued_node. (and self_block = true)
            
            previouse_pause_step = -1;

            //tell root that rollback is already finished. 
            char message_type_char[MSG_LENGTH];
            string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,"005");
            strcpy(message_type_char,complete_msg.c_str());
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);
            // printf("rank: %d, already send 005 to root...\n", comm_info.rank);

        }
    }

    else if(migrate_agents.size() == 0 && step == previouse_pause_step)
    {
        // catch up the pausued_node. (and self_block = false)
        //tell root that rollback is already finished. 

        char message_type_char[MSG_LENGTH];
        string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,"005");
        strcpy(message_type_char,complete_msg.c_str());
        MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);

        no_migrate = false; // stop simulation and tell root that I already catched up the pause_node
        previouse_pause_step = -1;

    }

}
void SimulationController::migrate_agents_op(map<int, migrate_ball_S> agent_to_move_in)
{   

    //print_out migrates
    /*
    map<int, migrate_ball_S>::iterator itr_check;
    for(itr_check = agent_to_move_in.begin(); itr_check!=agent_to_move_in.end(); ++itr_check)
    {
        printf("rank:%d, --really start to migrate-- ball_id: %d\n",comm_info.rank,itr_check->first);
    }
    */


    //commit_own_mpi_data_structure: agent_mpi_structure

    // printf("rank: %d, move_agents_num: %lu\n",comm_info.rank,agent_to_move_in.size());
    MPI_Datatype agent_mpi_structure_type;
    int lengths[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const MPI_Aint displacements[10] = { 0, sizeof(int), 2*sizeof(int),\
                                        2*sizeof(int)+sizeof(float), \
                                        2*sizeof(int)+2*sizeof(float),\
                                        2*sizeof(int)+3*sizeof(float),\
                                        2*sizeof(int)+4*sizeof(float),\
                                        2*sizeof(int)+5*sizeof(float),\
                                        2*sizeof(int)+6*sizeof(float),\
                                        2*sizeof(int)+7*sizeof(float)};
    MPI_Datatype types[10] = { MPI_INT, MPI_INT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,\
                               MPI_FLOAT, MPI_FLOAT,MPI_FLOAT };
    MPI_Type_create_struct(10, lengths, displacements, types, &agent_mpi_structure_type);
    MPI_Type_commit(&agent_mpi_structure_type);

    comm comm_info = get_communicator_info(communicator);

    //Step0: send the message type;
    MPI_Request request;
    map<int, migrate_ball_S>::iterator itr;
    int migrate_size = agent_to_move_in.size();
    // printf("segmentation_check, migrate_size: %lu\n", agent_to_move_in.size());
 
    for(itr=agent_to_move_in.begin();itr!=agent_to_move_in.end();++itr){

        //ToDo: send the agent to the corresponding sub nodes;
        string msg_type_string = "0000995";
        char message_type_char[MSG_LENGTH];
        strcpy(message_type_char,msg_type_string.c_str());
        // MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR, itr->second.partition_id,0,communicator, &request);
        MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, itr->second.partition_id,0,communicator);

        //Step2: send the true data;(define the message structure;)
        struct agent_mpi_structure agent_true_data;
        agent_true_data.agent_id = itr->first;
        agent_true_data.diameter = itr->second.migrate_ball->diameter;
        
        agent_true_data.state_next_pos_x = itr->second.migrate_ball->state_next->position->x;
        agent_true_data.state_next_pos_y = itr->second.migrate_ball->state_next->position->y;
        agent_true_data.state_next_traj_x = itr->second.migrate_ball->state_next->trajectory->x;
        agent_true_data.state_next_traj_y = itr->second.migrate_ball->state_next->trajectory->y;

        agent_true_data.pos_x = itr->second.migrate_ball->state_current->position->x;
        agent_true_data.pos_y= itr->second.migrate_ball->state_current->position->y;
        agent_true_data.traj_x = itr->second.migrate_ball->state_current->trajectory->x;
        agent_true_data.traj_y = itr->second.migrate_ball->state_current->trajectory->y;

        MPI_Send(&agent_true_data,1,agent_mpi_structure_type,itr->second.partition_id,1,communicator);
       
    }

    // tell all neighbor that I already sent all migrates.(msg = 1111995)
    
    string msg_type_string = "1111995";
    char message_type_char[MSG_LENGTH];
    strcpy(message_type_char,msg_type_string.c_str());
    for(int m=0; m<comm_info.size-1; ++m)
    {
        if(comm_info.rank != m)
        {
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR,m ,0,communicator);  
        }
    }
    // printf("rank: %d alread_send_msg: 1111995 to neighbors\n",comm_info.rank);

}

void SimulationController::migrate_agents_op_neighbors(map<int, migrate_ball_S> agent_to_move_in)
{   
    //commit_own_mpi_data_structure: agent_mpi_structure
    // printf("pause_node_move_agents_num: %lu\n",agent_to_move_in.size());

    MPI_Datatype agent_mpi_structure_type;
    int lengths[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const MPI_Aint displacements[10] = { 0, sizeof(int), 2*sizeof(int),\
                                        2*sizeof(int)+sizeof(float), \
                                        2*sizeof(int)+2*sizeof(float),\
                                        2*sizeof(int)+3*sizeof(float),\
                                        2*sizeof(int)+4*sizeof(float),\
                                        2*sizeof(int)+5*sizeof(float),\
                                        2*sizeof(int)+6*sizeof(float),\
                                        2*sizeof(int)+7*sizeof(float)};
    MPI_Datatype types[10] = { MPI_INT, MPI_INT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,\
                               MPI_FLOAT, MPI_FLOAT,MPI_FLOAT };
    MPI_Type_create_struct(10, lengths, displacements, types, &agent_mpi_structure_type);
    MPI_Type_commit(&agent_mpi_structure_type);

    comm comm_info = get_communicator_info(communicator);

    //Step0: send the message type;
    
    MPI_Request request;

    map<int, migrate_ball_S>::iterator itr;
    for(itr=agent_to_move_in.begin();itr!=agent_to_move_in.end();++itr){

        //ToDo: send the agent to the corresponding sub nodes;
        string msg_type_string = "0000995";
        char message_type_char[MSG_LENGTH];
        strcpy(message_type_char,msg_type_string.c_str());
        MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR, itr->second.partition_id,0,communicator, &request);

        //Step2: send the true data;(define the message structure;)
        struct agent_mpi_structure agent_true_data;
        agent_true_data.agent_id = itr->second.migrate_ball->agent_id;
        agent_true_data.diameter = itr->second.migrate_ball->diameter;
        
        agent_true_data.state_next_pos_x = itr->second.migrate_ball->state_next->position->x;
        agent_true_data.state_next_pos_y = itr->second.migrate_ball->state_next->position->y;
        agent_true_data.state_next_traj_x = itr->second.migrate_ball->state_next->trajectory->x;
        agent_true_data.state_next_traj_y = itr->second.migrate_ball->state_next->trajectory->y;

        agent_true_data.pos_x = itr->second.migrate_ball->state_current->position->x;
        agent_true_data.pos_y= itr->second.migrate_ball->state_current->position->y;
        agent_true_data.traj_x = itr->second.migrate_ball->state_current->trajectory->x;
        agent_true_data.traj_y = itr->second.migrate_ball->state_current->trajectory->y;
       
        //Step2: send to specific sub nodes; tag=1
        // printf("step: %d,sender: %d, receiver: %d \n", step, comm_info.rank, itr->second.partition_id);
        MPI_Isend(&agent_true_data,1,agent_mpi_structure_type,itr->second.partition_id,1,communicator,&request);
        
    }
    MPI_Wait(&request, MPI_STATUS_IGNORE);

    // printf("rank: %d alread_send_msg:  0000996 \n",comm_info.rank);
    //send the msg_type='008' to root, tag=0
    string msg_type_string = "0000008";
    char message_type_char[MSG_LENGTH];
    strcpy(message_type_char,msg_type_string.c_str());
    MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR,comm_info.size-1 ,0,communicator);
    printf("rank: %d alread_send_msg:  008\n",comm_info.rank);
    return;

}


comm SimulationController::get_communicator_info(MPI_Comm communicator_in){


    int rank;
    MPI_Comm_rank(communicator, &rank);
    int comm_size;
    MPI_Comm_size(communicator, &comm_size);

    comm communicator_info;
    communicator_info.rank = rank;
    communicator_info.size = comm_size;

    
    return communicator_info;

}


MPI_Request *SimulationController::send_msg_to_itself(string type,float data, MPI_Request *request)
{
    comm comm_info = get_communicator_info(communicator);
    //send message_type
    char message_type[MSG_LENGTH];
    strcpy(message_type,type.c_str());
    // cout<< "sender_float_itself:"<<strlen(message_type)+1 <<endl;
    MPI_Isend(message_type, MSG_LENGTH , MPI_CHAR, comm_info.rank,0,communicator, request);

    return request;
}

void SimulationController::send_msg_to_root(string type,float data)
{
    
}

void SimulationController::process_msg_from_itself(MPI_Request *request, string msg_type)
{
    // printf("get_msg_from_iteself : %s\n", msg_type.c_str());
    // parse msg_type from itself
    if(msg_type == "0000001")
    {   
        //step1: do next simulation
        if(no_migrate){
            next_step();
        }

        MPI_Wait(request,MPI_STATUS_IGNORE);
    }
    else{
        // cout <<"0000001-->undefine_msg_type" <<string(msg_type)<<endl;
        printf("0000001-->msg_type to be defined \n");
    }

}
void SimulationController::process_msg_from_root(string msg_type)
{
    // printf("get_msg_from_root : %s\n", msg_type.c_str());
    
    comm comm_info = get_communicator_info(communicator);
    string sub_msg_type = string(msg_type).substr(4,3);

    if(sub_msg_type == "999"){   
        process_999_msg_from_root();
    }
    else if(sub_msg_type == "998"){
        process_998_msg_from_root();

    }
    else if(sub_msg_type == "997")
    {
        process_997_msg_from_root();
    }

    else if(sub_msg_type == "996")
    {
        process_996_msg_from_root();
    }

    else if(sub_msg_type == "994")
    {
        process_994_msg_from_root();
    }
    else if(sub_msg_type == "993")
    {
        process_993_msg_from_root();
    }
    else if(sub_msg_type == "992"){
        process_992_msg_from_root();
    }
    


}

void SimulationController::process_999_msg_from_root()
{   
    if(OBSERVE_NODE==comm_info.rank)
    {
        cout << "rank: "<< comm_info.rank << " receice_msg 999 from root" <<endl;
    }

    if(comm_info.rank == OBSERVE_NODE){
        printf("(999)rank: 1 , agent_size: %lu\n",simulator->agents.size());
    }
    // printf("(999)rank: %d, agent_size: %lu\n",comm_info.rank,simulator->agents.size());
    no_migrate = false; // other nodes also need to stop simulation.
    
    // 1- Send msg type="003", tag=0
    char message_type_char[MSG_LENGTH];
    string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,"003");
    strcpy(message_type_char,complete_msg.c_str());
    MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);

    //2- Send true msg.(current simulation step), tag=1
    int true_msg = step;
    MPI_Send(&true_msg,1,MPI_INT,comm_info.size-1,1,communicator);
}

void SimulationController::process_998_msg_from_root()
{
    /*
    "998": stop the simulation step for synch,
    and send the current status back to root node(blocking)
    */

    //simulation_step_table = [node1_step,node1_step,.....,pause_node];
    //simulation_step_table[pause_node] --> pause_node_step

    if(OBSERVE_NODE == comm_info.rank)
    {
        cout << "rank: "<< comm_info.rank << " receice_msg 998 from root" <<endl;
    }


    int* simulation_step_table = (int*)malloc(sizeof(int)*comm_info.size);
    //print out simulation_step_table
    /*
    printf("before_table: %d %d %d %d\n",simulation_step_table[0]\
                                    ,simulation_step_table[1]\
                                    ,simulation_step_table[2]\
                                    ,simulation_step_table[3]);
    */
    // 1- receive msg from node. tag=1
    MPI_Recv(simulation_step_table,comm_info.size,MPI_INT,comm_info.size-1,1,communicator,MPI_STATUS_IGNORE);
    //print out simulation_step_table
    /*
    if(comm_info.rank == OBSERVE_NODE)
    {
        printf("rank: %d, before process self_blocked: [%d, %d, %d]\n",\
            comm_info.rank,\
            simulation_step_table[0],\
            simulation_step_table[1],\
            simulation_step_table[2]
            );
    }
    */
    int current_pause_node_step = simulation_step_table[simulation_step_table[comm_info.size-1]];
    int interval_1 = simulation_step_table[comm_info.rank] - current_pause_node_step;
    
    // save the pause_node_info
    pause_node = simulation_step_table[comm_info.size-1];
            
    // update_simulation_table to make sure there is only one node blocked.(004msg)
    if(comm_info.rank!=pause_node && self_blocked)
    {
        // printf("rank: %d, self_block == true, pause_node: %d,\n", comm_info.rank,pause_node);
        update_simtable_before_rollback = true;

        previous_status.pop_back();
        // previous_status.pop_back();
        simulator->agents = previous_status.back();
        // previous_agents_waiting_to_move.pop_back();
        // simulator->agents_corresponding_to_previous_status = previous_agents_waiting_to_move.back();
        
        step -= 1;
        self_blocked = false;  //pause_node will keep it true.
        agents_waiting_to_move.clear();
       
    }

    // 1- Send msg type="004", tag=0
    char message_type_char[MSG_LENGTH];
    string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,"004");
    strcpy(message_type_char,complete_msg.c_str());
    MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);

    //2- Send true msg.(current simulation step), tag=1
    int true_msg = step;
    MPI_Send(&true_msg,1,MPI_INT,comm_info.size-1,1,communicator);
 
}

void SimulationController::process_997_msg_from_root()
{
    if(OBSERVE_NODE == comm_info.rank)
    {
        cout << "rank: "<< comm_info.rank << " receice_msg 997 from root" <<endl;
    }
    // cout << "rank: "<< comm_info.rank << " receice_msg 997 from root" <<endl;
    int* simulation_step_table = (int*)malloc(sizeof(int)*comm_info.size);
  
    // 1- receive msg from node. tag=1
    MPI_Recv(simulation_step_table,comm_info.size,MPI_INT,comm_info.size-1,1,communicator,MPI_STATUS_IGNORE);
    //print out simulation_step_table
    /*
    printf("rank: %d,after process self_blocked: [%d, %d, %d, %d]\n",\
                comm_info.rank,\
                simulation_step_table[0],\
                simulation_step_table[1],\
                simulation_step_table[2],\
                simulation_step_table[3]\
                );
    */
    int current_pause_node_step = simulation_step_table[simulation_step_table[comm_info.size-1]];
    int interval_1 = simulation_step_table[comm_info.rank] - current_pause_node_step;
    if(interval_1 >=0)
    {
        while(interval_1)
        {
            previous_status.pop_back();
            interval_1 -= 1;

        }

        if(comm_info.rank != pause_node)
        {
            simulator->agents = previous_status.back();
            step = current_pause_node_step;
            previouse_pause_step = -1;
            printf("step_roll_back:%d\n",step);
            
        }  

        //send the msg to root(005), I am already finish my rollback
        char message_type_char[MSG_LENGTH];
        string complete_msg = generate_msg_type(comm_info.rank,comm_info.size-1,"005");
        strcpy(message_type_char,complete_msg.c_str());
        MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, comm_info.size-1,0,communicator);

    }

    else{
        no_migrate = true; // go to catche up the pause_node step;           
        previouse_pause_step = current_pause_node_step;

    }

}

void SimulationController::process_996_msg_from_root()
{

    if(comm_info.rank == OBSERVE_NODE)
    {
        printf("rank: %d receive_msg_996_from_root\n",comm_info.rank);
    }
    distribute = true;

}

void SimulationController::process_994_msg_from_root(){

    previous_status.pop_back();
    previous_status.push_back(simulator->agents);

    printf("(994)rank: %d, synch_step: %d, synch_agent_size: %lu\n", comm_info.rank, step, simulator->agents.size());
    
    self_blocked = false;
    no_migrate = true;
    pause_node = -1;
    previouse_pause_step = -1;
    update_simtable_before_rollback = false;
    agents_waiting_to_move.clear();
    distribute = false;
    // sleep(1);
    // printf("============next_simulation=============================\n");

}

void SimulationController::process_993_msg_from_root(){

    // reserved msg slots.(do nothing)
    printf("(993)rank: %d, step: %d, agent_size: %lu\n", comm_info.rank, step, simulator->agents.size());
    

}

void SimulationController::process_992_msg_from_root(){

    printf("rank: %d, receive ending signal 992\n",comm_info.rank);
    done = true;

}

void SimulationController::process_msg_from_neighbor_nodes(MPI_Status *status_prob,string msg_type)
{

    // printf("rank:%d, get_msg_from_neighbors : %s\n", comm_info.rank, msg_type.c_str());
    // cout << "rank: "<< comm_info.rank << " receice_msg 0000995 from neighbors" <<endl;
    
    if(string(msg_type) == "0000995")
    {
        process_0000995_msg_from_neighbor(status_prob);
     
    }
    else if(string(msg_type) == "1111995")
    {
        process_1111995_msg_from_neighbor(status_prob);
    }


}


void SimulationController::process_0000995_msg_from_neighbor(MPI_Status *status_prob){
    

    //commit_own_mpi_data_structure: agent_mpi_structure
    MPI_Datatype agent_mpi_structure_type;
    int lengths[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const MPI_Aint displacements[10] = { 0, sizeof(int), 2*sizeof(int),\
                                        2*sizeof(int)+sizeof(float), \
                                        2*sizeof(int)+2*sizeof(float),\
                                        2*sizeof(int)+3*sizeof(float),\
                                        2*sizeof(int)+4*sizeof(float),\
                                        2*sizeof(int)+5*sizeof(float),\
                                        2*sizeof(int)+6*sizeof(float),\
                                        2*sizeof(int)+7*sizeof(float)};
    MPI_Datatype types[10] = { MPI_INT, MPI_INT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT,\
                            MPI_FLOAT, MPI_FLOAT,MPI_FLOAT };
    MPI_Type_create_struct(10, lengths, displacements, types, &agent_mpi_structure_type);
    MPI_Type_commit(&agent_mpi_structure_type);



    // receive true data: agent.
    MPI_Status status_true_data;
    struct agent_mpi_structure agent_true_data;
    MPI_Recv(&agent_true_data, 1, agent_mpi_structure_type, status_prob->MPI_SOURCE, 1, communicator,&status_true_data);
    // printf("rank: %d, 0000996 ---> true_data:%d \n",comm_info.rank, agent_test);

    //add_agents() use true data instead of "int"
    Ball *ball = reconstruct_ball_agent(agent_true_data);

    // printf("rank: %d receive agents from: %d\n", comm_info.rank,status_true_data.MPI_SOURCE);
    simulator->agents[ball->agent_id] = *ball;
    // printf("rank:%d,add_agent_after: %lu\n",comm_info.rank,simulator->agents.size());




}

void SimulationController::process_1111995_msg_from_neighbor(MPI_Status *status_s){

    // cout << "rank: "<< comm_info.rank << " receice_msg 11..95 from neighbor: "<< status_s->MPI_SOURCE <<endl;
    neighbor_counter -= 1;

    if(neighbor_counter == 0)
    {
        neighbor_counter = comm_info.size - 2;
        //send the msg_type='007' to root, tag=0
        string msg_type_string = "0000007";
        char message_type_char[MSG_LENGTH];
        strcpy(message_type_char,msg_type_string.c_str());
        MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR,comm_info.size-1 ,0,communicator);
        // printf("rank: %d alread_send_msg:  007\n",comm_info.rank);

    }




}

void SimulationController::process_msg_from_other_all_node(MPI_Request *request){

    // printf("probe the message\n");
    comm comm_info = get_communicator_info(communicator);
    
    // char * msg_type = new char[30];
    char msg_type[MSG_LENGTH * 2];
    //Probe message arriving from all node;
    MPI_Status status_prob;
    int flag_itself;
    MPI_Iprobe(MPI_ANY_SOURCE,0,communicator,&flag_itself,&status_prob);

    if(!flag_itself){
        int m = 0;
        // printf("Process %d: no message arrived from itself yet.\n", comm_info.rank);
    }
    else{
        if(status_prob.MPI_SOURCE == comm_info.rank)
        {   // process msg from itself.
            MPI_Status status_s;
            MPI_Recv(msg_type, MSG_LENGTH, MPI_CHAR, comm_info.rank, 0, communicator,&status_s);
            process_msg_from_itself(request,string(msg_type));
        }
        else if(status_prob.MPI_SOURCE == (comm_info.size-1))
        {
            // process msg from root node.
            MPI_Status status_s;
            // printf("Process %d: message arrived from itself.\n", comm_info.rank);
            MPI_Recv(msg_type, MSG_LENGTH, MPI_CHAR, comm_info.size-1, 0, communicator,&status_s);
            process_msg_from_root(string(msg_type));
        }
        else{
            // process msg from neighbor node.
            MPI_Status status_s;
            // printf("Process %d: message arrived from itself.\n", comm_info.rank);
            MPI_Recv(msg_type, MSG_LENGTH, MPI_CHAR, status_prob.MPI_SOURCE, 0, communicator,&status_s);
            process_msg_from_neighbor_nodes(&status_prob,string(msg_type));
        }

    }

}

string SimulationController::generate_msg_type(int sender_rank,int receiver_rank,string msg)
{

    string complete_msg = "";
    
    if(sender_rank < 10)
    {
        complete_msg += "0"+ to_string(sender_rank);
    }
    else{
        complete_msg += to_string(sender_rank);
    }
    if(receiver_rank < 10)
    {
        complete_msg += "0" + to_string(receiver_rank);
    }
    else{
        complete_msg += to_string(receiver_rank);
    }

    return complete_msg += msg;



}

Ball* SimulationController::reconstruct_ball_agent(agent_mpi_structure agent_true_data){

    Position* pos_in = new Position(agent_true_data.pos_x ,\
                                    agent_true_data.pos_y);
    Trajectory* traj_in = new Trajectory( agent_true_data.traj_x , \
                                        agent_true_data.traj_y);

    int diameter_in =agent_true_data.diameter;
    int agent_id_in = agent_true_data.agent_id;

    Ball *ball;
    ball = new Ball(pos_in, traj_in, diameter_in, agent_id_in);

    Position *position_next = new Position(agent_true_data.state_next_pos_x ,\
                                    agent_true_data.state_next_pos_y);

    Trajectory *trajectory_next = new Trajectory(agent_true_data.state_next_traj_x ,\
                                    agent_true_data.state_next_traj_y);
    

    ball->state_next = new BallState(position_next, trajectory_next);

    return ball;
    
}



SimulationController::~SimulationController(){



}

