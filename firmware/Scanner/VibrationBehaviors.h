#ifndef VIBRATION_BEHAVIORS_H
#define VIBRATION_BEHAVIORS_H

#include "Arduino.h"
#include "Timer.h"
#include "config.h"

// --- Vibration Behavior Base Class ---
class VibrationBehavior {
public:
    virtual ~VibrationBehavior() {}
    virtual void setup() {
        pinMode(VIBRATION_MOTOR_PIN, OUTPUT);
    }
    virtual void update() = 0;
};

// --- Concrete Vibration Behaviors ---

// 1. MotorOffBehavior
class MotorOffBehavior : public VibrationBehavior {
public:
    void setup() override {
        VibrationBehavior::setup();
        analogWrite(VIBRATION_MOTOR_PIN, 0);
    }
    void update() override {
        // Do nothing, motor is off
    }
};

// 2. ConstantVibrationBehavior
class ConstantVibrationBehavior : public VibrationBehavior {
public:
    ConstantVibrationBehavior(uint8_t intensity) : intensity(intensity) {}
    
    void setup() override {
        VibrationBehavior::setup();
        analogWrite(VIBRATION_MOTOR_PIN, intensity);
    }
    
    void update() override {
        // Do nothing, motor is at constant vibration
    }
private:
    uint8_t intensity;
};

// 3. BurstVibrationBehavior
class BurstVibrationBehavior : public VibrationBehavior {
public:
    BurstVibrationBehavior(uint8_t intensity, unsigned long frequency) 
        : intensity(intensity), burstTimer(1000 / frequency), motorOn(false) {}

    void setup() override {
        VibrationBehavior::setup();
        burstTimer.reset();
        analogWrite(VIBRATION_MOTOR_PIN, 0); // Start in off state
    }
    
    void update() override {
        if (burstTimer.checkAndReset()) {
            motorOn = !motorOn;
            analogWrite(VIBRATION_MOTOR_PIN, motorOn ? intensity : 0);
        }
    }
private:
    uint8_t intensity;
    Timer burstTimer;
    bool motorOn;
};

#endif // VIBRATION_BEHAVIORS_H 