#include "SelfTest.h"
#include <iostream>

using namespace std;
using namespace dmq;

SelfTest::SelfTest(uint8_t maxStates) :
    StateMachine(maxStates)
{
    m_stateMap[ST_IDLE].action      += MakeStateDelegate(this, &SelfTest::ST_Idle);
    m_stateMap[ST_IDLE].entry       += MakeStateDelegate(this, &SelfTest::EN_EntryIdle);
    m_stateMap[ST_COMPLETED].action += MakeStateDelegate(this, &SelfTest::ST_Completed);
    m_stateMap[ST_FAILED].action    += MakeStateDelegate(this, &SelfTest::ST_Failed);
}

void SelfTest::Cancel()
{
    PARENT_TRANSITION(ST_FAILED)

    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_COMPLETED
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_FAILED
    END_TRANSITION_MAP(nullptr)
}

void SelfTest::ST_Idle(const NoEventData* data)
{
    cout << "SelfTest::ST_Idle" << endl;
}

void SelfTest::EN_EntryIdle(const NoEventData* data)
{
    cout << "SelfTest::EN_EntryIdle" << endl;
}

void SelfTest::ST_Completed(const NoEventData* data)
{
    cout << "SelfTest::ST_Completed" << endl;
    InternalEvent(ST_IDLE);
}

void SelfTest::ST_Failed(const NoEventData* data)
{
    cout << "SelfTest::ST_Failed" << endl;
    InternalEvent(ST_IDLE);
}
