#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <vector>
#include <map>
#include "Event.h"
#include "Process.h"

// The EventManager acts as a central bus for sending and receiving events.
// This decouples the processes from each other.
class EventManager {
public:
    void subscribe(EventType type, Process* process) {
        subscribers[type].push_back(process);
    }

    void publish(Event& event) {
        if (subscribers.find(event.type) != subscribers.end()) {
            for (auto* process : subscribers[event.type]) {
                if(process) {
                    process->onEvent(event);
                }
            }
        }
    }
private:
    std::map<EventType, std::vector<Process*>> subscribers;
};

#endif 