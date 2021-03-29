#include "root.h"



Root::Root()
{

}

Root::Root(MPI_Comm communicator_in){
    

    communicator = communicator_in;
    comm comm_info = get_communicator_info(communicator);
    comm_size = comm_info.size;
    comm_rank = comm_info.rank;

    
    for(int i=0; i<comm_size-1; ++i){
        count_simulation_step_receive.push_back(1);
        already_rollback_counter.push_back(1);
        count_pause_nodes_finish_migrate.push_back(1);
        count_ending.push_back(1);
    }

    nodes_amount_except_pause_node = comm_size - 2;

}

comm Root::get_communicator_info(MPI_Comm communicator_in){


    int rank;
    MPI_Comm_rank(communicator, &rank);
    int comm_size;
    MPI_Comm_size(communicator, &comm_size);

    comm communicator_info;
    communicator_info.rank = rank;
    communicator_info.size = comm_size;

    
    return communicator_info;

}

int Root::get_msg_type(string msg){

    string sub_msg = msg.substr(4,3);

    if(sub_msg == "002"){   //002-->999
        // there are subnodes get blocked. 
        return 0;
    }
    else if(sub_msg == "003") //003-->998
    {
        // subnodes send their status(simulation_step) back.
        return 1;
    }
    else if(sub_msg == "004") //004-->997
    {
        return 2;
    }

    else if(sub_msg == "005") //005-->996
    {
        return 3;
    }

    else if(sub_msg == "006") //006-->999 
    {
        return 4;
    }
    else if(sub_msg == "007") // 007--->994
    {
        return 5;
    }
    else if(sub_msg == "008")
    {
        return 7;
    }

}



string Root::get_complete_msg_according_to_receiving(int receive_msgtype){
    
    
    string sendback_complete_msg = "";

    switch (receive_msgtype)
    {
    case SUB_NODE_BLOCK:   //002   ---> ----999
        
        sendback_complete_msg = "----999";
        break;
    
    case SUB_NODE_CURRENT_STATUS:  //003  ---> ----998
        sendback_complete_msg = "----998";
        break;
    
    case ALIGN_THE_SIMULATION_TABLE: //004---->997
        sendback_complete_msg = "----997";
        break;

    
    case ALREADY_ROLLBACK:  //005  ---> ----996
        sendback_complete_msg = "----996";
        break;
    
    case UPDATE_SIMULATION_TABLE:  //006  ---> ----999(update_simulation_step)
        sendback_complete_msg = "----999";
        break;
    
    case PAUSE_NODE_FINISH_DISTRIBUTING:
        sendback_complete_msg = "----994";
        break;

    case NEIGHBOR_MIGRATE:
        sendback_complete_msg = "----993";
        break;
    
    case SUB_NODE_FINISH_SIM:
        sendback_complete_msg = "----992";
        break;

    default:
        break;
    }

    return sendback_complete_msg;

}


void Root::process_msg(string msg)
{
    int msg_type_int = get_msg_type(msg);

}

