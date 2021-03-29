#ifndef _BALL_H
#define _BALL_H

#include "agent.h"
#include "world.h"



struct ball
{
    /* data */
};



class BallState: public AgentState

{
public:
    BallState();
    BallState(Position *pos_in , Trajectory *traj_in);

    Position *position;
    Trajectory *trajectory;

};


class Ball: public Agent
{

    public:
        Ball();
        Ball(Position *pos_in , Trajectory *traj_in, float diameter_in, int agent_id_in=-1); ////构建函数
        // static Ball* getInstance(); //获取实例指针
        
        void update_state(World *my_world_in);
        void _bounce_walls(BallState *state_new_in, World *my_world_in);
        void commit_state_update();

        BallState *state_next;
        BallState *state_current;


    private:
        // static Ball *pBall; //创建的唯一对象指针
        int none = 0;



};

#endif //_BALL_H