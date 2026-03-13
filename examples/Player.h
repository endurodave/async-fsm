#ifndef _PLAYER_H
#define _PLAYER_H

#include "StateMachine.h"

class Player : public StateMachine
{
public:
    Player();

    void OpenClose();
    void Play();
    void Stop();
    void Pause();
    void EndPause();

private:
    enum States
    {
        ST_EMPTY,
        ST_OPEN,
        ST_STOPPED,
        ST_PAUSED,
        ST_PLAYING,
        ST_MAX_STATES
    };

    // States
    void ST_Empty(const NoEventData* data);
    void ST_Open(const NoEventData* data);
    void ST_Stopped(const NoEventData* data);
    void ST_Paused(const NoEventData* data);
    void ST_Playing(const NoEventData* data);
};

#endif
