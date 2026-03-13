#ifndef _CENTRIFUGE_TEST_H
#define _CENTRIFUGE_TEST_H

#include "SelfTest.h"

/// @brief CentrifugeTest demonstrates state machine inheritance, state
/// function override, and guard/entry/exit actions. SelfTest provides the
/// common base states.
class CentrifugeTest : public SelfTest
{
public:
    CentrifugeTest();

    virtual void Start();
    void Poll();

    bool IsPollActive() { return m_pollActive; }

private:
    bool m_pollActive;
    int  m_speed;

    void StartPoll() { m_pollActive = true; }
    void StopPoll()  { m_pollActive = false; }

    enum States
    {
        ST_START_TEST = SelfTest::ST_MAX_STATES,
        ST_ACCELERATION,
        ST_WAIT_FOR_ACCELERATION,
        ST_DECELERATION,
        ST_WAIT_FOR_DECELERATION,
        ST_MAX_STATES
    };

    // States
    void ST_Idle(const NoEventData* data);
    void ST_StartTest(const NoEventData* data);
    void ST_Acceleration(const NoEventData* data);
    void ST_WaitForAcceleration(const NoEventData* data);
    void ST_Deceleration(const NoEventData* data);
    void ST_WaitForDeceleration(const NoEventData* data);

    // Guard
    bool GD_GuardStartTest(const NoEventData* data);

    // Exit
    void EX_ExitWaitForAcceleration();
    void EX_ExitWaitForDeceleration();
};

#endif
