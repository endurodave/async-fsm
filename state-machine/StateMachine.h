#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H

// @see https://github.com/endurodave/delegate-fsm
// David Lafreniere

#include <cstdint>
#include <memory>
#include <vector>
#include "delegate-mq/DelegateMQ.h"
#include "delegate-mq/predef/util/Fault.h"

/// @brief Base class for all state machine event data.
class EventData {
public:
    virtual ~EventData() = default;
};
using NoEventData = EventData;

/// @brief One row in the state map. All fields are optional delegates.
///
/// Populate in the derived class constructor using the adapter helpers:
///
///   m_stateMap[ST_START].action += MakeStateDelegate(this, &Motor::ST_Start);
///   m_stateMap[ST_START].guard   = MakeGuardDelegate(this, &Motor::GD_Start);
///   m_stateMap[ST_START].entry  += MakeStateDelegate(this, &Motor::EN_Start);
///   m_stateMap[ST_START].exit   += MakeExitDelegate(this, &Motor::EX_Start);
///
/// action/entry/exit are MulticastDelegate so logging or test hooks can attach
/// without touching SM logic:
///   motor.m_stateMap[Motor::ST_START].action +=
///       MakeDelegate(logger, &Logger::LogAction);
struct StateMapRow {
    /// State action. Fires on every visit, including self-transitions.
    dmq::MulticastDelegate<void(std::shared_ptr<const EventData>)> action;
    /// Optional guard. When bound and returning false the transition is vetoed.
    dmq::UnicastDelegate<bool(std::shared_ptr<const EventData>)>   guard;
    /// Optional entry action. Called only when the state actually changes.
    dmq::MulticastDelegate<void(std::shared_ptr<const EventData>)> entry;
    /// Optional exit action. Called only when the state actually changes.
    dmq::MulticastDelegate<void()>                                  exit;
};

/// @brief Delegate-based finite state machine base class.
///
/// Sync mode (default): ExternalEvent runs on the caller's thread.
/// Async / active-object mode: call SetThread() with a dedicated IThread.
/// ExternalEvent then marshals to that thread; callers return immediately.
/// All state logic, signals, and guards fire on the SM thread — no mutex needed.
class StateMachine {
public:
    enum : uint8_t { EVENT_IGNORED = 0xFE, CANNOT_HAPPEN = 0xFF };

    StateMachine(uint8_t maxStates, uint8_t initialState = 0);
    virtual ~StateMachine() = default;

    uint8_t GetCurrentState() const { return m_currentState; }
    uint8_t GetMaxStates()    const { return m_maxStates; }

    /// Enable active-object mode. ExternalEvent will marshal to @p thread.
    /// Call before the first ExternalEvent. Pass the same thread for the
    /// lifetime of the SM; switching threads at runtime is not supported.
    void SetThread(dmq::IThread& thread) { m_smThread = &thread; }

    /// Validate that every state in the map has at least one action delegate
    /// bound. Call once after construction (before the first event) to catch
    /// states added to the enum but never registered in the constructor.
    /// @param[in] onUnregistered  Optional callback invoked for each unregistered
    ///                            state index, e.g. to log the offending state.
    /// @return true if all states have an action bound; false otherwise.
    bool Validate(std::function<void(uint8_t)> onUnregistered = nullptr) const
    {
        bool valid = true;
        for (uint8_t i = 0; i < m_maxStates; ++i) {
            if (m_stateMap[i].action.Empty()) {
                valid = false;
                if (onUnregistered)
                    onUnregistered(i);
            }
        }
        return valid;
    }

    /// Fired after every completed state action: (fromState, toState).
    /// fromState == toState indicates a self-transition.
    /// Fires on the SM thread in async mode.
    dmq::Signal<void(uint8_t fromState, uint8_t toState)> OnTransition;

    /// Fired when entering a new state (only on actual state change, before action).
    /// Fires on the SM thread in async mode.
    dmq::Signal<void(uint8_t state)> OnEntry;

