#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <HTTPClient.h>
#include "Process.h"
#include "BehaviorManager.h"
#include "Configuration.h"

class HTTPManager : public Process {
public:
    HTTPManager(Config& config, BehaviorManager* behavior) 
        : cfg(config), behaviorManager(behavior) {}
    
    // This method is called by DataManager when data is ready
    void sendData(String& jsonPayload) {
        HTTPClient http;
        http.begin(cfg.serverUrl.c_str());
        http.addHeader("Content-Type", "application/json");

        Serial.println("Sending JSON: " + jsonPayload);
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Received response: " + payload);
            if (behaviorManager) {
                behaviorManager->handleServerResponse(payload);
            }
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
    }

    void update() override {
        // This manager is reactive, so it does nothing in its update loop.
    }

private:
    Config& cfg;
    BehaviorManager* behaviorManager;
};

#endif // HTTP_MANAGER_H 