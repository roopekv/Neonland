#pragma once

class GameClock {
public:
    GameClock();
    
    double Time() const;
    
    void Reset();
    
    void Paused(bool paused);
    bool Paused() const;
    
private:
    double _startTime;
    double _time = 0;
    bool _paused;
};
