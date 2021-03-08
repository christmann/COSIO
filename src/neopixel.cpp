#include "neopixel.h"

Adafruit_NeoPixel strip(COUNT, PIN, NEO_GRB + NEO_KHZ800);
int signalPulse = 0;

NeoPxl::NeoPxl(){
  strip.begin();
  /* high brightness value may cause problems */
  strip.setBrightness(Config::instance()->ledIntensity);
  setColor(0, 0, 0);
  fadeJobLastMillis = millis();
  signalJobLastMillis = millis();
  ledRefreshTime = millis();

  fadeJobActive = false;
  signalPulseActive = false;
  
  signalMaxPulses = 10;
  signalFadeSpeed = 5;
  signalDuration = 10;
}

bool NeoPxl::isFadeActive(){
  return fadeJobActive;
}

void NeoPxl::setColor(int r, int g, int b){
  currentColor = strip.Color(r, g, b);
  for(int i = 0; i < strip.numPixels(); i++){
    strip.setPixelColor(i, currentColor);
  }
  fadeJobActive = false;
}

void NeoPxl::displayPpm(uint16_t ppm) {
  int ledsOn;

  if (signalPulseActive) return;

  int r = map(ppm, Config::instance()->ppmNormal, Config::instance()->ppmMax, 0, 255);
  if (r < 0) r = 0;
  if (r > 255) r = 255;
  int g = 255 - r;

  switch (Config::instance()->displayMode) {
    case 0:
    default:
      fadeToColor(r, g, 0, CO2_COLOR_FADE_DELAY);
      break;
    case 1:
      ledsOn = map(ppm, Config::instance()->ppmNormal, Config::instance()->ppmMax, 1, strip.numPixels());
      if (ledsOn < 0) ledsOn = 0;
      for (int i = 0; i < strip.numPixels(); i++) {
        uint8_t led = (i + LED_RING_OFFSET) % strip.numPixels();
        if (i < ledsOn) {
          strip.setPixelColor(led, strip.Color(r, g, 0));
        } else {
          strip.setPixelColor(led, strip.Color(0, 0, 0));
        }
      }
      break;
    case 2:
      ledsOn = map(ppm, Config::instance()->ppmNormal, Config::instance()->ppmMax, 1, strip.numPixels());
      if (ledsOn < 0) ledsOn = 0;
      for (int i = 0; i < strip.numPixels(); i++) {
        uint8_t led = (i + LED_RING_OFFSET) % strip.numPixels();
        r = map(i + 1, 1, strip.numPixels(), 0, 255);
        g = 255 - r;
        if (i < ledsOn) {
          strip.setPixelColor(led, strip.Color(r, g, 0));
        } else {
          strip.setPixelColor(led, strip.Color(0, 0, 0));
        }
      }
      break;
  }
}

void NeoPxl::fadeToColor(int r, int g, int b, int speed){
  fadeJobColor = strip.Color(r, g, b);
  fadeJobSpeed = speed;
  fadeJobActive = true;
}

void NeoPxl::calibrateSignal(int r, int g, int b, int duration){
  signalJobData[0] = r; signalJobData[1] = g; signalJobData[2] = b; 
  signalPulseActive = true;
  signalDuration = duration;
  signalMaxPulses = 5000;
  signalPulse = 0;
}

void NeoPxl::apFadeSignal(int r, int g, int b){
  signalJobData[0] = r; signalJobData[1] = g; signalJobData[2] = b; 
  signalPulseActive = true;
  signalDuration = 10;
  signalPulse = 0;
}

void NeoPxl::processSignalJob(){
  int r = signalJobData[0]; int g = signalJobData[1]; int b = signalJobData[2]; 
  if(signalPulse == 0 && signalPulseActive){
    fadeToColor(r, g, b, signalFadeSpeed);
    signalPulse++;
    signalPulseActive = true;
    return;
  }
  if(signalPulse % 2 == 0 && !fadeJobActive && signalPulseActive){
    fadeToColor(r, g, b, signalFadeSpeed);
    signalPulse++;
  }
  else if(signalPulse % 2 != 0 && !fadeJobActive && signalPulseActive){
    fadeToColor(0, 0, 0, signalFadeSpeed);
    signalPulse++;
  }
  if(signalPulseActive && (signalPulse >= signalMaxPulses || signalDuration * 1000 < millis())){
    fadeToColor(0, 0, 0, signalFadeSpeed);
    signalPulse = 0;
    signalPulseActive = false;
  }
  signalJobLastMillis = millis();
}

void NeoPxl::processFadeJob() {
  if (currentColor != fadeJobColor) {
    uint8_t curr_r, curr_g, curr_b;
    uint8_t fade_r, fade_g, fade_b;
    curr_b = currentColor & 0xFF; curr_g = (currentColor >> 8) & 0xFF; curr_r = (currentColor >> 16) & 0xFF;
    fade_b = fadeJobColor & 0xFF; fade_g = (fadeJobColor >> 8) & 0xFF; fade_r = (fadeJobColor >> 16) & 0xFF;

    if (curr_r < fade_r) curr_r++; else if (curr_r > fade_r) curr_r--;
    if (curr_g < fade_g) curr_g++; else if (curr_g > fade_g) curr_g--;
    if (curr_b < fade_b) curr_b++; else if (curr_b > fade_b) curr_b--;

    currentColor = strip.Color(curr_r, curr_g, curr_b);
    for(int i = 0; i < strip.numPixels(); i++){
      strip.setPixelColor(i, currentColor);
    }
  } else{
    fadeJobActive = false;
  }
}

void NeoPxl::update(){
  // update fade
  if(fadeJobActive && millis() - fadeJobLastMillis > fadeJobSpeed) {
    processFadeJob();
    fadeJobLastMillis = millis();
  }

  // update signals
  processSignalJob();

  if (millis() - ledRefreshTime > CO2_COLOR_FADE_DELAY) {
    strip.show();
    ledRefreshTime = millis();
  }
}
