#include "sensor.h"

Sensor* Sensor::_instance = NULL;

#if CO2_SENSOR_TYPE == 1
  Sensor::Sensor(){
    lastSensorUpdate = 0;
    isCalibrating = false;
    calibratedOnPwr = false;
    ppm = 0;
    temperature = 0;
    humidity = 0;
  }
#elif CO2_SENSOR_TYPE == 2
  Sensor::Sensor() :
  sensorSerial(D7, D8){
    lastSensorUpdate = 0;
    isCalibrating = false;
    calibratedOnPwr = false;
    ppm = 0;
    temperature = 0;
    humidity = 0;
  }
#elif CO2_SENSOR_TYPE == 3
  Sensor::Sensor() :
  sensorSerial(D7, D8){
    lastSensorUpdate = 0;
    isCalibrating = false;
    calibratedOnPwr = false;
    ppm = 0;
    temperature = 0;
    humidity = 0;
  }
#endif


unsigned long calibrationInit = 0;

// modbus commands for SenseAir S8
const byte readCO2[7] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};
const byte disableABC[8] = {0xFE, 0x06, 0x00, 0x1F, 0x00, 0x00, 0xAC, 0x03};
const byte enableABC[8] = {0xFE, 0x06, 0x00, 0x1F, 0x00, 0xB4, 0xAC, 0x74};

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
    // set inbuild auto-calibratio

    sensor.autoCalibration(CONFIG->useInbuildCalibration, 24);

    logger.printlog(logger.INFO, "MH-Z19C CO2 Sensor initialized!");
  #elif CO2_SENSOR_TYPE == 3
    // initialize SenseAir S8 sensor
    logger.printlog(logger.INFO, "Initializing SenseAir S8 CO2 sensor");
    
    sensorSerial.begin(9600);
    delay(1000);

    for(int i = 0; i < 20; i++){
      if (!sensorSerial.available())
      {
        sensorSerial.write(readCO2, 7);
        delay(200);
      }
      else{
        logger.printlog(logger.INFO, "SenseAir S8 CO2 sensor initialized");
        if(CONFIG->useInbuildCalibration){
          if (!sendSenseairCommand(enableABC, true)) break;
        }
        else {
          if (!sendSenseairCommand(disableABC, true)) break;
        }
        logger.printlog(logger.INFO, "Automatic Baseline Correction was configured");
        return;
      }
    }

    oled->hwError("Sensor error");
    logger.printlog(logger.ERROR, "SenseAir S8 communication problem! Halting");
    while (1) {
      delay(100);
    }

  #endif
}

void Sensor::startManualCalibration(){
  #if CO2_SENSOR_TYPE == 1
    sensor.setForcedRecalibrationFactor(400);
  #elif CO2_SENSOR_TYPE == 2
    sensor.calibrate();
  #endif
  logger.printlog(logger.INFO, "Calibration was performed");
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
      if(ppm < LOWER_LIMIT){
        ppm = LOWER_LIMIT;
      }
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
      if(ppm < LOWER_LIMIT){
        ppm = LOWER_LIMIT;
      }
      return true;
    }
    return false;
  #elif CO2_SENSOR_TYPE == 3
    if (millis() - lastSensorUpdate > (CONFIG->senseInterval * 1000)) {
      lastSensorUpdate = millis();
      int response[7];

      while (!sensorSerial.available())
      {
        sensorSerial.write(readCO2, 7);
        delay(50);
      }

      int timeout = 0;
      while (sensorSerial.available() < 7 )
      {
        timeout++;
        if (timeout > 10)
        {
          while (sensorSerial.available())
            sensorSerial.read();

          break;
        }
        delay(50);
      }

      for (int i = 0; i < 7; i++)
      {
        response[i] = sensorSerial.read();
      }

      ppm = response[3] * 256 + response[4];

      if(ppm < LOWER_LIMIT){
        ppm = LOWER_LIMIT;
      }
      return true;
    }
    return false;
  #endif
  return false;
}

#if CO2_SENSOR_TYPE == 3

bool Sensor::sendSenseairCommand(const byte command[], bool validIfEchoed){
  
  logger.printlog(logger.INFO, "Sending bytes: ");
  for (int i = 0; i < sizeof(command); i++) Serial.print(command[i], HEX);

  while (!sensorSerial.available()) {
    sensorSerial.write(command, sizeof(command));
    delay(50);
  }
  
  if(!validIfEchoed){
    return true;
  }

  int timeout = 0;

  while (sensorSerial.available() < sizeof(command) ) {
    timeout++;
    if (timeout > 10) {
      while (sensorSerial.available()){
        sensorSerial.read();
      }
      break;
    }
    delay(50);
  }

  logger.printlog(logger.INFO, "Recieved bytes: ");
  
  for (int i = 0; i < sizeof(command); i++) {
    byte currentByte = sensorSerial.read();
    Serial.print(currentByte, HEX);
    if(command[i] != currentByte){
      return false;
    }
  }
  return true;
}

#endif
