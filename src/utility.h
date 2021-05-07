#ifndef UTILITY
#define UTILITY

#include <Arduino.h>  
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

class Utility {
	private:
		Utility();
    Utility(const Utility&);
    static Utility* _instance;

	public:
		static Utility* instance() {
			if (!_instance)
					_instance = new Utility();
			return _instance;
		}

    String encryptionTypeStr(uint8_t authmode);
};

#endif
