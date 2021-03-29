#include "simulator.h"

Simulator::Simulator (map<string,map<string,float>> config, MPI_Comm communicator_in,int partition_id)
{
    ignore_remote_agent = false;
    migrate_ball_list.clear();
    agents.clear();
    agents_corresponding_to_previous_status.clear();

    idprovider = new IDProvider(partition_id);
    _id_provider = idprovider->get_id();

    // configure simulation world
    world = new World(config["world_config"]["x_min"],
                      config["world_config"]["y_min"],
                      config["world_config"]["x_max"],
                      config["world_config"]["y_max"],
                      config["world_config"]["grid_width"],
                      config["world_config"]["grid_height"],
                      config["world_config"]["comm_size"]
                        );
    my_partition =  world->get_region_by_id(partition_id);
    printf("my_partion: %d \n",my_partition->partition_id);
    step = 1;
    
    previous_pos.clear();
    target_pos.clear();
    migrate_ball_pos_list.clear();
    ball_diameter = 1.0f;
    communicator = communicator_in;
    comm_info = get_communicator_info(communicator);

}


void Simulator::get_id_provider()
{
    return ;
}


void Simulator::next_step()
{
    map<int,Ball>::iterator itr;
    
    for(itr = agents.begin(); itr!=agents.end();++itr){
        itr->second.update_state(world); 
        
    }

    return ;
}

void Simulator::ball_collide_check()
{
    return ;
}

void Simulator::commit_states()
{
    map<int,Ball>::iterator itr;
    
    for(itr = agents.begin(); itr!=agents.end();++itr){
        itr->second.commit_state_update(); 
    }
    step += 1;

    return ;
}

void Simulator::print_state()
{
    return;
}

void Simulator::add_agents(vector<agent_S> agents_list, bool ignore_remote_agent, vector<agent_S> migrate_list)
{
    // migrate_ball_list.swap(migrate_list);
    migrate_ball_list = migrate_list;
    // cout << "load_agent_size:" << agents_list.size()<<endl;
    // vector<agent_S>::iterator itr; 
    for(int i=0; i< agents_list.size(); ++i){
        
        bool agent_is_local = true;
        if(ignore_remote_agent)
        {
            Partition *tempPartition;
            tempPartition = world->map_partition(agents_list[i].state_current.position.x,\
                                agents_list[i].state_current.position.y);
            // int m = tempPartition->partition_id ;
            // int n =  my_partition->partition_id;
            // cout << "tempPartition->partition_id:" << tempPartition->partition_id <<" -->" <<my_partition->partition_id<<endl;
            agent_is_local = (tempPartition->partition_id == my_partition->partition_id);
            // cout << "agent_is_local: "<< agent_is_local <<endl;
        }

        if(agent_is_local){
            // std::cout << "insert_agent_id:" << agents_list[i].agent_id <<endl;
            // agents.insert(pair<int, agent_S>(agents_list[i].agent_id,agents_list[i]));
        
            // agents[agents_list[i].agent_id] = agents_list[i];

            Position *pos_in;
            Trajectory *traj_in;
            float diameter_in;
            int agent_id_in;

            pos_in = new Position(agents_list[i].state_current.position.x,\
                                 agents_list[i].state_current.position.y);
            traj_in = new Trajectory(agents_list[i].state_current.trajectory.x, \
                                     agents_list[i].state_current.trajectory.y);
                    
            diameter_in = agents_list[i].diameter;
            agent_id_in = agents_list[i].agent_id;

            Ball *ball;

            ball = new Ball(pos_in, traj_in, diameter_in, agent_id_in);
            
        
            agents[agents_list[i].agent_id] = *ball;
        }
            
            
        
    }

}

void Simulator::add_agents_new(Ball *agents_ball, bool ignore_remote_agent)
{   
    agents[agents_ball->agent_id] = *agents_ball;
}


