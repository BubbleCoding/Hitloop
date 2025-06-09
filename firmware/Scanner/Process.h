#ifndef PROCESS_H
#define PROCESS_H

#include "Event.h"

class EventManager; // Forward declaration

class Process {
public:
    virtual ~Process() {}
    virtual void setup(EventManager* em) { this->eventManager = em; }
    virtual void update() = 0;
    virtual void onEvent(Event& event) {}

protected:
    EventManager* eventManager = nullptr;
};

#endif // PROCESS_H 