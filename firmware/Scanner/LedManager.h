#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include <Ticker.h>
#include "Process.h"
#include "config.h"
#include "LedBehaviors.h"

class LedManager : public Process {
public:
    LedManager() : Process(), pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800), currentBehavior(nullptr) {
    }

    ~LedManager() {
        ledTicker.detach();
    }

    void setBehavior(LedBehavior* newBehavior) {
        currentBehavior = newBehavior;
        if (currentBehavior) {
            currentBehavior->setup(pixels);
        }
    }

    void setup(EventManager* em) override {
        Process::setup(em);
        pixels.begin();
        pixels.setBrightness(127); // Don't set too high to avoid high current draw
        // Use a lambda to call the member function, passing 'this'
        ledTicker.attach_ms(20, +[](LedManager* instance) { instance->update(); }, this);
    }

    void update() override {
        if (currentBehavior) {
            currentBehavior->update();
        }
    }

    // Public members for access by BleManager
    Adafruit_NeoPixel pixels;
    LedBehavior* currentBehavior;

private:
    Ticker ledTicker;
};

#endif // LED_MANAGER_H 