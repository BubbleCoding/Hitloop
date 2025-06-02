#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#include "SparkFun_LIS2DH12.h"  
SPARKFUN_LIS2DH12 accel;        

#define LED_PIN D1
#define MOTOR_PIN D0

#define LED_COUNT 6
#define MOTOR_INTERVAL 2500

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t motorState = LOW;

void setup() {
  strip.begin();            
  strip.show();             
  strip.setBrightness(50);  

  Serial.begin(115200);
  Serial.println("Hitloop Beacon Controller Tester");

  Wire.begin();

  if (accel.begin() == false) {
    Serial.println("Accelerometer not detected. Check address jumper and wiring. Freezing...");
    while (1)
      ;
  }
}

long firstPixelHue = 0;

void loop() {

  // Print accel values only if new data is available
  if (accel.available()) {
    float accelX = accel.getX();
    float accelY = accel.getY();
    float accelZ = accel.getZ();
    float tempC = accel.getTemperature();

    Serial.print(accelX, 1);
    Serial.print("\t");
    Serial.print(accelY, 1);
    Serial.print("\t");
    Serial.print(accelZ, 1);
    Serial.print("\t");
    Serial.print(tempC, 1);
    Serial.println();

    firstPixelHue = firstPixelHue > 5 * 65536 ? 0 : firstPixelHue + 256;

    strip.rainbow(firstPixelHue);
    strip.show();  
  }

  if (millis() % MOTOR_INTERVAL == 0){
    motorState = !motorState;
    digitalWrite(MOTOR_PIN, motorState);
  }

}
