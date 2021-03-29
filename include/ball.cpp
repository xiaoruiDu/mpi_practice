#include "ball.h"



BallState::BallState()
{
   
}

// initialization_2
BallState::BallState(Position *pos_in , Trajectory *traj_in)
{
   position = pos_in;
   trajectory = traj_in;
}



Ball::Ball()
{

}

// initialization 
Ball::Ball(Position *pos_in , Trajectory *traj_in, float diameter_in, int agent_id_in){

    agent_id = agent_id_in;

    state_current = new BallState(pos_in, traj_in);
    diameter = diameter_in;
    state_next = NULL;

    
}

void Ball::update_state(World *my_world_in)
{   

    // printf("before_state_current->position->x:%f   %f\n",state_current->position->x,state_current->position->y);
    Position *pos_new;
    Trajectory *traj;
    pos_new = new Position(state_current->position->x + state_current->trajectory->x,\
                        state_current->position->y + state_current->trajectory->y);

    traj = new Trajectory(state_current->trajectory->x, state_current->trajectory->y);
    
    BallState *state_new;
    state_new = new BallState(pos_new, traj);

    // printf("after_state_next->position->x:%f    %f\n",state_new->position->x, state_new->position->y);
    
    _bounce_walls(state_new, my_world_in);
    state_next = state_new;

    return;
}

void Ball::_bounce_walls(BallState *state_new_in, World *my_world_in)
{   

    state_new_in->position->x = max(state_new_in->position->x, my_world_in->x_min);
    state_new_in->position->x = min(state_new_in->position->x, my_world_in->x_max);
    
    if (state_new_in->position->x == my_world_in->x_min || \
        state_new_in->position->x == my_world_in->x_max)
    {
        state_new_in->trajectory->x *= -1;

    }
    state_new_in->position->y = max(state_new_in->position->y, my_world_in->y_min);
    state_new_in->position->y = min(state_new_in->position->y, my_world_in->y_max);

    if (state_new_in->position->y == my_world_in->y_min || \
        state_new_in->position->y == my_world_in->y_max)
    {
        state_new_in->trajectory->y *= -1;
    }
    
    // printf("likai_X: %f", state_new_in->position->x);
    
    return;
}


void Ball::commit_state_update(){

    // printf("noting\n");
    // cout << "state_next_agent:" << state_next->position->x << endl;
    if(state_next!=NULL)
    {   
        
        state_current = state_next;
        state_next = NULL;
    }

}