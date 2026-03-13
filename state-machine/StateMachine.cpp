#include "StateMachine.h"

using namespace dmq;

//----------------------------------------------------------------------------
// StateMachine
//----------------------------------------------------------------------------
StateMachine::StateMachine(uint8_t maxStates, uint8_t initialState)
    : m_maxStates(maxStates)
    , m_currentState(initialState)
    , m_newState(0)
    , m_eventGenerated(false)
    , m_stateMap(maxStates)
{
    ASSERT_TRUE(maxStates < EVENT_IGNORED);
}

//----------------------------------------------------------------------------
// ExternalEvent
//
// shared_ptr<const EventData> is used rather than a raw pointer for two reasons:
//   1. Ownership is unambiguous — no heap-allocation contract to document.
//   2. Async safety — DelegateMQ copies shared_ptr by value across the thread
//      boundary, keeping the derived type alive without slicing. A raw pointer
//      would be deep-copied as EventData by make_tuple_heap, losing derived
//      class data (e.g. MotorData::speed).
//----------------------------------------------------------------------------
void StateMachine::ExternalEvent(uint8_t newState, std::shared_ptr<const EventData> pData)
{
    if (newState == EVENT_IGNORED)
        return;

    if (newState == CANNOT_HAPPEN) {
        OnCannotHappen(m_currentState);
        ASSERT_TRUE(false);
        return;
    }

    if (m_smThread)
        MakeDelegate(this, &StateMachine::ExternalEventImpl, *m_smThread)(newState, pData);
    else
        ExternalEventImpl(newState, pData);
}

//----------------------------------------------------------------------------
// ExternalEventImpl
//----------------------------------------------------------------------------
void StateMachine::ExternalEventImpl(uint8_t newState, std::shared_ptr<const EventData> pData)
{
    InternalEvent(newState, pData);
    StateEngine();
}

//----------------------------------------------------------------------------
// InternalEvent
//----------------------------------------------------------------------------
void StateMachine::InternalEvent(uint8_t newState, std::shared_ptr<const EventData> pData)
{
    if (!pData)
        pData = std::make_shared<NoEventData>();

    m_pEventData     = pData;
    m_eventGenerated = true;
    m_newState       = newState;
}

//----------------------------------------------------------------------------
// StateEngine
//----------------------------------------------------------------------------
void StateMachine::StateEngine()
{
    while (m_eventGenerated) {
        ASSERT_TRUE(m_newState < m_maxStates);

        StateMapRow& row   = m_stateMap[m_newState];
        auto         pData = m_pEventData;
        m_pEventData       = nullptr;
        m_eventGenerated   = false;

        if (row.guard && !row.guard(pData))
            continue;

        const uint8_t fromState = m_currentState;
        const bool    changing  = (m_newState != m_currentState);

        if (changing) {
            m_stateMap[m_currentState].exit();
            OnExit(m_currentState);

            m_currentState = m_newState;
            ASSERT_TRUE(m_eventGenerated == false);

            row.entry(pData);
            OnEntry(m_currentState);
            ASSERT_TRUE(m_eventGenerated == false);
        } else {
            m_currentState = m_newState;
        }

        row.action(pData);
        OnTransition(fromState, m_currentState);
    }
}
