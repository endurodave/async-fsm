#ifndef _SELF_TEST_H
#define _SELF_TEST_H

#include "StateMachine.h"

/// @brief SelfTest is a subclass state machine for other self tests to
/// inherit from. The class has common states for all derived classes to share.
class SelfTest : public StateMachine
{
public:
    SelfTest(uint8_t maxStates);

    virtual void Start() = 0;
    void Cancel();

protected:
    enum States
    {
        ST_IDLE,
        ST_COMPLETED,
        ST_FAILED,
        ST_MAX_STATES
    };

    // States
    void ST_Idle(const NoEventData* data);
    void ST_Completed(const NoEventData* data);
    void ST_Failed(const NoEventData* data);

    // Entry
    void EN_EntryIdle(const NoEventData* data);
};

#endif
