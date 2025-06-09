#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <Arduino.h>

// A simple container for data that needs to be accessed by multiple processes.
// This avoids direct dependencies between managers for data retrieval.
struct SharedState {
    String macAddress;
    bool wifiConnected = false;
};

#endif // SHARED_STATE_H 