void Root::process_msg_SUB_NODE_BLOCK(MPI_Status status_s, string send_back_to_subnodes_msg){
/*  
        receive_msg_type: 002
        
        002: 1- receive msg from one(only one, discard others) of sub nodes  
            2- let sub nodes stop simulation
            3- broadcast msg to all sub nodes asking for their current status 

        sendback_msg_type: 999
            introduction: see the corresponding receiver.
        
    */

    // printf("Process %d: message arrived from rank: %d\n", rank, status_s.MPI_SOURCE); 
    // cout << "(999)root_send_back_msg:" << send_back_to_subnodes_msg<< endl;
    char message_type_char[MSG_LENGTH];
    MPI_Request request;
    // MPI_Request simulation_step_table_request;

    if(receive_msg_from_only_one_node_flag)
    {
        // pause_node_update = true;
        pause_node = status_s.MPI_SOURCE;
        // printf("pause_node: %d\n", pause_node);
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR,node_num,0,communicator, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        // printf("rank: %d already sent 999\n", rank);
        // only receive 002 msg once,
        // discard following '002' msg from other sub nodes.
        receive_msg_from_only_one_node_flag = false;
    }


}
void Root::process_msg_SUB_NODE_CURRENT_STATUS(MPI_Status status_s, string send_back_to_subnodes_msg)
{
    /*
    receive_msg_type: 003

    003: 1- receive current status(simulation step) from all sub nodes  
         2- organize the whole infomation as a simulation status 
            table of all nodes.
         3- broadcast the simulation status table to all sub nodes. 

    sendback_msg_type: 998
        introduction: see the corresponding receiver.
    */

    // cout << "(998)root_send_back_msg:" << send_back_to_subnodes_msg<< endl;
    char message_type_char[MSG_LENGTH];
    MPI_Request request;
    MPI_Request simulation_step_table_request;

    MPI_Probe(MPI_ANY_SOURCE, 1, communicator, &status_s);
    MPI_Recv(&simulation_step_table[status_s.MPI_SOURCE],1,MPI_INT,status_s.MPI_SOURCE,1,communicator,MPI_STATUS_IGNORE);
    count_simulation_step_receive.at(status_s.MPI_SOURCE) = 0;
    // received all simulaiton status.
    if(accumulate(count_simulation_step_receive.begin(),count_simulation_step_receive.end(),0)==0)
    {   
        // save the info of pause_node at the last position.
        simulation_step_table[comm_size-1] = pause_node;

        //1- send msg type
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR, node_num,0,communicator, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        //2- [node1_step,node1_step,.....,pause_node];
        // simulation_step_table[pause_node] --> pause_node_step
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(simulation_step_table, comm_size , MPI_INT, node_num,1,communicator, &simulation_step_table_request);
        }

        MPI_Wait(&simulation_step_table_request, MPI_STATUS_IGNORE);

        //reset simulation_step_table to all 0.
        fill(simulation_step_table, simulation_step_table+comm_size-1, 0);
        fill(count_simulation_step_receive.begin(),count_simulation_step_receive.end(),1);// for another round.
        receive_msg_from_only_one_node_flag = true; // allow to receive new 002 msg

    }

}

void Root::process_msg_ALIGN_THE_SIMULATION_TABLE(MPI_Status status_s, string send_back_to_subnodes_msg)
{
    /*
    receive_msg_type: 004 (get new simulation_step_table, pause_nope keeps the same)

    004: 1- receive new current status(simulation step) from all sub nodes  
         2- organize the whole infomation as a new simulation status 
            table of all nodes.
         3- broadcast the new simulation status table to all sub nodes. 

    sendback_msg_type: 997
        introduction: see the corresponding receiver.
    */

    // cout << "(997)_root_send_back_msg:" << send_back_to_subnodes_msg<< endl;
    char message_type_char[MSG_LENGTH];
    MPI_Request request;
    MPI_Request simulation_step_table_request;
    //1- get simulation status from all sub nodes
    //2- Probe for an incoming message from all process 
    // 3- get true data(tag = 1)
    MPI_Probe(MPI_ANY_SOURCE, 1, communicator, &status_s);
    MPI_Recv(&simulation_step_table[status_s.MPI_SOURCE],1,MPI_INT,status_s.MPI_SOURCE,1,communicator,MPI_STATUS_IGNORE);
    count_simulation_step_receive.at(status_s.MPI_SOURCE) = 0;
    
    // received all simulaiton status.
    if(accumulate(count_simulation_step_receive.begin(),count_simulation_step_receive.end(),0)==0)
    {   
        // save the info of pause_node at the last position.
        //1- send msg type 997
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR, node_num,0,communicator, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        //2- [node1_step,node1_step,.....,pause_node];
        //   simulation_step_table[pause_node] --> pause_node_step
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(simulation_step_table, comm_size , MPI_INT, node_num,1,communicator, &simulation_step_table_request);
        }

        MPI_Wait(&simulation_step_table_request, MPI_STATUS_IGNORE);

        //reset simulation_step_table to all 0.
        fill(simulation_step_table, simulation_step_table+comm_size-1, 0);
        fill(count_simulation_step_receive.begin(),count_simulation_step_receive.end(),1);// for another round.
        receive_msg_from_only_one_node_flag = true; // allow to receive new 002 msg

    }

}

