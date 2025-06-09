#ifndef EVENT_H
#define EVENT_H

#include <BLEScan.h>
#include <Arduino.h>

class Process; // Forward declaration

enum EventType {
    EVT_SCAN_COMPLETE,
    EVT_DATA_READY_FOR_HTTP,
    EVT_HTTP_RESPONSE_RECEIVED,
    EVT_WIFI_CONNECTED,
    EVT_SYNC_TIMER,
    EVT_SERVER_DISCONNECTED,
    // Add other event types here
};

// Base class for all events
struct Event {
    EventType type;
    Event(EventType t) : type(t) {}
    virtual ~Event() {}
};

struct ScanCompleteEvent : Event {
    BLEScanResults results;
    float avgAngleXZ;
    float avgAngleYZ;
    float totalMovement;

    ScanCompleteEvent(BLEScanResults res, float axz, float ayz, float move)
        : Event(EVT_SCAN_COMPLETE), results(res), avgAngleXZ(axz), avgAngleYZ(ayz), totalMovement(move) {}
};

struct HttpResponseEvent : Event {
    String response;
    HttpResponseEvent(String resp)
        : Event(EVT_HTTP_RESPONSE_RECEIVED), response(resp) {}
};

struct DataReadyForHttpEvent : Event {
    String jsonData;
    DataReadyForHttpEvent(String data)
        : Event(EVT_DATA_READY_FOR_HTTP), jsonData(data) {}
};

struct WifiConnectedEvent : public Event {
    WifiConnectedEvent() : Event(EVT_WIFI_CONNECTED) {}
};

struct SyncTimerEvent : public Event {
    unsigned long wait_ms;
    SyncTimerEvent(unsigned long wait)
        : Event(EVT_SYNC_TIMER), wait_ms(wait) {}
};

struct ServerDisconnectedEvent : public Event {
    ServerDisconnectedEvent() : Event(EVT_SERVER_DISCONNECTED) {}
};

#endif 