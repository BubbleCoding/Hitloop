#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include "Process.h"
#include "Timer.h"
#include "config.h"

class LedManager : public Process {
public:
    LedManager() : Process(), pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800), updateTimer(1000 / 30) {
    }

    void setup() override {
        pixels.begin();
        pixels.setBrightness(50); // Don't set too high to avoid high current draw
        colorWipe(pixels.Color(0, 0, 0)); // Clear all pixels
        updateTimer.reset();
    }

    void update() override {
        if (updateTimer.checkAndReset()) {
            // Simple animation: chase a red pixel
            int currentPixel = (millis() / 200) % LED_COUNT;
            
            // Turn off all pixels
            pixels.clear(); 
            
            // Set the color of the current pixel
            pixels.setPixelColor(currentPixel, pixels.Color(255, 0, 0)); 
            
            pixels.show(); // This sends the updated pixel color to the hardware.
        }
    }

private:
    void colorWipe(uint32_t color) {
        for (unsigned int i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, color);
        }
        pixels.show();
    }

    Adafruit_NeoPixel pixels;
    Timer updateTimer;
};

#endif // LED_MANAGER_H 