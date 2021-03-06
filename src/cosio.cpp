/*
# COSIO Main project code
# all web server functions listed here
*/

#include "cosio.h"

COSIO* COSIO::_instance = NULL;

COSIO::COSIO() : display(0x3C, D2, D1), 
  mqttClient(espClient), 
  server(80) {
    lastMqttPublish = 0;
    lastMqttReconnectAttempt = 0;
    isApMode = false;
    wifiDisabled = false;
}

void COSIO::getSignalStrength(){
  server.send(200, "text/plain", (String) WiFi.RSSI());
}

void COSIO::setCalibrationFlag(){
  CONFIG->calibrateOnPwr = true;
  CONFIG->saveConfig();
  server.send(200, "text/plain", "DONE");
}

void COSIO::scanForNetworks(){
  logger.printlog(logger.INFO, "Scanning for WiFi networks");
  int n = WiFi.scanNetworks();
  String json = "[";
  if (n == 0){
    logger.printlog(logger.INFO, "No networks found!");
    server.send(200, "application/json", "[]");
    return;
  }
  else {
    logger.printlog(logger.INFO, (String) n + " Networks were found");
    for (int i = 0; i < n; ++i) {
      json = json + "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":\"" + WiFi.RSSI(i) + "\",\"channel\":\"" + 
                WiFi.channel(i) + "\",\"encryption\":\"" + (String) Utility::instance()->encryptionTypeStr(WiFi.encryptionType(i)) + "\"},";
    }
    json.remove(json.length() - 1);
    json = json + "]";
    server.send(200, "application/json", json);
  }
}

void COSIO::serveConfigParameter(){
  String key = server.argName(0);
  logger.printlog(logger.INFO, "Configuration key " + key + " was requested by client");
  // special keys that are read from system and not config.json
  if(key == "currentIpAddress"){
    server.send(200, "text/plain", WiFi.localIP().toString()); 
    return;
  } else if(key == "currentSubnet") {
    server.send(200, "text/plain", WiFi.subnetMask().toString()); 
    return;
  } else if (key == "currentGateway") {
    server.send(200, "text/plain", WiFi.gatewayIP().toString()); 
    return;
  } else if (key == "currentDns") {
    server.send(200, "text/plain", WiFi.dnsIP().toString()); 
    return;
  } else if (key == "mqttStatus") {
    String mqttStatus = "Not enabled";
    if (CONFIG->mqttEnabled) {
      if (mqttClient.connected()) {
        mqttStatus = "Connected";
      } else {
        mqttStatus = "Disconnected";
      }
    }
    server.send(200, "text/plain", mqttStatus); 
    return;
  }
  if(key == "passwordStored" && server.arg(0) != NULL){
    logger.printlog(logger.INFO, "checking for pw storage ssid: " + server.arg(0));
    if(CONFIG->ssid == server.arg(0)){
      server.send(200, "text/plain", "true");
    }
    else{
      server.send(200, "text/plain", "false"); 
    }
    return;
  }
  String value = CONFIG->getParameter(key);
  if(value != ""){
    server.send(200, "text/plain", value);
  }
  else {
    server.send(200, "text/plain", "");
  }
}

void COSIO::modifyConfigParameter(){
  String key = server.argName(0);
  logger.printlog(logger.INFO, "Modifying request for key " + key + " to new value " + server.arg(0));
  CONFIG->setParameter(key, server.arg(0));
  server.send(200, "text/plain", "OK");
  CONFIG->saveConfig();
}

/* send last measured values to client */
void COSIO::sendSensorData(){
  server.send(200, "text/plain", (String) SENSOR->temperature + ";" + (String) SENSOR->humidity + ";" + (String) SENSOR->ppm);
}

