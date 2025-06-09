#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <map>
#include <vector>
#include "Event.h"
#include "Process.h"

// The EventManager acts as a central bus for sending and receiving events.
// This decouples the processes from each other.
class EventManager {
public:
    void publish(Event& event);
    void subscribe(EventType type, Process* process);

private:
    std::map<EventType, std::vector<Process*>> subscribers;
};

#endif // EVENT_MANAGER_H 