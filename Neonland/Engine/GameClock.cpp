#include "GameClock.hpp"
#include <chrono>

namespace {

double SteadyTime() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

}

GameClock::GameClock(bool paused)
: _startTime{SteadyTime()}
, _time{0} {
    paused = false;
    Paused(paused);
}

void GameClock::Reset() {
    _time = 0;
    _startTime = SteadyTime();
}

void GameClock::Paused(bool paused) {
    if (paused == _paused) { return; }
    
    if (paused) {
        auto now = SteadyTime();
        _time += now - _startTime;
        _startTime = now;
    }
    
    _paused = paused;
}

bool GameClock::Paused() const { return _paused; }

double GameClock::Time() const {
    if (_paused) { return _time; }
    return _time + (SteadyTime() - _startTime);
}
