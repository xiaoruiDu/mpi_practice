#ifndef _WORLD_H
#define _WORLD_H

#include<iostream>
#include <math.h>
#include <algorithm>  
#include <string>
#include <map>
#include <vector>
#include <assert.h>
using namespace std;

struct position_xy
{
    float x_pos;
    float y_pos;
};



class Partition{

    public:
        Partition(float x_min_in, float y_min_in, float x_max_in, float y_max_in, int partition_id_in);
        int partition_id;
    private:
        float x_min;
        float y_min;
        float x_max;
        float y_max;
        // int partition_id;
        // Partition *position_S;



};

struct position_map_partition
{
    position_xy pos;  //(0,0),(0,1),(0,2)(0,3)
    Partition *position_S;  // partition_id + area(x_min,x_max,y_min,y_max)

};

struct id_map_partition
{
    int id;
    Partition *position_S;

};



class World{
    public:
        World();
        World(float x_min_in, float y_min_in, float x_max_in, float y_max_in,
                float grid_width_in, float grid_height_in,float comm_size_in);

        Partition *map_partition(float x, float y);
        Partition *get_region_by_id(int partition_id);

        float x_min;
        float y_min;
        float x_max;
        float y_max;
        
        float grid_width;
        float grid_height;
        int comm_size;

        vector<position_map_partition> _partitions_by_position;
        vector<id_map_partition> _partitions_by_id;

        void init_partitions();

    private:
        

};


#endif //_WORLD_H