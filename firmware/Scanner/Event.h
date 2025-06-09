#ifndef EVENT_H
#define EVENT_H

#include <BLEScan.h>
#include <Arduino.h>

class Process; // Forward declaration

enum EventType {
    EVT_SCAN_COMPLETE,
    EVT_HTTP_RESPONSE_RECEIVED,
    EVT_DATA_READY_FOR_HTTP
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

#endif 