    /// Fired when exiting a state (only on actual state change, before entry).
    /// Fires on the SM thread in async mode.
    dmq::Signal<void(uint8_t state)> OnExit;

    /// Fired when a CANNOT_HAPPEN transition is triggered from @p currentState.
    /// Use this to log, record diagnostics, or initiate a controlled shutdown.
    /// If no subscriber is connected, or after the signal returns, ASSERT_TRUE(false)
    /// aborts execution — redefine FaultHandler for platform-specific behaviour.
    dmq::Signal<void(uint8_t currentState)> OnCannotHappen;

protected:
    /// Trigger an external state machine event. Call from public event methods.
    /// @param[in] newState  The state to transition to.
    /// @param[in] pData     Event data. In async mode the shared_ptr keeps the
    ///                      data alive on the queue until the SM thread processes it.
    void ExternalEvent(uint8_t newState, std::shared_ptr<const EventData> pData = nullptr);

    /// Trigger an internal event from within a running state action.
    /// Always executes synchronously on the current (SM) thread.
    void InternalEvent(uint8_t newState, std::shared_ptr<const EventData> pData = nullptr);

    /// State map populated by the derived class constructor.
    std::vector<StateMapRow> m_stateMap;

private:
    const uint8_t m_maxStates;
    uint8_t       m_currentState;
    uint8_t       m_newState;
    bool          m_eventGenerated;

    std::shared_ptr<const EventData> m_pEventData;

    /// Optional SM thread. Non-null enables active-object async dispatch.
    dmq::IThread* m_smThread = nullptr;

    /// Actual event implementation — runs on SM thread in async mode,
    /// called directly by ExternalEvent in sync mode.
    void ExternalEventImpl(uint8_t newState, std::shared_ptr<const EventData> pData);

    void StateEngine();
};

// ---------------------------------------------------------------------------
// Transition map macros — table-driven lookup is still optimal here.
// ---------------------------------------------------------------------------

#define BEGIN_TRANSITION_MAP \
    static const uint8_t TRANSITIONS[] = {

#define TRANSITION_MAP_ENTRY(entry) \
    entry,

#define END_TRANSITION_MAP(data) \
    }; \
    static_assert((sizeof(TRANSITIONS) / sizeof(uint8_t)) == ST_MAX_STATES, \
        "Transition map size does not match ST_MAX_STATES"); \
    ExternalEvent(TRANSITIONS[GetCurrentState()], data);

/// Used in derived state machines that extend another SM via inheritance.
#define PARENT_TRANSITION(state) \
    if (GetCurrentState() >= ST_MAX_STATES && \
        GetCurrentState() < GetMaxStates()) { \
        ExternalEvent(state); \
        return; }

// ---------------------------------------------------------------------------
// Typed delegate adapters.
//
// These wrap typed member functions into the shared_ptr<const EventData>
// signature expected by StateMapRow, enforcing the correct function signature
// at the point of registration via template deduction.
// ---------------------------------------------------------------------------

template <class SM, class Data>
auto MakeStateDelegate(SM* obj, void(SM::*func)(const Data*))
{
    return dmq::MakeDelegate(
        std::function<void(std::shared_ptr<const EventData>)>(
            [obj, func](std::shared_ptr<const EventData> data) {
                (obj->*func)(static_cast<const Data*>(data.get()));
            }));
}

template <class SM, class Data>
auto MakeGuardDelegate(SM* obj, bool(SM::*func)(const Data*))
{
    return dmq::MakeDelegate(
        std::function<bool(std::shared_ptr<const EventData>)>(
            [obj, func](std::shared_ptr<const EventData> data) {
                return (obj->*func)(static_cast<const Data*>(data.get()));
            }));
}

template <class SM>
auto MakeExitDelegate(SM* obj, void(SM::*func)())
{
    return dmq::MakeDelegate(
        std::function<void()>(
            [obj, func]() {
                (obj->*func)();
            }));
}

#endif // _STATE_MACHINE_H
