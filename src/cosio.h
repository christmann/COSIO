#ifndef COSIO_
#define COSIO_

#include <Wire.h>  
#include <Arduino.h>  
#include <Adafruit_NeoPixel.h>
#include "logger.h"
#include "config.h"
#include "neopixel.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include "oled.h"
#include "utility.h"
#include "sensor.h"

#define SENSOR Sensor::instance()
#define CONFIG Config::instance()

//D2 (SDA/Serial Data), D1 (SCL/Serial Clock)

class COSIO {
	private:
		COSIO();
    COSIO(const COSIO&);
    static COSIO* _instance;

		SSD1306 display;
		Logger logger;
		ESP8266WebServer server;
		WiFiClient espClient;
		PubSubClient mqttClient;
		NeoPxl* pixel;
		Oled* oled;

		unsigned long lastMqttPublish;
		unsigned long lastMqttReconnectAttempt;
		bool isApMode;
		bool wifiDisabled;

		// web server methods
		void getSignalStrength();
		void scanForNetworks();
		void serveConfigParameter();
		void modifyConfigParameter();
		void setCalibrationFlag();
		void sendSensorData();
		void initServer();
		void wifiConnect(bool displayStatus);
		boolean mqttReconnect();

	public:
		static COSIO* instance() {
			if (!_instance)
					_instance = new COSIO();
			return _instance;
		}

		void startManualCalibration();
		void setup();
		void loop();
};

#endif
