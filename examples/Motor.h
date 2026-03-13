#ifndef _MOTOR_H
#define _MOTOR_H

#include "StateMachine.h"

class MotorData : public EventData
{
public:
    int speed;
};

class Motor : public StateMachine
{
public:
    Motor();

    void SetSpeed(std::shared_ptr<MotorData> data);
    void Halt();

private:
    int m_currentSpeed;

    enum States
    {
        ST_IDLE,
        ST_STOP,
        ST_START,
        ST_CHANGE_SPEED,
        ST_MAX_STATES
    };

    // States
    void ST_Idle(const NoEventData* data);
    void ST_Stop(const NoEventData* data);
    void ST_Start(const MotorData* data);
    void ST_ChangeSpeed(const MotorData* data);
};

#endif
