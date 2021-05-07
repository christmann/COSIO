#ifndef CALIBRATE
#define CALIBRATE
// Possible values for CO2_SENSOR_TYPE:
//  1: SCD30
//  2: MH-Z19B / MH-Z19C
//  3: SenseAir S8
#define CO2_SENSOR_TYPE	3
// calibration time in minutes
#define CALIBRATE_TIME 20

// lowest measurement
#define LOWER_LIMIT 400

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
#elif CO2_SENSOR_TYPE == 3
	#include <SoftwareSerial.h>
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
    unsigned long lastSensorUpdate;

    #if CO2_SENSOR_TYPE == 1
			SCD30 sensor;
		#elif CO2_SENSOR_TYPE == 2
			//D7 (To sensor Pin 6, sensor TXD), D8 (To sensor Pin 5, sensor RXD)
			SoftwareSerial sensorSerial;
			MHZ19 sensor;
    #elif CO2_SENSOR_TYPE == 3
      //D7 (To sensor Pin 6, sensor TXD), D8 (To sensor Pin 5, sensor RXD)
			SoftwareSerial sensorSerial;
      bool sendSenseairCommand(const byte command[], bool validIfEchoed);
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
