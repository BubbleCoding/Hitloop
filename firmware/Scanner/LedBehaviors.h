#ifndef LED_BEHAVIORS_H
#define LED_BEHAVIORS_H

#include <Adafruit_NeoPixel.h>
#include "Timer.h"

// --- LED Behavior Base Class ---
class LedBehavior {
public:
    const char* type;
    virtual ~LedBehavior() {}
    virtual void setup(Adafruit_NeoPixel& pixels) {
        this->pixels = &pixels;
    }
    virtual void update() = 0;

protected:
    LedBehavior(const char* type) : type(type) {}
    Adafruit_NeoPixel* pixels;
    uint32_t scaleColor(uint32_t color, uint8_t brightness) {
        uint8_t r = (uint8_t)((color >> 16) * brightness / 255);
        uint8_t g = (uint8_t)((color >> 8) * brightness / 255);
        uint8_t b = (uint8_t)(color * brightness / 255);
        return pixels->Color(r, g, b);
    }
};

// --- Concrete LED Behaviors ---

// 1. LedsOffBehavior
class LedsOffBehavior : public LedBehavior {
public:
    LedsOffBehavior() : LedBehavior("Off") {}
    void setup(Adafruit_NeoPixel& pixels) override {
        LedBehavior::setup(pixels);
        this->pixels->clear();
        this->pixels->show();
    }
    void update() override {
        // Do nothing, LEDs are off
    }
};

// 2. BreathingBehavior
class BreathingBehavior : public LedBehavior {
public:
    uint32_t color;
    BreathingBehavior(uint32_t color) : LedBehavior("Breathing"), color(color), updateTimer(1000 / 50) {} // 50Hz for smooth animation

    void setup(Adafruit_NeoPixel& pixels) override {
        LedBehavior::setup(pixels);
        updateTimer.reset();
    }

    void update() override {
        if (updateTimer.checkAndReset()) {
            float sine_wave = sin(millis() * 2.0 * PI / 4000.0); // 4-second period
            uint8_t brightness = (uint8_t)(((sine_wave + 1.0) / 2.0) * 255.0);
            pixels->fill(scaleColor(color, brightness));
            pixels->show();
        }
    }

private:
    Timer updateTimer;
};

// 3. HeartBeatBehavior
class HeartBeatBehavior : public LedBehavior {
public:
    uint32_t color;
    unsigned long period;
    HeartBeatBehavior(uint32_t color, unsigned long period) 
        : LedBehavior("HeartBeat"), color(color), period(period), periodTimer(period), beatTimer(50), state(IDLE) {}

    void setup(Adafruit_NeoPixel& pixels) override {
        LedBehavior::setup(pixels);
        periodTimer.reset();
    }

    void update() override {
        switch (state) {
            case IDLE:
                if (periodTimer.checkAndReset()) {
                    state = FADE_IN_1;
                    beatTimer.interval = 60; // Fast fade in
                    beatTimer.reset();
                }
                break;
            case FADE_IN_1: {
                unsigned long elapsed = millis() - beatTimer.last_update;
                if (elapsed >= beatTimer.interval) {
                    pixels->fill(color);
                    pixels->show();
                    state = FADE_OUT_1;
                    beatTimer.interval = 150;
                    beatTimer.reset();
                } else {
                    uint8_t brightness = (elapsed * 255) / beatTimer.interval;
                    pixels->fill(scaleColor(color, brightness));
                    pixels->show();
                }
                break;
            }
            case FADE_OUT_1: {
                 unsigned long elapsed = millis() - beatTimer.last_update;
                if (elapsed >= beatTimer.interval) {
                    pixels->clear();
                    pixels->show();
                    state = PAUSE;
                    beatTimer.interval = 100; // Pause for 100ms
                    beatTimer.reset();
                } else {
                    uint8_t brightness = 255 - (elapsed * 255 / beatTimer.interval);
                    pixels->fill(scaleColor(color, brightness));
                    pixels->show();
                }
                break;
            }
            case PAUSE:
                if (beatTimer.checkAndReset()) {
                    state = FADE_IN_2;
                    beatTimer.interval = 60;
                    beatTimer.reset();
                }
                break;
            case FADE_IN_2: {
                 unsigned long elapsed = millis() - beatTimer.last_update;
                if (elapsed >= beatTimer.interval) {
                    pixels->fill(color);
                    pixels->show();
                    state = FADE_OUT_2;
                    beatTimer.interval = 400; // Slower fade out
                    beatTimer.reset();
                } else {
                    uint8_t brightness = (elapsed * 255) / beatTimer.interval;
                    pixels->fill(scaleColor(color, brightness));
                    pixels->show();
                }
                break;
            }
            case FADE_OUT_2: {
                unsigned long elapsed = millis() - beatTimer.last_update;
                if (elapsed >= beatTimer.interval) {
                    pixels->clear();
                    pixels->show();
                    state = IDLE;
                } else {
                    uint8_t brightness = 255 - (elapsed * 255 / beatTimer.interval);
                    pixels->fill(scaleColor(color, brightness));
                    pixels->show();
                }
                break;
            }
        }
    }

private:
    enum BeatState { IDLE, FADE_IN_1, FADE_OUT_1, PAUSE, FADE_IN_2, FADE_OUT_2 };
    BeatState state;
    Timer periodTimer; // Time between heartbeats
    Timer beatTimer;   // Time for individual fades/pauses
};


// 4. CycleBehavior
class CycleBehavior : public LedBehavior {
public:
    uint32_t color;
    int delay;
    CycleBehavior(uint32_t color, int delay) : LedBehavior("Cycle"), color(color), delay(delay), updateTimer(delay) {}

    void setup(Adafruit_NeoPixel& pixels) override {
        LedBehavior::setup(pixels);
        updateTimer.reset();
        currentPixel = 0;
    }

    void update() override {
        if (updateTimer.checkAndReset()) {
            pixels->clear();
            pixels->setPixelColor(currentPixel, color);
            pixels->show();
            currentPixel = (currentPixel + 1) % pixels->numPixels();
        }
    }

private:
    Timer updateTimer;
    int currentPixel;
};

#endif // LED_BEHAVIORS_H 