void Root::process_msg_ALREADY_ROLLBACK(MPI_Status status_s, string send_back_to_subnodes_msg){

    //Step1: wait untill all "005" msgs are received from all sub nodes.
    //Step2: send "996" msg to the pause_node.

    // cout << "(996)_root_send_back_msg_test:" << send_back_to_subnodes_msg<< endl;
    char message_type_char[MSG_LENGTH];
    MPI_Request request;
    MPI_Request simulation_step_table_request;

    already_rollback_counter[status_s.MPI_SOURCE] = 0;

    if(accumulate(already_rollback_counter.begin(),already_rollback_counter.end(),0) == 0)
    {   
        
        //Send "996" msg to pause node(marked node in simulation status table)

        // cout << "root_send_back_to_subnodes_msg:" <<send_back_to_subnodes_msg <<endl;
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());

        for(int node_num=0; node_num <comm_size-1; ++node_num)
        {
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, node_num,0,communicator);
        }

        // MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, pause_node,0,communicator);

        fill(already_rollback_counter.begin(),already_rollback_counter.end(),1); //for another round
    }

}

void Root::process_msg_UPDATE_SIMULATION_TABLE(MPI_Status status_s, string send_back_to_subnodes_msg)
{

    // cout << "(995-->999)_root_send_back_msg:" << send_back_to_subnodes_msg<< endl;
    char message_type_char[MSG_LENGTH];
    MPI_Request request;
    MPI_Request simulation_step_table_request;


    if(receive_msg_from_only_one_node_flag)
    {   
        // update all counters for new simulation_step_table.
        receive_msg_from_only_one_node_flag = false;
        pause_node = -1; 
        count_simulation_step_receive.clear();
        already_rollback_counter.clear();
        for(int i=0; i<comm_size-1; ++i){
            
            count_simulation_step_receive.push_back(1);
            already_rollback_counter.push_back(1);
        }
        // pause_node_update = true;
        pause_node = status_s.MPI_SOURCE;
        // printf("pause_node: %d\n", pause_node);
        
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());
        
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR,node_num,0,communicator, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        
    }

}

void Root::process_msg_PAUSE_NODE_FINISH_DISTRIBUTING(MPI_Status status_s, string send_back_to_subnodes_msg){

    // cout << "(994)_root_send_back_msg:" << send_back_to_subnodes_msg<< endl;

    char message_type_char[MSG_LENGTH];
    MPI_Request request;

    count_pause_nodes_finish_migrate.at(status_s.MPI_SOURCE) = 0;
    
    if(accumulate(count_pause_nodes_finish_migrate.begin(), count_pause_nodes_finish_migrate.end(),0) == 0)
    {   
        
        //Send "996" msg to pause node(marked node in simulation status table)

        // cout << "send_back_to_subnodes_msg:" <<send_back_to_subnodes_msg <<endl;
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());

        for(int node_num=0; node_num <comm_size-1; ++node_num)
        {
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, node_num,0,communicator);
        }

        // MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, pause_node,0,communicator);

        fill(count_pause_nodes_finish_migrate.begin(),count_pause_nodes_finish_migrate.end(),1); //for another round
    }


}

