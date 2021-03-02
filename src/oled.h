#ifndef OLED
#define OLED

#include <Arduino.h>
#include "logger.h"
#include "SSD1306.h"
#include "config.h"
#include "font.h"

#define AP_MSG_TIME 		10000
#define CONNECTED_MSG_TIME	5000

#define WIFI_HEIGHT   8
#define WIFI_WIDTH    8

static const unsigned char PROGMEM wifi_bmp[] =
{ B00001000,
  B00100100,
  B00010010,
  B11001010,
  B11001010,
  B00010010,
  B00100100,
  B00001000 };

class Oled {
  private:
    SSD1306* _display;
    Logger logger;
    bool _messageOnScreen;
    unsigned long _messageTimeout;

  public:
    Oled(SSD1306* display);
    void showSensorData(String ppm, String temp, String humi);
    void showStatus(boolean wifiConnected, boolean apMode, boolean mqtt);
    void update();
    void connectMessage();
    void connectedMessage();
    void accessPointMessage();
    void showCalibrateMessage(int calibrateTime);
    void hwError(String error);
};
#endif
