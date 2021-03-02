#ifndef NEOPXL
#define NEOPXL

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "config.h"
#include "logger.h"

#define PIN D4
#define COUNT 16

#define CO2_COLOR_FADE_DELAY 20 // ms
#define LED_RING_OFFSET 4

class NeoPxl{
  private:
    void processFadeJob();
    void processSignalJob();
    int signalJobData[4];
    unsigned long fadeJobLastMillis;
    unsigned long signalJobLastMillis;
    bool signalPulseActive;

    uint32_t currentColor;
    bool fadeJobActive;
    uint32_t fadeJobColor;
    uint32_t fadeJobSpeed;

    Logger logger;
  public:

    int signalMaxPulses;
    int signalFadeSpeed;
    int signalDuration;

    bool isFadeActive();
    NeoPxl();
    void displayPpm(uint16_t ppm);
    void setColor(int r, int g, int b);
    void fadeToColor(int r, int g, int b, int speed);
    void apFadeSignal(int r, int g, int b);
    void calibrateSignal(int r, int g, int b, int duration);
    void connectSignal();
    void stopSignals();
    void update();
};
#endif
