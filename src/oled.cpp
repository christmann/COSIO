#include "oled.h"

#include <ESP8266WiFi.h>

Oled::Oled(SSD1306* display) : _display(display) {
  _display->init();
  _display->flipScreenVertically();
  _display->clear();
  _messageOnScreen = false;
  _messageTimeout = 0;
}

void Oled::update(){
  if(_messageOnScreen && millis() >= _messageTimeout) {
    _display->clear();
    _display->display();
    _messageOnScreen = false;
  }
}

void Oled::showSensorData(String ppm, String temp, String humi){

  if (_messageOnScreen) {
    return;
  }

  _display->clear();

  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  if (temp != "0") {
    _display->drawString(30, 0, temp + Config::instance()->tempUnit);
  }
  if (humi != "0") {
    _display->drawString(100, 0, humi + Config::instance()->humidUnit);
  }

  _display->setFont((uint8_t *) Roboto_Mono_32);
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 13, ppm);
  
  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 52, Config::instance()->co2Unit);

  _display->display();
}

void Oled::showStatus(boolean wifiConnected, boolean apMode, boolean mqtt) {
  if (_messageOnScreen) {
    return;
  }

  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_LEFT);
  if (apMode) {
    _display->drawString(2, 52, apMode ? "AP" : "  ");
  } else if (wifiConnected) {
    _display->drawFastImage(2, 52, WIFI_WIDTH, WIFI_HEIGHT, wifi_bmp);
  }

  if (mqtt) {
    _display->drawString(118, 52, "M");
  }

  _display->display();
}

void Oled::showCalibrateMessage(int calibrateTime){
  _display->clear();
  _display->setFont((uint8_t *) Roboto_Mono_24);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 10, "CALIBRATE");
  
  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 40, "Place outside");
  _display->drawString(64, 52, "for " + (String) calibrateTime + " minutes");
  _display->display();
  _messageTimeout = millis() + calibrateTime * 60000;
  _messageOnScreen = true;
}

void Oled::accessPointMessage(){
  _display->clear();
  _display->setFont((uint8_t *) Roboto_Mono_24);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 2, "AP MODE");
  
  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 28, "SSID: " + Config::instance()->apSsid);
  _display->drawString(64, 40, "PW: " + Config::instance()->apPassword);
  _display->drawString(64, 52, "http://" + WiFi.softAPIP().toString());
  _display->display();
  _messageTimeout = millis() + AP_MSG_TIME;
  _messageOnScreen = true;
}

void Oled::connectMessage(){
  _display->clear();
  _display->setFont((uint8_t *) Roboto_Mono_24);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 10, "CONNECT");
  
  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 40, Config::instance()->ssid);
  _display->display();
}

void Oled::connectedMessage(){
  _display->clear();
  _display->setFont((uint8_t *) Roboto_Mono_24);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 10, "CONNECTED");
  
  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 40, "http://" + WiFi.localIP().toString());
  _display->display();
  _messageTimeout = millis() + CONNECTED_MSG_TIME;
  _messageOnScreen = true;
}

void Oled::hwError(String error) {
  _display->clear();
  _display->setFont((uint8_t *) Roboto_Mono_24);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 10, "HW ERROR");

  _display->setFont((uint8_t *) Roboto_Mono_10);  
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->drawString(64, 40, error);
  _display->display();
}
