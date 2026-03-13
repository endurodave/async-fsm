#include "Player.h"
#include <iostream>

using namespace std;
using namespace dmq;

Player::Player() :
    StateMachine(ST_MAX_STATES)
{
    m_stateMap[ST_EMPTY].action   += MakeStateDelegate(this, &Player::ST_Empty);
    m_stateMap[ST_OPEN].action    += MakeStateDelegate(this, &Player::ST_Open);
    m_stateMap[ST_STOPPED].action += MakeStateDelegate(this, &Player::ST_Stopped);
    m_stateMap[ST_PAUSED].action  += MakeStateDelegate(this, &Player::ST_Paused);
    m_stateMap[ST_PLAYING].action += MakeStateDelegate(this, &Player::ST_Playing);
}

void Player::OpenClose()
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(ST_OPEN)           // ST_EMPTY
        TRANSITION_MAP_ENTRY(ST_EMPTY)          // ST_OPEN
        TRANSITION_MAP_ENTRY(ST_OPEN)           // ST_STOPPED
        TRANSITION_MAP_ENTRY(ST_OPEN)           // ST_PAUSED
        TRANSITION_MAP_ENTRY(ST_OPEN)           // ST_PLAYING
    END_TRANSITION_MAP(nullptr)
}

void Player::Play()
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_EMPTY
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_OPEN
        TRANSITION_MAP_ENTRY(ST_PLAYING)        // ST_STOPPED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PAUSED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PLAYING
    END_TRANSITION_MAP(nullptr)
}

void Player::Stop()
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_EMPTY
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_OPEN
        TRANSITION_MAP_ENTRY(ST_STOPPED)        // ST_STOPPED
        TRANSITION_MAP_ENTRY(ST_STOPPED)        // ST_PAUSED
        TRANSITION_MAP_ENTRY(ST_STOPPED)        // ST_PLAYING
    END_TRANSITION_MAP(nullptr)
}

void Player::Pause()
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_EMPTY
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_OPEN
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_STOPPED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PAUSED
        TRANSITION_MAP_ENTRY(ST_PAUSED)         // ST_PLAYING
    END_TRANSITION_MAP(nullptr)
}

void Player::EndPause()
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_EMPTY
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_OPEN
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_STOPPED
        TRANSITION_MAP_ENTRY(ST_PLAYING)        // ST_PAUSED
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_PLAYING
    END_TRANSITION_MAP(nullptr)
}

void Player::ST_Empty(const NoEventData* data)
{
    static bool CD_DetectedToggle = false;
    CD_DetectedToggle = !CD_DetectedToggle;

    cout << "Player::ST_Empty" << endl;
    if (CD_DetectedToggle)
        InternalEvent(ST_STOPPED);
}

void Player::ST_Open(const NoEventData* data)
{
    cout << "Player::ST_Open" << endl;
}

void Player::ST_Stopped(const NoEventData* data)
{
    cout << "Player::ST_Stopped" << endl;
}

void Player::ST_Paused(const NoEventData* data)
{
    cout << "Player::ST_Paused" << endl;
}

void Player::ST_Playing(const NoEventData* data)
{
    cout << "Player::ST_Playing" << endl;
}
