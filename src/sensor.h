#ifndef CALIBRATE
#define CALIBRATE
// Possible values for CO2_SENSOR_TYPE:
//  1: SCD30
//  2: MH-Z19B / MH-Z19C
#define CO2_SENSOR_TYPE	2
// calibration time in minutes
#define CALIBRATE_TIME 20

#include <Arduino.h>
#include "logger.h"
#include "config.h"
#include "oled.h"
#include "neopixel.h"
#if CO2_SENSOR_TYPE == 1
	#include <SparkFun_SCD30_Arduino_Library.h>
#elif CO2_SENSOR_TYPE == 2
	#include <SoftwareSerial.h>
	#include <MHZ19.h>
#else
	#error "Unknown sensor type defined (CO2_SENSOR_TYPE)"
#endif

#define CONFIG Config::instance()

class Sensor {
  private:
    Sensor();
    Sensor(const Sensor&);

    static Sensor* _instance;
    Logger logger;

    bool isCali;

    #if CO2_SENSOR_TYPE == 1
			SCD30 sensor;
		#elif CO2_SENSOR_TYPE == 2
			//D7 (To sensor Pin 6, sensor TXD), D8 (To sensor Pin 5, sensor RXD)
			SoftwareSerial sensorSerial;
			MHZ19 sensor;
			unsigned long lastSensorUpdate;
		#endif

    void startManualCalibration();

  public:
    static Sensor* instance() {
        if (!_instance)
            _instance = new Sensor();
        return _instance;
    }

    void init(Oled* oled);
    void calibrate();
    bool update();

    int ppm;
    int temperature;
    int humidity;

    bool calibratedOnPwr;
    bool isCalibrating;

};
#endif
