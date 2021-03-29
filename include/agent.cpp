#include "agent.h"
#include <iostream>

using namespace std;

Position::Position()
{
    agent.agent_id = 0;
    agent.diameter = 1;
    agent.state_current.position.x = 0;
    agent.state_current.position.y = 0;
    agent.state_current.trajectory.x = 0.0f;
    agent.state_current.trajectory.y = 0.0f;

  
}

Position::Position(float x_pos, float y_pos)
{
    
    x = x_pos;
    y = y_pos;
}

void Position::get_position_xy()
{
    cout << "x:" << x << " y:" << y << endl;
    return;
}

Trajectory::Trajectory()
{
    int i = 0;
}

Trajectory::Trajectory(float x_pos, float y_pos)
{
    x = x_pos;
    y = y_pos;
}

AgentState::AgentState()
{
   
}


Agent::Agent()
{
    int i=0;
}

 Agent::Agent(int agent_id_in){

     agent_id = agent_id_in;
    //  state_next = NULL;
     state_current_agent = NULL;

    
 }


