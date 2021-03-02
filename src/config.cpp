#include "config.h"
#include "FS.h"

Config* Config::_instance = NULL;

Config::Config() : configOptions{
    {"ssid", "wifi", &ssid, STRING, 0, ""},
    {"password", "wifi", &password, STRING, 0, ""},
    {"apSsid", "wifi", &apSsid, STRING, 0, "CO2 Sensor"},
    {"apPassword", "wifi", &apPassword, STRING, 0, "Passw0rd!"},
    {"mdns", "wifi", &mdnsname, STRING, 0, "co2sensor"},
    {"ipAddress", "wifi", &staticIpAddress, STRING, 0, "0.0.0.0"},
    {"subnet", "wifi", &staticSubnet, STRING, 0, "0.0.0.0"},
    {"dhcp", "wifi", &dhcpEnabled, BOOLEAN, 1, ""},
    {"gateway", "wifi", &staticGateway, STRING, 0, "0.0.0.0"},
    {"dns", "wifi", &staticDns, STRING, 0, "0.0.0.0"},
    {"id", "sensor", &sensorId, STRING, 0, ""},
    {"name", "sensor", &sensorName, STRING, 0, ""},
    //{"loginPassword", "sensor", &loginPassword, STRING, 0, "Passw0rd!"},
    {"ppmMax", "sensor", &ppmMax, U16, 1500, ""},
    {"ppmNormal", "sensor", &ppmNormal, U16, 900, ""},
    {"senseInterval", "sensor", &senseInterval, U32, 2, ""},
    {"showTemperature", "sensor", &showTemperature, BOOLEAN, 0, ""},
    {"useInbuildCalibration", "sensor", &useInbuildCalibration, BOOLEAN, 0, ""},
    {"calibrateOnPwr", "sensor", &calibrateOnPwr, BOOLEAN, 0, ""},
    {"intensity", "led", &ledIntensity, U32, 255, ""},
    {"displayMode", "led", &displayMode, U8, 0, ""},
    //{"active", "led", &ledActive, BOOLEAN, 1, ""},
    {"tempUnit", "ui", &tempUnit, STRING, 0, "°C"},
    {"humidUnit", "ui", &humidUnit, STRING, 0, "%"},
    {"co2Unit", "ui", &co2Unit, STRING, 0, "ppm"},
    //{"warningMessage", "ui", &warningMessage, STRING, 0, "WARNING"},
    //{"warningInterval", "ui", &warningInterval, U16, 10, ""},
    //{"showWarning", "ui", &showWarning, BOOLEAN, 1, ""},
    {"enabled", "mqtt", &mqttEnabled, BOOLEAN, 0, ""},
    {"brokerAddress", "mqtt", &mqttBrokerAddress, STRING, 0, ""},
    {"brokerPort", "mqtt", &mqttBrokerPort, U16, 1883, ""},
    {"username", "mqtt", &mqttUsername, STRING, 0, ""},
    {"password", "mqtt", &mqttPassword, STRING, 0, ""},
    {"topicPrefix", "mqtt", &mqttTopicPrefix, STRING, 0, "CO2Sensor/"},
    {"updateInterval", "mqtt", &mqttUpdateInterval, U16, 10, ""}
    // Remember to update CONFIG_OPTIONS_CNT in config.h when adding/removing config options!
} {
  setDefaultValues();
  logger.printlog(logger.INFO, "Mounting SPI flash file system");
  if (!SPIFFS.begin()) {
    logger.printlog(logger.ERROR, "Failed mounting SPI flash file system");
    return;
  }
  logger.printlog(logger.INFO, "SPI flash file system mounted");
  loadConfig();
}

String Config::getParameter(String key) {
  for (uint8_t i = 0; i < CONFIG_OPTIONS_CNT; ++i) {
    if (String(configOptions[i].name) == key) {
      switch (configOptions[i].type) {
        case U8:
          return (String)*(uint8_t*)(configOptions[i].ptr);
        case U16:
          return (String)*(uint16_t*)(configOptions[i].ptr);
        case U32:
          return (String)*(uint32_t*)(configOptions[i].ptr);
        case BOOLEAN:
          return (String)*(bool*)(configOptions[i].ptr);
        case STRING:
          return *(String*)(configOptions[i].ptr);
      }
    }
  }
  return "";
}

