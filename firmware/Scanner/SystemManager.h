#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "Arduino.h"
#include "Process.h"
#include "Timer.h"
#include "Configuration.h"
#include "config.h"
#include "EventManager.h"
#include "SharedState.h"

class SystemManager : public Process {
private:
    Config& cfg;
    Timer debounceTimer;
    int lastButtonState;
    int currentButtonState;
    Timer configCheckTimer;
    SharedState& sharedState;

public:
    SystemManager(Config& config, SharedState& state) 
        : cfg(config), 
        debounceTimer(50), // 50ms debounce delay
        lastButtonState(HIGH),
        currentButtonState(HIGH),
        configCheckTimer(500),
        sharedState(state)
    {}

    void setup(EventManager* em) override {
        Process::setup(em);
        pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

        // Populate the initial shared state
        sharedState.macAddress = WiFi.macAddress();
        Serial.println("SystemManager Initialized. MAC: " + sharedState.macAddress);
    }

    void update() override {
        int reading = digitalRead(BOOT_BUTTON_PIN);

        // Reset the debounce timer if the state has changed
        if (reading != lastButtonState) {
            debounceTimer.reset();
        }

        if (debounceTimer.hasElapsed()) {
            // The reading is stable, so we can update the current state
            if (reading != currentButtonState) {
                currentButtonState = reading;

                // If the new stable state is pressed (LOW)
                if (currentButtonState == LOW) {
                    Serial.println("Entering config mode due to button press...");
                    cfg.enterSerialConfig(); // This will block and restart
                }
            }
        }
        
        lastButtonState = reading;
    }
};

#endif // SYSTEM_MANAGER_H 