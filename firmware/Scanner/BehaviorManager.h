#ifndef BEHAVIOR_MANAGER_H
#define BEHAVIOR_MANAGER_H

#include <ArduinoJson.h>
#include "Process.h"
#include "LedManager.h"
#include "VibrationManager.h"
#include "EventManager.h"

class BehaviorManager : public Process {
public:
    BehaviorManager(LedManager* led, VibrationManager* vib) 
        : ledManager(led), vibrationManager(vib) {}

    void setup(EventManager* em) override {
        Process::setup(em);
        eventManager->subscribe(EVT_HTTP_RESPONSE_RECEIVED, this);
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
            Serial.printf("deserializeJson() failed: %s\n", error.c_str());
            return;
        }

        // --- Parse and Set LED Behavior ---
        if (doc.containsKey("led_behavior")) {
            JsonObject led_config = doc["led_behavior"];
            const char* type = led_config["type"];
            bool createNew = true;

            if (ledManager->currentBehavior && strcmp(ledManager->currentBehavior->type, type) == 0) {
                createNew = false;
                if (strcmp(type, "Breathing") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    if (static_cast<BreathingBehavior*>(ledManager->currentBehavior)->color != newColor) createNew = true;
                } else if (strcmp(type, "HeartBeat") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    unsigned long newPeriod = led_config["params"]["period"].as<unsigned long>();
                    auto* current = static_cast<HeartBeatBehavior*>(ledManager->currentBehavior);
                    if (current->color != newColor || current->period != newPeriod) createNew = true;
                } else if (strcmp(type, "Cycle") == 0) {
                    uint32_t newColor = hexToColor(led_config["params"]["color"].as<String>());
                    int newDelay = led_config["params"]["delay"].as<int>();
                    auto* current = static_cast<CycleBehavior*>(ledManager->currentBehavior);
                    if (current->color != newColor || current->delay != newDelay) createNew = true;
                }
            }

            if (createNew) {
                Serial.printf("Setting new LED behavior: %s\n", type);
                if (strcmp(type, "Off") == 0) ledManager->setBehavior(new LedsOffBehavior());
                else if (strcmp(type, "Breathing") == 0) ledManager->setBehavior(new BreathingBehavior(hexToColor(led_config["params"]["color"].as<String>())));
                else if (strcmp(type, "HeartBeat") == 0) ledManager->setBehavior(new HeartBeatBehavior(hexToColor(led_config["params"]["color"].as<String>()), led_config["params"]["period"].as<unsigned long>()));
                else if (strcmp(type, "Cycle") == 0) ledManager->setBehavior(new CycleBehavior(hexToColor(led_config["params"]["color"].as<String>()), led_config["params"]["delay"].as<int>()));
            }
        }

        // --- Parse and Set Vibration Behavior ---
        if (doc.containsKey("vibration_behavior")) {
            JsonObject vib_config = doc["vibration_behavior"];
            const char* type = vib_config["type"];
            bool createNew = true;

            if (vibrationManager->currentBehavior && strcmp(vibrationManager->currentBehavior->type, type) == 0) {
                createNew = false;
                if (strcmp(type, "Constant") == 0) {
                    uint8_t newIntensity = vib_config["params"]["intensity"].as<uint8_t>();
                    if (static_cast<ConstantVibrationBehavior*>(vibrationManager->currentBehavior)->intensity != newIntensity) createNew = true;
                } else if (strcmp(type, "Burst") == 0) {
                    uint8_t newIntensity = vib_config["params"]["intensity"].as<uint8_t>();
                    unsigned long newFreq = vib_config["params"]["frequency"].as<unsigned long>();
                    auto* current = static_cast<BurstVibrationBehavior*>(vibrationManager->currentBehavior);
                    if (current->intensity != newIntensity || current->frequency != newFreq) createNew = true;
                }
            }

            if (createNew) {
                Serial.printf("Setting new vibration behavior: %s\n", type);
                if (strcmp(type, "Off") == 0) vibrationManager->setBehavior(new MotorOffBehavior());
                else if (strcmp(type, "Constant") == 0) vibrationManager->setBehavior(new ConstantVibrationBehavior(vib_config["params"]["intensity"].as<uint8_t>()));
                else if (strcmp(type, "Burst") == 0) vibrationManager->setBehavior(new BurstVibrationBehavior(vib_config["params"]["intensity"].as<uint8_t>(), vib_config["params"]["frequency"].as<unsigned long>()));
            }
        }
    }

    LedManager* ledManager;
    VibrationManager* vibrationManager;

    uint32_t hexToColor(String hex) {
        if (hex.startsWith("#")) {
            hex = hex.substring(1);
        }
        return (uint32_t)strtol(("0x" + hex).c_str(), NULL, 16);
    }
};

#endif // BEHAVIOR_MANAGER_H 