void Root::process_msg_NEIGHBOR_MIGRATE(MPI_Status status_s, string send_back_to_subnodes_msg){
     
    //  cout << "(993)_root_send_back_msg:" << send_back_to_subnodes_msg<< endl;
     nodes_amount_except_pause_node -= 1;
     if(nodes_amount_except_pause_node == 0)
     {
        char message_type_char[MSG_LENGTH];
        MPI_Request request;
        
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());
            
        for(int node_num=0; node_num<comm_size-1;++node_num)
        {   
            MPI_Isend(message_type_char, MSG_LENGTH , MPI_CHAR,node_num,0,communicator, &request);
        }
        MPI_Wait(&request, MPI_STATUS_IGNORE);

        nodes_amount_except_pause_node = comm_size - 2;
     }
    

}

void Root::process_msg_SUB_NODE_FINISH_SIM(MPI_Status status_s, string send_back_to_subnodes_msg){

    char message_type_char[MSG_LENGTH];
    MPI_Request request;

    count_ending.at(status_s.MPI_SOURCE) = 0;
    
    if(accumulate(count_ending.begin(), count_ending.end(),0) == 0)
    {   
        
        //Send "992" msg to pause node(marked node in simulation status table)

        // cout << "send_back_to_subnodes_msg:" <<send_back_to_subnodes_msg <<endl;
        strcpy(message_type_char,send_back_to_subnodes_msg.c_str());

        for(int node_num=0; node_num <comm_size-1; ++node_num)
        {
            MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, node_num,0,communicator);
        }

        // MPI_Send(message_type_char, MSG_LENGTH , MPI_CHAR, pause_node,0,communicator);
        
        fill(count_ending.begin(),count_ending.end(),1); //for another round
        flag = false;

        printf("root_simulation_finished......\n");

    }


}
void Root::run()
{
    comm comm_info = get_communicator_info(communicator);

    while(flag)
    {  
        //Step1: receive msg from sub notes
        char msg_type[MSG_LENGTH];
        MPI_Status status_s;
        
        // cout<< "msg_type_string_before_Recv:"<<string(msg_type) <<endl;
        MPI_Recv(msg_type, MSG_LENGTH, MPI_CHAR, MPI_ANY_SOURCE, 0, communicator,&status_s);

        int msg_id = get_msg_type(msg_type);
        string send_back_to_subnodes_msg = get_complete_msg_according_to_receiving(msg_id);
       
        switch (msg_id)
        {
            case SUB_NODE_BLOCK:
            //(process 002, send back 999)
                process_msg_SUB_NODE_BLOCK(status_s, send_back_to_subnodes_msg);
                break;

            case SUB_NODE_CURRENT_STATUS:
            //(process 003, send back 998)
                process_msg_SUB_NODE_CURRENT_STATUS(status_s,send_back_to_subnodes_msg);
                break;
            
            case ALIGN_THE_SIMULATION_TABLE:
            //(process 004, send back 997)
                process_msg_ALIGN_THE_SIMULATION_TABLE(status_s,send_back_to_subnodes_msg);
                break;

            case ALREADY_ROLLBACK:
            //(process 005, send back 996)
              
                process_msg_ALREADY_ROLLBACK(status_s,send_back_to_subnodes_msg);
                break;
            
            case UPDATE_SIMULATION_TABLE:
            //(process 006, send back 999)
                process_msg_UPDATE_SIMULATION_TABLE(status_s,send_back_to_subnodes_msg);
                break;
            
            case PAUSE_NODE_FINISH_DISTRIBUTING:
            //(process 007, send back 994)
                process_msg_PAUSE_NODE_FINISH_DISTRIBUTING(status_s,send_back_to_subnodes_msg);
                break;
            case NEIGHBOR_MIGRATE:
                process_msg_NEIGHBOR_MIGRATE(status_s,send_back_to_subnodes_msg);
                break;
            
            case SUB_NODE_FINISH_SIM:
                process_msg_SUB_NODE_FINISH_SIM(status_s,send_back_to_subnodes_msg);
                break;
            
            default:
                break;
            }

    }


}







