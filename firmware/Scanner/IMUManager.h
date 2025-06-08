#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "Process.h"
#include "Timer.h"
#include "SparkFun_LIS2DH12.h"
#include <Wire.h>
#include <math.h>

// Conversion factor from mm/s^2 to g. 1g = 9806.65 mm/s^2
#define MMS2_TO_G 0.00010197

class IMUManager : public Process {
private:
    Timer readTimer;
    SPARKFUN_LIS2DH12 sensor;       //Create instance
    bool sensorOk = false;

    // Accumulators for the interval
    float sumAngleXZ = 0.0;
    float sumAngleYZ = 0.0;
    float totalMovement = 0.0;
    int readingCount = 0;

    // Final calculated values for the interval
    float averageAngleXZ = 0.0;
    float averageAngleYZ = 0.0;
    
public:
    IMUManager() : 
        readTimer(100) // Read every 100ms
    {}

    void setup() override {
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
            float x_mm = sensor.getX();
            float y_mm = sensor.getY();
            float z_mm = sensor.getZ();

            // Convert to g-force
            float x_g = x_mm * MMS2_TO_G;
            float y_g = y_mm * MMS2_TO_G;
            float z_g = z_mm * MMS2_TO_G;

            // Accumulate angle data
            sumAngleXZ += atan2(x_g, z_g) * 180.0 / PI;
            sumAngleYZ += atan2(y_g, z_g) * 180.0 / PI;

            // Accumulate total movement
            totalMovement += sqrt(pow(x_g, 2) + pow(y_g, 2) + pow(z_g, 2))-1.0f;
            
            readingCount++;
        }
    }

    void prepareForNextInterval() {
        if (readingCount > 0) {
            averageAngleXZ = sumAngleXZ / readingCount;
            averageAngleYZ = sumAngleYZ / readingCount;
        } else {
            averageAngleXZ = 0;
            averageAngleYZ = 0;
        }
        
        // Reset accumulators for the next interval
        sumAngleXZ = 0.0;
        sumAngleYZ = 0.0;
        totalMovement = 0.0; // Total movement is also reset after being read
        readingCount = 0;
    }

    float getAverageAngleXZ() const { return averageAngleXZ; }
    float getAverageAngleYZ() const { return averageAngleYZ; }
    float getTotalMovement() const { return totalMovement; }
};

#endif // IMU_MANAGER_H 