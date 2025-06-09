#ifndef VIBRATION_MANAGER_H
#define VIBRATION_MANAGER_H

#include <Arduino.h>
#include "Process.h"
#include "Timer.h"
#include "config.h"

class VibrationManager : public Process {
public:
    VibrationManager() : Process(), updateTimer(1000 / 10) { // 10 Hz update rate
    }

    void setup() override {
        // Set the pin to output mode for analogWrite
        pinMode(VIBRATION_MOTOR_PIN, OUTPUT);
        updateTimer.reset();
    }

    void update() override {
        if (updateTimer.checkAndReset()) {
            // Create a pulsing effect using a sine wave
            // The pulse will have a period of 2 seconds
            float sine_wave = sin(millis() * 2.0 * PI / 2000.0);
            
            // Map the sine wave (-1 to 1) to the PWM duty cycle range (0 to 255)
            uint8_t dutyCycle = (uint8_t)((sine_wave + 1.0) / 2.0 * 255);
            
            analogWrite(VIBRATION_MOTOR_PIN, dutyCycle);
        }
    }

private:
    Timer updateTimer;
};

#endif // VIBRATION_MANAGER_H 