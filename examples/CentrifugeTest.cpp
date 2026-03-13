#include "CentrifugeTest.h"
#include <iostream>

using namespace std;
using namespace dmq;

CentrifugeTest::CentrifugeTest() :
    SelfTest(ST_MAX_STATES),
    m_pollActive(false),
    m_speed(0)
{
    // ST_IDLE: override the base action registered by SelfTest, keeping its entry.
    m_stateMap[ST_IDLE].action.Clear();
    m_stateMap[ST_IDLE].action += MakeStateDelegate(this, &CentrifugeTest::ST_Idle);

    m_stateMap[ST_START_TEST].action            += MakeStateDelegate(this, &CentrifugeTest::ST_StartTest);
    m_stateMap[ST_START_TEST].guard              = MakeGuardDelegate(this, &CentrifugeTest::GD_GuardStartTest);

    m_stateMap[ST_ACCELERATION].action          += MakeStateDelegate(this, &CentrifugeTest::ST_Acceleration);

    m_stateMap[ST_WAIT_FOR_ACCELERATION].action += MakeStateDelegate(this, &CentrifugeTest::ST_WaitForAcceleration);
    m_stateMap[ST_WAIT_FOR_ACCELERATION].exit   += MakeExitDelegate(this, &CentrifugeTest::EX_ExitWaitForAcceleration);

    m_stateMap[ST_DECELERATION].action          += MakeStateDelegate(this, &CentrifugeTest::ST_Deceleration);

    m_stateMap[ST_WAIT_FOR_DECELERATION].action += MakeStateDelegate(this, &CentrifugeTest::ST_WaitForDeceleration);
    m_stateMap[ST_WAIT_FOR_DECELERATION].exit   += MakeExitDelegate(this, &CentrifugeTest::EX_ExitWaitForDeceleration);
}

void CentrifugeTest::Start()
{
    BEGIN_TRANSITION_MAP                            // - Current State -
        TRANSITION_MAP_ENTRY(ST_START_TEST)         // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)         // ST_COMPLETED
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)         // ST_FAILED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_START_TEST
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_ACCELERATION
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_WAIT_FOR_ACCELERATION
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_DECELERATION
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)         // ST_WAIT_FOR_DECELERATION
    END_TRANSITION_MAP(nullptr)
}

void CentrifugeTest::Poll()
{
    BEGIN_TRANSITION_MAP                                    // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)                 // ST_IDLE
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)                 // ST_COMPLETED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)                 // ST_FAILED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)                 // ST_START_TEST
        TRANSITION_MAP_ENTRY(ST_WAIT_FOR_ACCELERATION)      // ST_ACCELERATION
        TRANSITION_MAP_ENTRY(ST_WAIT_FOR_ACCELERATION)      // ST_WAIT_FOR_ACCELERATION
        TRANSITION_MAP_ENTRY(ST_WAIT_FOR_DECELERATION)      // ST_DECELERATION
        TRANSITION_MAP_ENTRY(ST_WAIT_FOR_DECELERATION)      // ST_WAIT_FOR_DECELERATION
    END_TRANSITION_MAP(nullptr)
}

void CentrifugeTest::ST_Idle(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_Idle" << endl;
    SelfTest::ST_Idle(data);
    StopPoll();
}

void CentrifugeTest::ST_StartTest(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_StartTest" << endl;
    InternalEvent(ST_ACCELERATION);
}

bool CentrifugeTest::GD_GuardStartTest(const NoEventData* data)
{
    cout << "CentrifugeTest::GD_GuardStartTest" << endl;
    return m_speed == 0;
}

void CentrifugeTest::ST_Acceleration(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_Acceleration" << endl;
    StartPoll();
}

void CentrifugeTest::ST_WaitForAcceleration(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_WaitForAcceleration : Speed is " << m_speed << endl;
    if (++m_speed >= 5)
        InternalEvent(ST_DECELERATION);
}

void CentrifugeTest::EX_ExitWaitForAcceleration()
{
    cout << "CentrifugeTest::EX_ExitWaitForAcceleration" << endl;
    StopPoll();
}

void CentrifugeTest::ST_Deceleration(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_Deceleration" << endl;
    StartPoll();
}

void CentrifugeTest::ST_WaitForDeceleration(const NoEventData* data)
{
    cout << "CentrifugeTest::ST_WaitForDeceleration : Speed is " << m_speed << endl;
    if (m_speed-- == 0)
        InternalEvent(ST_COMPLETED);
}

void CentrifugeTest::EX_ExitWaitForDeceleration()
{
    cout << "CentrifugeTest::EX_ExitWaitForDeceleration" << endl;
    StopPoll();
}
