#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "Arduino.h"
#include "Process.h"
#include "Timer.h"
#include "Configuration.h"
#include "config.h"

class SystemManager : public Process {
private:
    Timer configModeTimer;
    bool configModeEntered = false;
    Config& cfg;

public:
    SystemManager(Config& config) : configModeTimer(2000), cfg(config) {}

    void setup() override {
        configModeTimer.reset();
        pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
        Serial.println("Hold BOOT button (PIN 9) now to enter config mode.");
    }

    void update() override {
        if (!configModeTimer.hasElapsed() && !configModeEntered) {
            if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
                configModeEntered = true;
                cfg.enterSerialConfig();
            }
        }
    }
};

#endif // SYSTEM_MANAGER_H 