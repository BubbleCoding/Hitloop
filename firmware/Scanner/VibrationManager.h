#ifndef VIBRATION_MANAGER_H
#define VIBRATION_MANAGER_H

#include <Arduino.h>
#include "Process.h"
#include "config.h"
#include "VibrationBehaviors.h"

class VibrationManager : public Process {
public:
    VibrationManager() : Process(), currentBehavior(nullptr) {
    }

    ~VibrationManager() {
        delete currentBehavior;
    }

    void setBehavior(VibrationBehavior* newBehavior) {
        if (currentBehavior) {
            delete currentBehavior;
        }
        currentBehavior = newBehavior;
        if (currentBehavior) {
            currentBehavior->setup();
        }
    }

    void setup(EventManager* em) override {
        Process::setup(em);
        setBehavior(new MotorOffBehavior());
    }

    void update() override {
        if (currentBehavior) {
            currentBehavior->update();
        }
    }

    VibrationBehavior* currentBehavior;

private:
    // No private members
};

#endif // VIBRATION_MANAGER_H 