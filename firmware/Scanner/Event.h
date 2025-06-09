#ifndef EVENT_H
#define EVENT_H

#include <BLEScan.h>
#include <Arduino.h>

enum EventType {
    EVT_NONE,
    EVT_SCAN_COMPLETE,
    EVT_DATA_READY_FOR_HTTP,
    EVT_HTTP_RESPONSE
};

// Base event struct
struct Event {
    Event(EventType type) : type(type) {}
    virtual ~Event() {}
    const EventType type;
};

// Event fired by BleManager when a scan is finished
struct ScanCompleteEvent : public Event {
    ScanCompleteEvent(BLEScanResults& res) 
        : Event(EVT_SCAN_COMPLETE), results(res) {}
    BLEScanResults& results;
};

// Event fired by DataManager when JSON is ready to be sent
struct DataReadyForHttpEvent : public Event {
    DataReadyForHttpEvent(String& payload) 
        : Event(EVT_DATA_READY_FOR_HTTP), jsonPayload(payload) {}
    String& jsonPayload;
};

// Event fired by HTTPManager when a response from the server is received
struct HttpResponseEvent : public Event {
    HttpResponseEvent(String& payload)
        : Event(EVT_HTTP_RESPONSE), responsePayload(payload) {}
    String& responsePayload;
};

#endif 