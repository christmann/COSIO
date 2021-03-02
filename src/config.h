#ifndef CONFIG
#define CONFIG

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include "logger.h"

#define CONFIG_OPTIONS_CNT	(35 - 5)
#define MAX_CHAR_LENGTH 64

class Config {
  private:
    enum configDataType {
      U8,
      U16,
      U32,
      BOOLEAN,
      STRING
    };

    struct configOption_t {
      char name[25];
      char category[25];
      void* ptr;
      configDataType type;
      uint32_t defaultVal;
      char defaultStr[25];
    };

    Config(); // Singleton, so constructor is private
    Config(const Config&); // Do not allow copy constructor

    void setDefaultValues();

    static Config* _instance;
    Logger logger;
    StaticJsonDocument<1536> document;
    struct configOption_t configOptions[CONFIG_OPTIONS_CNT];

  public:  
    static Config* instance() {
        if (!_instance)
            _instance = new Config();
        return _instance;
    }

    // String setBlacklist[] = {"apSsid", "apPassword", "loginPassword"};
    // String getBlacklist[] = {"password", "apPassword", "loginPassword"};

    void saveConfig();
    void reloadConfig();
    void loadConfig();

    String getParameter(String key);
    bool setParameter(String key, String value);

    bool checkValue(String key, String value);

    JsonObject& getJsonObject();

    String ssid;
    String password;
    String apSsid;
    String apPassword;
    String mdnsname;
    String staticIpAddress;
    String staticGateway;
    String staticSubnet;
    String staticDns;
    String sensorId;
    String sensorName;
    //String loginPassword;
    String tempUnit;
    String humidUnit;
    String co2Unit;
    //String warningMessage;
    String mqttBrokerAddress;
    String mqttUsername;
    String mqttPassword;
    String mqttTopicPrefix;
    uint8_t displayMode;
    uint16_t ppmMax;
    //uint16_t warningInterval;
    uint16_t ppmNormal;
    uint16_t mqttBrokerPort;
    uint16_t mqttUpdateInterval;
    uint32_t senseInterval;
    //uint32_t apColor;
    uint32_t ledIntensity;
    bool showTemperature;
    //bool showWarning;
    //bool ledActive;
    bool dhcpEnabled;
    bool mqttEnabled;
    bool useInbuildCalibration;
    bool calibrateOnPwr;
};
#endif
