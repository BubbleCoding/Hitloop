#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <HTTPClient.h>
#include "Process.h"
#include "Configuration.h"
#include "EventManager.h"

class HTTPManager : public Process {
public:
    HTTPManager(Configuration& config) 
        : cfg(config) {}
    
    void setup(EventManager* em) override {
        Process::setup(em);
        eventManager->subscribe(EVT_DATA_READY_FOR_HTTP, this);
    }
    
    void onEvent(Event& event) override {
        if (event.type == EVT_DATA_READY_FOR_HTTP) {
            DataReadyForHttpEvent& e = static_cast<DataReadyForHttpEvent&>(event);
            sendData(e.jsonData);
        }
    }

    void update() override {
        // This manager is reactive, so it does nothing in its update loop.
    }

private:
    void sendData(String& jsonPayload) {
        HTTPClient http;
        http.begin(cfg.serverUrl.c_str());
        http.addHeader("Content-Type", "application/json");

        Serial.println("Sending JSON: " + jsonPayload);
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Received response: " + payload);
            HttpResponseEvent responseEvent(payload);
            eventManager->publish(responseEvent);
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    }

    Configuration& cfg;
};

#endif // HTTP_MANAGER_H 