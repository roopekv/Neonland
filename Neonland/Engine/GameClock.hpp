#pragma once

class GameClock {
public:
    GameClock(bool paused = false);
    
    double Time() const;
    
    void Reset();
    
    void Paused(bool paused);
    bool Paused() const;
    
private:
    double _startTime;
    double _time;
    bool _paused;
};