map<int, migrate_ball_S> Simulator::migrate_agents()
{  
    if(OBSERVE_NODE == comm_info.rank)
    {
        printf("(migrate_agent)rank: %d, before_migrate_agent_size: %lu\n",comm_info.rank,agents.size());
    }
    map<int, migrate_ball_S> agents_to_migrate;
    agents_to_migrate.clear();
    map<int, Ball>::iterator itr;
    for(itr=agents.begin(); itr!=agents.end(); ++itr){
        Partition *p = world->map_partition(itr->second.state_current->position->x,\
                                            itr->second.state_current->position->y);
        
        // cout << p->partition_id << "   "<< my_partition->partition_id << endl;
        if(p->partition_id!= my_partition->partition_id)
        {   
            migrate_ball_S migrate_info;
            migrate_info.migrate_ball = new Ball();
            migrate_info.partition_id = p->partition_id;
            *migrate_info.migrate_ball = itr->second;
            agents_to_migrate[itr->second.agent_id] = migrate_info;
            // cout <<"move_out_agent_id:" <<itr->second.agent_id <<endl;
            // cout << "-----------------------migrate-----------------------"<<endl;
        }
    }

    //delete migrated agents
    // printf("agents_to_migrate.size(): %d",agents_to_migrate.size());
    map<int, migrate_ball_S>::iterator itr_migrate;
    // printf("before_migrate_____:%lu \n",agents.size());

    // printf("before_erase: %lu xuyao erase: %lu\n", agents.size(),agents_to_migrate.size());

    for(itr_migrate=agents_to_migrate.begin(); itr_migrate!=agents_to_migrate.end(); ++itr_migrate)
    {
        // printf("itr_migrate->first: %d\n",itr_migrate->first);
        
        agents.erase(agents.find(itr_migrate->second.migrate_ball->agent_id));

        /*
        if(agents.count(itr_migrate->second.migrate_ball->agent_id) == 0)
        {
            printf("rank:%d, succeed to remove the agents....%d\n", comm_info.rank,itr_migrate->second.migrate_ball->agent_id);
        }
        else{
            
            printf("rank:%d, failed to remove the agents....%d\n", comm_info.rank,itr_migrate->second.migrate_ball->agent_id);
        }
        */


    }    
    //  cout<<"rank:" << idprovider->_id_prefix <<"after_migrate:" << agents.size() << "  "<< agents_to_migrate.size()<<endl;
    // printf("(migrate_agent)rank: %d, after_migrate_agent_size: %lu\n",comm_info.rank,agents.size());
   
    // printf("after_erase: %lu\n", agents.size());
    return agents_to_migrate;
}

// void Simulator::print_agent_S(map<int,agent_S> agents){

//     map<int,agent_S>::iterator iter;
//     cout << "agent_size:" <<agents.size() << endl;
//     for(iter = agents.begin(); iter!=agents.end();iter++)
//     {
//         cout <<"agent_id:" <<iter->first<<endl; 
//     }

    
// }


void Simulator::print_agent_S_ball(map<int, Ball> agents)
{
    map<int, Ball>::iterator iter;
    cout << "agent_size:" <<agents.size() << endl;
    for(iter = agents.begin(); iter!=agents.end();iter++)
    {
        cout <<"agent_id:" <<iter->first << "  "<< iter->second.agent_id << \
        
        "  "<< iter->second.state_current->position->x << \
       
        "  "<< iter->second.state_current->position->y << endl; 
    }
}


comm_simulator Simulator::get_communicator_info(MPI_Comm communicator_in){


    int rank;
    MPI_Comm_rank(communicator_in, &rank);
    int comm_size;
    MPI_Comm_size(communicator_in, &comm_size);

    comm_simulator communicator_info;
    communicator_info.rank = rank;
    communicator_info.size = comm_size;

    
    return communicator_info;

}


Simulator::~Simulator()
{

    delete idprovider;
    delete world;
    delete my_partition;

}
const vector<agent_S> Simulator::default_migrate_list={};




