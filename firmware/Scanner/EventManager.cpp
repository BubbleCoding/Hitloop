#include "EventManager.h"

void EventManager::subscribe(EventType type, Process* process) {
    subscribers[type].push_back(process);
}

void EventManager::publish(Event& event) {
    // Check if there are any subscribers for this event type
    if (subscribers.find(event.type) != subscribers.end()) {
        // If so, iterate through them and call their onEvent handler
        for (auto* process : subscribers[event.type]) {
            if (process) { // Safety check
                process->onEvent(event);
            }
        }
    }
} 