bool Config::setParameter(String key, String value) {
  for (uint8_t i = 0; i < CONFIG_OPTIONS_CNT; ++i) {
    if (String(configOptions[i].name) == key) {
      switch (configOptions[i].type) {
        case U8:
          *(uint8_t*)(configOptions[i].ptr) = value.toInt();
          break;
        case U16:
          *(uint16_t*)(configOptions[i].ptr) = value.toInt();
          break;
        case U32:
          *(uint32_t*)(configOptions[i].ptr) = value.toInt();
          break;
        case BOOLEAN:
          *(bool*)(configOptions[i].ptr) = value.equalsIgnoreCase("true");
          break;
        case STRING:
          *(String*)(configOptions[i].ptr) = value;
          break;
      }
	  break;
    }
  }
}

void Config::saveConfig(){
  logger.printlog(logger.INFO, "Saving config.json to SPIFFS");
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    logger.printlog(logger.ERROR, "Failed opening config.json");
  }

  StaticJsonDocument<1024> newdoc;

  // store values in document
  for (uint8_t i = 0; i < CONFIG_OPTIONS_CNT; ++i) {
    switch (configOptions[i].type) {
      case U8:
        newdoc[configOptions[i].category][configOptions[i].name] = *(uint8_t*)(configOptions[i].ptr);
        break;
      case U16:
        newdoc[configOptions[i].category][configOptions[i].name] = *(uint16_t*)(configOptions[i].ptr);
        break;
      case U32:
        newdoc[configOptions[i].category][configOptions[i].name] = *(uint32_t*)(configOptions[i].ptr);
        break;
      case BOOLEAN:
        newdoc[configOptions[i].category][configOptions[i].name] = *(bool*)(configOptions[i].ptr);
        break;
      case STRING:
        newdoc[configOptions[i].category][configOptions[i].name] = *(String*)(configOptions[i].ptr);
        break;
    }
  }

  //serializeJsonPretty(newdoc, Serial);

  if (serializeJson(newdoc, configFile) == 0) {
    logger.printlog(logger.ERROR, "config.json SerializationError");
  }

  logger.printlog(logger.INFO, "config.json Serialization complete");
  
  configFile.close();
}

void Config::loadConfig(){
  logger.printlog(logger.INFO, "Loading config.json from SPIFFS");
  setDefaultValues();
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    logger.printlog(logger.ERROR, "Failed opening config.json");
    return;
  }

  size_t size = configFile.size();
  if (size > document.capacity()) {
    logger.printlog(logger.ERROR, "config.json file size is too large for memory");
    return;
  }
  logger.printlog(logger.INFO, "config.json loaded into memory");

  DeserializationError error = deserializeJson(document, configFile);
  if (error){
      logger.printlog(logger.ERROR, "config.json DeserializationError: " + String(error.c_str()));
      return;
  }

  // load values from document
  for (uint8_t i = 0; i < CONFIG_OPTIONS_CNT; ++i) {
    switch (configOptions[i].type) {
      case U8:
        *(uint8_t*)(configOptions[i].ptr) = document[configOptions[i].category][configOptions[i].name];
        break;
      case U16:
        *(uint16_t*)(configOptions[i].ptr) = document[configOptions[i].category][configOptions[i].name];
        break;
      case U32:
        *(uint32_t*)(configOptions[i].ptr) = document[configOptions[i].category][configOptions[i].name];
        break;
      case BOOLEAN:
        *(bool*)(configOptions[i].ptr) = document[configOptions[i].category][configOptions[i].name];
        break;
      case STRING:
        *(String*)(configOptions[i].ptr) = document[configOptions[i].category][configOptions[i].name].as<String>();
        break;
    }
  }

  configFile.close();
  //serializeJsonPretty(document, Serial);
  logger.printlog(logger.INFO, "config.json Deserialization complete");
}

void Config::setDefaultValues() {
  for (uint8_t i = 0; i < CONFIG_OPTIONS_CNT; ++i) {
    switch (configOptions[i].type) {
      case U8:
        *(uint8_t*)(configOptions[i].ptr) = (uint8_t)configOptions[i].defaultVal;
        break;
      case U16:
        *(uint16_t*)(configOptions[i].ptr) = (uint16_t)configOptions[i].defaultVal;
        break;
      case U32:
        *(uint32_t*)(configOptions[i].ptr) = (uint32_t)configOptions[i].defaultVal;
        break;
      case BOOLEAN:
        *(bool*)(configOptions[i].ptr) = configOptions[i].defaultVal != 0;
        break;
      case STRING:
        *(String*)(configOptions[i].ptr) = configOptions[i].defaultStr;
        break;
    }
  }
}

// User input check
bool Config::checkValue(String key, String value) {
  if(key == "ipAddress" || key == "subnet" || key == "gateway" || key == "dns"){
    for (size_t i = 0; i < value.length(); i++) {
      int c = value.charAt(i);
      if (c != '.' && (c < '0' || c > '9')) {
        return false;
      }
    }
    return true;
  }
}

