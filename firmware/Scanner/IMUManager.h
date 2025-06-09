#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "Process.h"
#include "Timer.h"
#include "SparkFun_LIS2DH12.h"
#include <Wire.h>
#include <math.h>

// Conversion factor from cm/s^2 to g. 1g = 980.665 cm/s^2
#define CMS2_TO_G 0.0010197
#define MOVING_AVG_WINDOW_SIZE 10

class IMUManager : public Process {
private:
    Timer readTimer;
    SPARKFUN_LIS2DH12 sensor;       //Create instance
    bool sensorOk = false;

    // --- Moving Average Filter State ---
    float angleXZHistory[MOVING_AVG_WINDOW_SIZE] = {0.0f};
    float angleYZHistory[MOVING_AVG_WINDOW_SIZE] = {0.0f};
    float sumAngleXZ = 0.0;
    float sumAngleYZ = 0.0;
    int historyIndex = 0;
    int readingsInHistory = 0;

    // --- Interval Accumulator State ---
    float totalMovementInInterval = 0.0;
    float lastIntervalTotalMovement = 0.0;

    // --- Current Calculated Values ---
    float movingAverageAngleXZ = 0.0;
    float movingAverageAngleYZ = 0.0;
    
public:
    IMUManager() : 
        readTimer(100) // Read every 100ms
    {}

    void setup(EventManager* em) override {
        Process::setup(em);
        // The LIS2DH12 library uses Wire, so it should be initialized.
        // It's often safe to call Wire.begin() multiple times.
        Wire.begin(); 
        
        // The begin function returns a status, 0 on success
        if (sensor.begin() != 0) {
            Serial.println("IMU sensor initialized successfully.");
            sensorOk = true;
        } else {            
            Serial.println("Could not initialize IMU sensor.");
        }
    }

    void update() override {
        if (sensorOk && readTimer.checkAndReset() && sensor.available()) {
            // --- 1. Read and Convert Data ---
            float x_g = sensor.getX() * CMS2_TO_G;
            float y_g = sensor.getY() * CMS2_TO_G;
            float z_g = sensor.getZ() * CMS2_TO_G;

            // --- 2. Update Moving Average Filters for Angles ---
            float currentAngleXZ = atan2(x_g, z_g)  / PI * 180;
            float currentAngleYZ = atan2(y_g, z_g)  / PI * 180;

            // Subtract the oldest value from the sum and add the new one
            sumAngleXZ = sumAngleXZ - angleXZHistory[historyIndex] + currentAngleXZ;
            sumAngleYZ = sumAngleYZ - angleYZHistory[historyIndex] + currentAngleYZ;

            // Store the new value in the history buffer
            angleXZHistory[historyIndex] = currentAngleXZ;
            angleYZHistory[historyIndex] = currentAngleYZ;
            
            // Increment the index for the next reading
            historyIndex = (historyIndex + 1) % MOVING_AVG_WINDOW_SIZE;

            // Keep track of how many readings we have for the initial average calculation
            if (readingsInHistory < MOVING_AVG_WINDOW_SIZE) {
                readingsInHistory++;
            }
            
            // Calculate the new moving average
            movingAverageAngleXZ = sumAngleXZ / readingsInHistory;
            movingAverageAngleYZ = sumAngleYZ / readingsInHistory;

            // --- 3. Accumulate total movement for the current interval ---
            totalMovementInInterval += sqrt(x_g*x_g + y_g*y_g + z_g*z_g);
        }
    }

    // Called by BleManager before a new scan interval begins
    void prepareForNextInterval() {
        // Latch the total movement from the completed interval
        lastIntervalTotalMovement = totalMovementInInterval;
        
        // Reset the accumulator for the next interval
        totalMovementInInterval = 0.0;
    }

    float getAverageAngleXZ() const { return movingAverageAngleXZ; }
    float getAverageAngleYZ() const { return movingAverageAngleYZ; }
    float getTotalMovement() const { return lastIntervalTotalMovement; }
};

#endif // IMU_MANAGER_H 