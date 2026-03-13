#include "Motor.h"
#include <iostream>

using namespace std;
using namespace dmq;

Motor::Motor() :
    StateMachine(ST_MAX_STATES),
    m_currentSpeed(0)
{
    m_stateMap[ST_IDLE].action         += MakeStateDelegate(this, &Motor::ST_Idle);
    m_stateMap[ST_STOP].action         += MakeStateDelegate(this, &Motor::ST_Stop);
    m_stateMap[ST_START].action        += MakeStateDelegate(this, &Motor::ST_Start);
    m_stateMap[ST_CHANGE_SPEED].action += MakeStateDelegate(this, &Motor::ST_ChangeSpeed);
}

void Motor::SetSpeed(std::shared_ptr<MotorData> data)
{
    BEGIN_TRANSITION_MAP                            // - Current State -
        TRANSITION_MAP_ENTRY(ST_START)              // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)         // ST_STOP
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)       // ST_START
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)       // ST_CHANGE_SPEED
    END_TRANSITION_MAP(data)
}

void Motor::Halt()
{
    BEGIN_TRANSITION_MAP                            // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)         // ST_STOP
        TRANSITION_MAP_ENTRY(ST_STOP)               // ST_START
        TRANSITION_MAP_ENTRY(ST_STOP)               // ST_CHANGE_SPEED
    END_TRANSITION_MAP(nullptr)
}

void Motor::ST_Idle(const NoEventData* data)
{
    cout << "Motor::ST_Idle" << endl;
}

void Motor::ST_Stop(const NoEventData* data)
{
    cout << "Motor::ST_Stop" << endl;
    m_currentSpeed = 0;
    InternalEvent(ST_IDLE);
}

void Motor::ST_Start(const MotorData* data)
{
    cout << "Motor::ST_Start : Speed is " << data->speed << endl;
    m_currentSpeed = data->speed;
}

void Motor::ST_ChangeSpeed(const MotorData* data)
{
    cout << "Motor::ST_ChangeSpeed : Speed is " << data->speed << endl;
    m_currentSpeed = data->speed;
}
