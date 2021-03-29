#include "world.h"


Partition::Partition(float x_min_in, float y_min_in, float x_max_in, \
                    float y_max_in, int partition_id_in)
{
    x_min = x_min_in;
    y_min = y_min_in;
    x_max = x_max_in;
    y_max = y_max_in;
    partition_id = partition_id_in;
    

}

World::World()
{
    return;
}

World::World(float x_min_in, float y_min_in, float x_max_in, float y_max_in,
                float grid_width_in, float grid_height_in,float comm_size_in)
{
    // cout << "configure simulation world" << endl;
    x_min = x_min_in;
    y_min = y_min_in;
    x_max = x_max_in;
    y_max = y_max_in;
    grid_width = grid_width_in;
    grid_height = grid_height_in;
    comm_size = comm_size_in - 1;

    _partitions_by_id.clear();
    _partitions_by_position.clear();

    init_partitions();


}

void World::init_partitions(){

    int partition_i = 0;
    vector<position_map_partition> partitions_by_position_vector;
    vector<id_map_partition> partitions_by_id_vector;
    // std::map<position_xy,Partition> partitions_by_position;
    // std::map<int,Partition> partitions_by_id;

    float dy = y_max / comm_size;
    float grid_x = 0;

    for(int grid_y=0; grid_y<comm_size; ++grid_y)
    {
        Partition *partition;
        // position_map_partition *pos_map_part = new position_map_partition;
        
        position_map_partition pos_map_part;
        id_map_partition id_map_part;
        partition = new Partition(x_min, 
                                y_min+grid_y *dy,
                                x_min,
                                y_min + (grid_y + 1) * dy,
                                partition_i
                                );
        position_xy pos;
        pos.x_pos = grid_x;
        pos.y_pos = grid_y;
        pos_map_part.position_S = partition;
        pos_map_part.pos = pos;
        id_map_part.id = partition_i;
        id_map_part.position_S = partition;

        partitions_by_position_vector.push_back(pos_map_part);
        partitions_by_id_vector.push_back(id_map_part);
        
        partition_i += 1;

        // cout <<"class object:" <<partitions_by_position_vector[partition_i-1].position_S->partition_id \
        // << "posxy:" << partitions_by_position_vector[partition_i-1].pos.x_pos << " " << partitions_by_position_vector[partition_i-1].pos.y_pos << endl;

    }
    _partitions_by_position = partitions_by_position_vector;
    _partitions_by_id = partitions_by_id_vector;

    // for(int i=0; i<_partitions_by_position.size(); ++i)
    // {
    //     cout <<"class object:" <<_partitions_by_position[i].position_S->partition_id \
    //     << "posxy:" << _partitions_by_position[i].pos.x_pos << " " << _partitions_by_position[i].pos.y_pos << endl;

    //     cout << "id_map_partition:" << _partitions_by_id[i].id << "class object:" \
    //      << _partitions_by_id[i].position_S->partition_id;


    // }   
}

Partition *World::get_region_by_id(int partition_id)
{   
    // if(x_min <= x)
    // vector<id_map_partition>::iterator itr;
    // for(itr=_partitions_by_id.begin(); itr!=_partitions_by_id.end();++itr)
    // {
    //     if(itr->id == partition_id)
    //     {
    //         return itr->position_S;
    //     }
    // }



    int i;
    for(i=0; i<_partitions_by_id.size(); ++i)
    {
        if(_partitions_by_id[i].id == partition_id)
            return _partitions_by_id[i].position_S;
    }
    if(i>_partitions_by_id.size())
    {
        cout << "partition_id illegal....";
        return NULL;
    }
    
}


Partition *World::map_partition(float x, float y)
{
    // Partition *ptr = new Partition();
    // if(x<x_min || x>x_max || y>y_max || y<y_min)
    // {
    //     printf("illegal x_y: %f  %f \n",x,y);
    // }

    assert(x_min <= x && x <= x_max && y_min <= y && y <= y_max );
    
    float grid_y = min(floor(y / (y_max / comm_size)), (float)(comm_size - 1));
    float grid_x = 0;

    position_xy pos;
    pos.x_pos = grid_x;
    pos.y_pos = grid_y;
    int i;
    // cout << "_partitions_by_position.size:" <<_partitions_by_position.size()<<endl;
    for(i=0; i<_partitions_by_position.size(); ++i)
    {   
        
        // cout << "_partitions_x_pos:"<<_partitions_by_position[i].pos.x_pos <<" -->" <<pos.x_pos << endl;
        // cout << "_partitions_y_pos:"<<_partitions_by_position[i].pos.y_pos <<" -->" <<pos.y_pos<< endl;
        

        if(_partitions_by_position[i].pos.x_pos == pos.x_pos && \
            _partitions_by_position[i].pos.y_pos == pos.y_pos)
        {
            // printf("_partitions_by_position[i].pos.x_pos: %d\n", _partitions_by_position[i].position_S->partition_id);
            // Partition *p =  _partitions_by_position[i].position_S;
            // cout << "find matche_" << endl;
            return _partitions_by_position[i].position_S;

        }

            

    }

    

    if(i >= _partitions_by_position.size())
    {
        cout << "partition illegal..."<<endl;
        return NULL;
    }





    
    // return _partitions_by_position[1];

    

}
