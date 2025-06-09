#ifndef BEHAVIOR_MANAGER_H
#define BEHAVIOR_MANAGER_H

#include <ArduinoJson.h>
#include <map>
#include <string>
#include "Process.h"
#include "LedManager.h"
#include "VibrationManager.h"
#include "EventManager.h"
#include "LedBehaviors.h"
#include "VibrationBehaviors.h"
#include "Utils.h"

class BehaviorManager : public Process {
public:
    BehaviorManager(LedManager* led, VibrationManager* vib) 
        : ledManager(led), vibrationManager(vib),
          ledsOff(), solid(), breathing(0), heartBeat(0,0), cycle(0,0),
          motorOff(), constant(0), burst(0,0), pulse(0,0)
    {
        ledBehaviorMap["Off"] = &ledsOff;
        ledBehaviorMap["Solid"] = &solid;
        ledBehaviorMap["Breathing"] = &breathing;
        ledBehaviorMap["HeartBeat"] = &heartBeat;
        ledBehaviorMap["Cycle"] = &cycle;

        vibrationBehaviorMap["Off"] = &motorOff;
        vibrationBehaviorMap["Constant"] = &constant;
        vibrationBehaviorMap["Burst"] = &burst;
        vibrationBehaviorMap["Pulse"] = &pulse;
    }

    void setup(EventManager* em) override {
        Process::setup(em);
        eventManager->subscribe(EVT_HTTP_RESPONSE_RECEIVED, this);
        ledManager->setBehavior(&ledsOff);
        vibrationManager->setBehavior(&motorOff);
    }

    void onEvent(Event& event) override {
        if (event.type == EVT_HTTP_RESPONSE_RECEIVED) {
            HttpResponseEvent& e = static_cast<HttpResponseEvent&>(event);
            handleServerResponse(e.response);
        }
    }

    void update() override {
        // This manager is reactive, so it does nothing in its update loop.
    }

private:
    void handleServerResponse(String& payload) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.printf("deserializeJson() failed: %s\\n", error.c_str());
            return;
        }

        if (doc.containsKey("led_behavior")) {
            JsonObject led_config = doc["led_behavior"];
            const char* type = led_config["type"];
            
            auto it = ledBehaviorMap.find(type);
            if (it != ledBehaviorMap.end()) {
                LedBehavior* behavior = it->second;
                if(led_config.containsKey("params")) {
                    JsonObject params = led_config["params"];
                    behavior->updateParams(params);
                }
                ledManager->setBehavior(behavior);
            }
        }

        if (doc.containsKey("vibration_behavior")) {
            JsonObject vib_config = doc["vibration_behavior"];
            const char* type = vib_config["type"];

            auto it = vibrationBehaviorMap.find(type);
            if (it != vibrationBehaviorMap.end()) {
                VibrationBehavior* behavior = it->second;
                if(vib_config.containsKey("params")) {
                    JsonObject params = vib_config["params"];
                    behavior->updateParams(params);
                }
                vibrationManager->setBehavior(behavior);
            }
        }
    }

    LedManager* ledManager;
    VibrationManager* vibrationManager;

    // --- Behavior Pools ---
    LedsOffBehavior ledsOff;
    SolidBehavior solid;
    BreathingBehavior breathing;
    HeartBeatBehavior heartBeat;
    CycleBehavior cycle;

    MotorOffBehavior motorOff;
    ConstantVibrationBehavior constant;
    BurstVibrationBehavior burst;
    PulseVibrationBehavior pulse;
    
    std::map<std::string, LedBehavior*> ledBehaviorMap;
    std::map<std::string, VibrationBehavior*> vibrationBehaviorMap;
};

#endif // BEHAVIOR_MANAGER_H 