/* provide SPIFFS files for server */
void COSIO::initServer(){
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.serveStatic("/chartist.min.css", SPIFFS, "/chartist.min.css");
  server.serveStatic("/chartist.min.js", SPIFFS, "/chartist.min.js");
  server.serveStatic("/stylesheet.css", SPIFFS, "/stylesheet.css");
  server.serveStatic("/cloud.svg", SPIFFS, "/cloud.svg");
  server.serveStatic("/humidity.svg", SPIFFS, "/humidity.svg");
  server.serveStatic("/thermometer.svg", SPIFFS, "/thermometer.svg");
  server.serveStatic("/search.svg", SPIFFS, "/search.svg");
  server.serveStatic("/configInterface.js", SPIFFS, "/configInterface.js");
  server.serveStatic("/wifi.svg", SPIFFS, "/wifi.svg");
  server.serveStatic("/wifiscan.svg", SPIFFS, "/wifiscan.svg");
  server.serveStatic("/lock-open.svg", SPIFFS, "/lock-open.svg");
  server.serveStatic("/lock-closed.svg", SPIFFS, "/lock-closed.svg");
  server.serveStatic("/home.svg", SPIFFS, "/home.svg");
  server.serveStatic("/cog.svg", SPIFFS, "/cog.svg");
  server.serveStatic("/logocosio.svg", SPIFFS, "/logocosio.svg");
  server.serveStatic("/mqtt.svg", SPIFFS, "/mqtt.svg");
  server.serveStatic("/settings.html", SPIFFS, "/settings.html");
  server.serveStatic("/fancybutton.css", SPIFFS, "/fancybutton.css");
  server.serveStatic("/spinner.css", SPIFFS, "/spinner.css");
  server.serveStatic("/toggleswitch.css", SPIFFS, "/toggleswitch.css");
  server.serveStatic("/textinput.css", SPIFFS, "/textinput.css");

  server.on("/getsensordata", std::bind(&COSIO::sendSensorData, this));
  server.on("/getconfigparameter", std::bind(&COSIO::serveConfigParameter, this));
  server.on("/setconfigparameter", std::bind(&COSIO::modifyConfigParameter, this));
  server.on("/getwifinetworks", std::bind(&COSIO::scanForNetworks, this));
  server.on("/getsignalstrength", std::bind(&COSIO::getSignalStrength, this));
  server.on("/calibrate", std::bind(&COSIO::setCalibrationFlag, this));

  server.begin();
}

void COSIO::wifiConnect(bool displayStatus) {
  wifiDisabled = false;
  if (CONFIG->ssid == "" || CONFIG->password == "") {
    if (CONFIG->apSsid == "" || CONFIG->apPassword == "") {
      logger.printlog(logger.INFO, "No WiFi SSID or password set and AP mode disabled, disabling WiFi");
      isApMode = false;
      WiFi.mode(WIFI_OFF);
      wifiDisabled = true;
      return;
    }
    logger.printlog(logger.WARN, "No WiFi SSID or password set! Entering Access Point mode");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(CONFIG->apSsid, CONFIG->apPassword);
    isApMode = true;
    if(displayStatus){
      oled->accessPointMessage();
      pixel->apFadeSignal(0, 0, 255);
    }
  } else {
    logger.printlog(logger.INFO, "Connecting To: " + CONFIG->ssid);
    if(displayStatus)
      oled->connectMessage();
    WiFi.disconnect(true);
    if (!CONFIG->dhcpEnabled) {
      IPAddress ip, subnet, gateway, dns;
      bool parseStatus = true;
      parseStatus &= ip.fromString(CONFIG->staticIpAddress);
      parseStatus &= subnet.fromString(CONFIG->staticSubnet);
      parseStatus &= gateway.fromString(CONFIG->staticGateway);
      parseStatus &= dns.fromString(CONFIG->staticDns);
      if (parseStatus) {
        logger.printlog(logger.INFO, "Configuring fixed IP settings");
        WiFi.config(ip, dns, gateway, subnet);
      } else {
        logger.printlog(logger.ERROR, "Failed to parse IP settings, falling back to DHCP");
      }
    }
    if(isApMode){
      WiFi.softAPdisconnect(true);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(CONFIG->ssid, CONFIG->password);
    isApMode = false;
    uint8_t cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      cnt++;
      if (cnt == 120) {
        break;
      }
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
      logger.printlog(logger.INFO, "Connected! IP: " + WiFi.localIP().toString());
      if(displayStatus)
        oled->connectedMessage();
    } else {
      logger.printlog(logger.WARN, "Could not connect to configured WiFi! Entering Access Point mode");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_AP);
      WiFi.softAP(CONFIG->apSsid, CONFIG->apPassword);
      isApMode = true;
      if(displayStatus){
        oled->accessPointMessage();
        pixel->apFadeSignal(0, 0, 255);
      }
    }
  }
}

boolean COSIO::mqttReconnect() {
  boolean ret;

  mqttClient.setServer(CONFIG->mqttBrokerAddress.c_str(), CONFIG->mqttBrokerPort);
  if (CONFIG->mqttUsername != "") {
    logger.printlog(logger.INFO, "Attempting MQTT connection to " + CONFIG->mqttBrokerAddress + ":" + CONFIG->mqttBrokerPort + " with user " + CONFIG->mqttUsername + "...");
    ret = mqttClient.connect(CONFIG->sensorName.c_str(), CONFIG->mqttUsername.c_str(), CONFIG->mqttPassword.c_str());
  } else {
    logger.printlog(logger.INFO, "Attempting MQTT connection to " + CONFIG->mqttBrokerAddress + ":" + CONFIG->mqttBrokerPort + " without user/pass...");
    ret = mqttClient.connect(CONFIG->sensorName.c_str());
  }
  if (ret) {
    logger.printlog(logger.INFO, "MQTT connected!");
  } else {
    logger.printlog(logger.ERROR, "MQTT connection failed, rc=" + (String)mqttClient.state());
  }
  return mqttClient.connected();
}

