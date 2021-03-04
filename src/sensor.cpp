#include "sensor.h"

Sensor* Sensor::_instance = NULL;

Sensor::Sensor() : 
  sensorSerial(D7, D8){
    lastSensorUpdate = 0;
    isCalibrating = false;
    calibratedOnPwr = false;
    ppm = 0;
    temperature = 0;
    humidity = 0;
}

unsigned long calibrationInit = 0;

void Sensor::calibrate(){
  calibrationInit = millis();
  isCalibrating = true;
}

void Sensor::init(Oled* oled){
  #if CO2_SENSOR_TYPE == 1
    // initialize scd30 sensor
    logger.printlog(logger.INFO, "Initializing SCD30 CO2 sensor");
    Wire.begin();

    if (sensor.begin() == false)
    {
      oled->hwError("Sensor error");
      logger.printlog(logger.ERROR, "SCD30 CO2 Sensor not detected on I2C bus! Halting");
      while (1)
        delay(100);
        ;
    }

    // set inbuild auto-calibration
    sensor.setAutoSelfCalibration(CONFIG->useInbuildCalibration);

  #elif CO2_SENSOR_TYPE == 2
    // initialize MH-Z19C sensor
    logger.printlog(logger.INFO, "Initializing MH-Z19CB CO2 sensor");
    
    sensorSerial.begin(9600);
    delay(1000); // Sensor takes a while to be ready
    for (uint8_t i = 0; i < 10; ++i) {
      sensor.begin(sensorSerial);

      if (sensor.errorCode == RESULT_OK) {
        break;
      }
      logger.printlog(logger.ERROR, "MH-Z19C CO2 Sensor communication problem (" + (String)sensor.errorCode + ")! Retrying");
      delay(500);
    }

    if (sensor.errorCode != RESULT_OK) {
      // If all fails, try a sensor reset
      oled->hwError("Sensor resetting...");
      logger.printlog(logger.ERROR, "MH-Z19C CO2 Sensor communication problem (" + (String)sensor.errorCode + ")! Trying sensor reset");
      sensor.recoveryReset();
      delay(30000);
      for (uint8_t i = 0; i < 5; ++i) {
        sensor.verify();

        if (sensor.errorCode == RESULT_OK) {
          break;
        }
      }
    }
    
    if (sensor.errorCode != RESULT_OK) {
      oled->hwError("Sensor error");
      logger.printlog(logger.ERROR, "MH-Z19C CO2 Sensor communication problem (" + (String)sensor.errorCode + ")! Halting");
      while (1) {
        delay(100);
      }
    }
    // set inbuild auto-calibration
    sensor.autoCalibration(CONFIG->useInbuildCalibration, 24);

    logger.printlog(logger.INFO, "MH-Z19C CO2 Sensor initialized!");
  #endif
}

void Sensor::startManualCalibration(){
  #if CO2_SENSOR_TYPE == 1
    sensor.setForcedRecalibrationFactor(400);
  #elif CO2_SENSOR_TYPE == 2
    sensor.calibrate();
  #endif
  logger.printlog(logger.INFO, "Calibration was performeds");
}


bool Sensor::update(){
  /* start calibrating if enabled */
  if(isCalibrating && ((calibrationInit + CALIBRATE_TIME * 60000) < millis())){
    startManualCalibration();
    isCalibrating = false;
  }
  /* update sensor values */
  #if CO2_SENSOR_TYPE == 1
    if (sensor.dataAvailable()) {
      ppm = sensor.getCO2();
      if (Config::instance()->showTemperature) {
        temperature = sensor.getTemperature();
      } else {
        temperature = 0;
      }
      humidity = sensor.getHumidity();
      lastSensorUpdate = millis();
      return true;
    }
    return false;
  #elif CO2_SENSOR_TYPE == 2
    if (millis() - lastSensorUpdate > (CONFIG->senseInterval * 1000)) {
      ppm = sensor.getCO2();
      yield();
      if (CONFIG->showTemperature) {
        temperature = sensor.getTemperature(false, true);
        yield();
      } else {
        temperature = 0;
      }
      lastSensorUpdate = millis();
      return true;
    }
    return false;

  #endif
  return false;
}
