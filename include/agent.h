#ifndef _AGENT_H
#define _AGENT_H

#include"world.h"
#include<iostream>
using namespace std;

struct position_S
{
    float x=0;
    float y=0;
};

struct trajectory_S
{
    float x=0;
    float y=0;
};


struct state_current_S{
    position_S position;
    trajectory_S trajectory;

};

struct agent_S{

    int agent_id = 0;
    int diameter = 0;
    int state_next = 0;
    state_current_S state_current;


};







class Position
{
public:
    Position(); //default constructor
    Position(float x_pos, float y_pos); //none default constructor

    void get_position_xy();

    float x;
    float y;
    agent_S agent;

};



class Trajectory: public Position
{
public:
    Trajectory(); //default constructor
    Trajectory(float x_pos, float y_pos); //none default constructor
};



class AgentState
{
    public:
        AgentState();

};





class Agent
{
public:
    Agent();
    Agent(int agent_id_in);
    
    int agent_id;
    AgentState *state_next_agent;
    AgentState *state_current_agent;
    float diameter;


};


#endif //_AGENT_H