void COSIO::setup()   {
  // set SW-Watchdog interval higher, due to reset cause 4
  ESP.wdtDisable();
  ESP.wdtEnable(4000);

  Serial.begin(115200);
  logger.printlog(logger.INFO, "Initializing system");
  WiFi.persistent(false);
  WiFi.setAutoConnect(false);
  
  if (CONFIG->sensorName = "") {
    logger.printlog(logger.WARN, "Sensor name not set, using chip ID based one");
	  String sensorId = String(ESP.getChipId(), HEX);
    String sensorName = "CO2Sensor_" + sensorId;
    CONFIG->sensorName = sensorName;
    CONFIG->sensorId = sensorId;
  }

  logger.printlog(logger.INFO, "Sensor Name : " + CONFIG->sensorName);
  logger.printlog(logger.INFO, "Sensor ID   : " + CONFIG->sensorId);

 // initialize neopixel strip
  logger.printlog(logger.INFO, "Initializing LED strip");
  pixel = new NeoPxl();
  pixel->setColor(255, 0, 0);

  // initialize display
  oled = new Oled(&display);
  logger.printlog(logger.INFO, "Initializing SSD1306 OLED screen");
  logger.printlog(logger.INFO, "Auto Calibration is set to " + (String) CONFIG->useInbuildCalibration);

  // Initialize Sensor

  SENSOR->init(oled);

  // check for calibration

  if(CONFIG->calibrateOnPwr){
    logger.printlog(logger.INFO, "Manual Calibration was set active for poweron");
    oled->showCalibrateMessage(CALIBRATE_TIME);
    pixel->calibrateSignal(0, 0, 255, CALIBRATE_TIME * 60);

    logger.printlog(logger.INFO, "Calibration routine started");
    SENSOR->calibrate();
    CONFIG->calibrateOnPwr = false;
    CONFIG->saveConfig();
  }

  wifiConnect(!SENSOR->isCalibrating);

  logger.printlog(logger.INFO, "Initializing MDNS " + CONFIG->mdnsname + ".local");
  if (MDNS.begin(CONFIG->mdnsname)) {
    logger.printlog(logger.INFO, "MDNS responder stared");
  } else {
    logger.printlog(logger.ERROR, "Failed to start MDNS responder");
  }
  
  logger.printlog(logger.INFO, "Initializing WebServer");
  initServer();
  
  // set MQTT timeout
  mqttClient.setSocketTimeout(3);
  
  logger.printlog(logger.INFO, "Device ready");
}

// unsigned long heapTime = 0;

/* Main loop */
void COSIO::loop() {
  server.handleClient(); 
  yield();
  pixel->update();
  oled->update();
  yield();

  // if (millis() - heapTime > 2000) {
  //   uint32_t heap = ESP.getFreeHeap();
  //   logger.printlog(logger.INFO, (String) heap);
  //   heapTime = millis();
  // }

  if(SENSOR->update() && !SENSOR->isCalibrating){
    yield();
    oled->showSensorData((String) SENSOR->ppm, (String) SENSOR->temperature, (String) SENSOR->humidity);
    oled->showStatus(WiFi.status() == WL_CONNECTED, isApMode, CONFIG->mqttEnabled && mqttClient.connected());
    pixel->displayPpm(SENSOR->ppm);
    yield();
  }

  if (!wifiDisabled && WiFi.status() != WL_CONNECTED && !isApMode) {
    wifiConnect(!SENSOR->isCalibrating);
    yield();
  }

  if (CONFIG->mqttEnabled && !isApMode && !SENSOR->isCalibrating) {
    if (!mqttClient.connected()) {
      if (millis() - lastMqttReconnectAttempt > 50000) {
        lastMqttReconnectAttempt = millis();
        if (mqttReconnect()) {
          lastMqttReconnectAttempt = 0;
        }
      }
    } else {
      mqttClient.loop();
      yield();

      if (millis() - lastMqttPublish > (CONFIG->mqttUpdateInterval * 1000)) {
        lastMqttPublish = millis();

        String prefix = CONFIG->mqttTopicPrefix;
        if (!prefix.endsWith("/")) {
          prefix.concat("/");
        }

        mqttClient.publish((prefix + "co2").c_str(), ((String) SENSOR->ppm).c_str());
        yield();
      }
    }
  }
}
