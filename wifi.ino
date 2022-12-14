#define retryInterval 5000 //ms

unsigned long lastMQTTReconnect;

//send all settings to the connected client
void getAllSettings() {
  Serial.println("All settings requested");

  String response = "{";
  response += "\"ssid\": \"" + String(wifiConfig.ssid) + "\", ";
  response += "\"connected\": " + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ", ";
  response += "\"mdnsEnabled\": " + String(wifiConfig.wifiSettings.mdnsEnabled ? "true" : "false") + ", ";
  response += "\"mdnsName\": \"" + String(wifiConfig.mdnsName) + "\", ";
  
  response += "\"mqttEnabled\": " + String(wifiConfig.wifiSettings.mqttEnabled ? "true" : "false") + ", ";
  response += "\"mqttName\": \"" + String(wifiConfig.mqttName) + "\", ";
  response += "\"mqttHost\": \"" + String(wifiConfig.mqttHost) + "\", ";
  response += "\"mqttPort\": " + String(wifiConfig.mqttPort) + ", ";
  response += "\"mqttTopic\": \"" + String(wifiConfig.mqttTopic) + "\",";
  response += "\"mqttReportEnabled\": " + String(wifiConfig.wifiSettings.mqttReportEnabled ? "true" : "false") + ", ";
  response += "\"mqttReportTopic\": \"" + String(wifiConfig.mqttReportTopic) + "\"";
  response += "}";

  sendData(response);
}
void getMaxInputFields() {
  Serial.println("Max input field sizes requested");
  String response = "{";
  response += "\"maxPassLength\": " + String(MAX_PASS_LENGTH) + ", ";
  response += "\"maxMdnsLength\": " + String(MAX_MDNS_LENGTH) + ", ";
  response += "\"maxMqttNameLength\": " + String(MAX_MQTT_NAME_LENGTH) + ", ";
  response += "\"maxMqttHostLength\": " + String(MAX_MQTT_HOST_LENGTH) + ", ";
  response += "\"maxMqttTopicLength\": " + String(MAX_MQTT_TOPIC_LENGTH);
  response += "}";
  sendData(response);
}

//send a list of available networks to the client connected to the webwebServer
void getSSIDList() {
  Serial.println("Scanning networks");
  String ssidList;
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    Serial.println(String(i) + ": " + ssid);
    if (ssidList.indexOf(ssid) != -1) {
      Serial.println("SSID already in list");
    }
    else {
      if (ssid != "")
      {
        if (ssidList != "")
          ssidList += ",";
        ssidList += ssid;
      }
      else
        Serial.println("SSID empty");
    }
  }
  sendData(ssidList);
}

//send the wifi settings to the connected client of the webwebServer
void getWiFiSettings() {
  Serial.println("WiFi settings requested");
  sendData(wifiConfig.ssid);
}

//store the wifi settings configured on the webpage and restart the esp to connect to this network
void setWiFiSettings() {
  Serial.println("WiFi settings received");
  memset(wifiConfig.ssid, '\0', sizeof(wifiConfig.ssid));
  memset(wifiConfig.pass, '\0', sizeof(wifiConfig.pass));
  
  webServer->arg("ssid").toCharArray(wifiConfig.ssid, sizeof(wifiConfig.ssid) - 1);
  webServer->arg("pass").toCharArray(wifiConfig.pass, sizeof(wifiConfig.pass) - 1);
  sendData("ok");
  settingsChanged = true;
}

//get relay
void getRelayState() {
  Serial.println("Get relay");
  sendData(getRelay() ? "on" : "off");
}

//set relay
void setRelayState() {
  Serial.println("Set relay");
  bool state = webServer->arg("state") == "on";
  setRelay(state);
  
  //respond
  getRelayState();
}

//toggle relay
void toggleRelayState() {
  Serial.println("Toggle relay");
  toggleRelay();
  
  //respond
  getRelayState();
}

//get mqtt settings
void getMQTTSettings() {
  Serial.println("MQTT settings requested");
  
  String response = "{";
  response += "\"mqttEnabled\": " + String(wifiConfig.wifiSettings.mqttEnabled ? "true" : "false") + ", ";
  response += "\"mqttName\": \"" + String(wifiConfig.mqttName) + "\", ";
  response += "\"mqttHost\": \"" + String(wifiConfig.mqttHost) + "\", ";
  response += "\"mqttPort\": " + String(wifiConfig.mqttPort) + ", ";
  response += "\"mqttTopic\": \"" + String(wifiConfig.mqttTopic) + "\"";
  response += "\"mqttReportEnabled\": " + String(wifiConfig.wifiSettings.mqttReportEnabled ? "true" : "false") + ", ";
  response += "\"mqttReportTopic\": \"" + String(wifiConfig.mqttReportTopic) + "\"";
  response += "}";
  
  sendData(response);
}

void setMQTTSettings() {
  Serial.println("MQTT settings received");
  memset(wifiConfig.mqttName, '\0', sizeof(wifiConfig.mqttName));
  memset(wifiConfig.mqttHost, '\0', sizeof(wifiConfig.mqttHost));
  memset(wifiConfig.mqttTopic, '\0', sizeof(wifiConfig.mqttTopic));
  memset(wifiConfig.mqttReportTopic, '\0', sizeof(wifiConfig.mqttReportTopic));
  
  wifiConfig.wifiSettings.mqttEnabled = webServer->arg("enabled") == "1";
  webServer->arg("name").toCharArray(wifiConfig.mqttName, sizeof(wifiConfig.mqttName) - 1);
  webServer->arg("host").toCharArray(wifiConfig.mqttHost, sizeof(wifiConfig.mqttHost) - 1);
  wifiConfig.mqttPort = webServer->arg("port").toInt();
  webServer->arg("topic").toCharArray(wifiConfig.mqttTopic, sizeof(wifiConfig.mqttTopic) - 1);
  wifiConfig.wifiSettings.mqttReportEnabled = webServer->arg("reportEnabled") == "1";
  webServer->arg("reportTopic").toCharArray(wifiConfig.mqttReportTopic, sizeof(wifiConfig.mqttReportTopic) - 1);
  
  Serial.println("Enabled:        " + String(wifiConfig.wifiSettings.mqttEnabled ? "true" : "false"));
  Serial.println("Name:           " + String(wifiConfig.mqttName));
  Serial.println("Host:           " + String(wifiConfig.mqttHost));
  Serial.println("Port:           " + String(wifiConfig.mqttPort));
  Serial.println("Topic:          " + String(wifiConfig.mqttTopic));
  Serial.println("Report enabled: " + String(wifiConfig.wifiSettings.mqttReportEnabled ? "true" : "false"));
  Serial.println("Report topic:   " + String(wifiConfig.mqttReportTopic));
  
  //respond
  sendData("ok");
  settingsChanged = true;
}

void getMDNSSettings() {
  Serial.println("MDNS settings requested");
  
  String response = "{";
  response += "\"mdnsEnabled\": " + String(wifiConfig.wifiSettings.mdnsEnabled ? "true" : "false") + ", ";
  response += "\"mdnsName\": \"" + String(wifiConfig.mdnsName) + "\"";
  response += "}";
  
  sendData(response);
}

void setMDNSSettings() {
  memset(wifiConfig.mdnsName, '\0', sizeof(wifiConfig.mdnsName));
  
  wifiConfig.wifiSettings.mdnsEnabled = webServer->arg("enabled") == "1";
  webServer->arg("name").toCharArray(wifiConfig.mdnsName, sizeof(wifiConfig.mdnsName) - 1);
  
  sendData("ok");
  settingsChanged = true;
}

void getSWVer() {
  Serial.println("SW version requested");
  
  String response = "{";
  response += "\"swVer\": \"" + String(SW_VER) + "\", ";
  response += "\"compileDate\": \"" + String(COMPILE_DATE) + "\"";
  response += "}";

  sendData(response);
}

void restartESP() {
  Serial.println("Restart requested");
  sendData("ok");
  if (settingsChanged) {
    Serial.println("EEPROM changed");
    writeEEPROM();
  }

  //wait 0.5s, then restart
  delay(500);
  ESP.restart();
}

//get the content type of a filename
String getContentType(String filename) {
  if (webServer->hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//send a file from the LittleFS to the connected client of the webwebServer
void sendFile() {
  String path = webServer->uri();
  Serial.println("Got request for: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (LittleFS.exists(path)) {
    Serial.println("File " + path + " found");
    File file = LittleFS.open(path, "r");
    webServer->streamFile(file, contentType);
    file.close();
  }
  else {
    Serial.println("File '" + path + "' doesn't exist");
    webServer->send(404, "text/plain", "The requested file doesn't exist");
  }
  
  lastAPConnection = millis();
}

void listDir(const char* dirname) {
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm * tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}

//send data to the connected client of the webwebServer
void sendData(String data) {
  Serial.println("Sending: " + data);
  webServer->send(200, "text/plain", data);
  
  lastAPConnection = millis();
}

//initialize wifi by connecting to a wifi network or creating an accesspoint
void initWiFi() {
  //digitalWrite(APLed, LOW);
  Serial.print("WiFi: ");
  /*if (!digitalRead(APPin)) {
    Serial.println("AP");
    configureSoftAP();
  }
  else*/ {
    blinkState = bConnecting;
    Serial.println("STA");
    if (!connectWiFi()) {
      Serial.println("Connecting failed. Starting AP");
      configureSoftAP();
      blinkState = bAp;
    }
    else {
      blinkState = bConnected;
      configureMQTT();
    }
  }

  //configure MDNS
  configureMDNS();

  //configure webwebServer
  configureServer();

  //configure OTA (over-the-air update)
  configureOTA();
}

//connect the esp8266 to a wifi network
bool connectWiFi() {
  if (strlen(wifiConfig.ssid) == 0) {
    Serial.println("SSID empty");
    return false;
  }
  
  WiFi.mode(WIFI_STA);
  Serial.println("Attempting to connect to '" + String(wifiConfig.ssid) + "', pass: " + String(wifiConfig.pass));
  WiFi.begin(wifiConfig.ssid, wifiConfig.pass);
  for (int timeout = 0; timeout < 15; timeout++) { //max 15 seconds
    int status = WiFi.status();
    if ((status == WL_CONNECTED) || (status == WL_CONNECT_FAILED))
      break;
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to " + String(wifiConfig.ssid));
    Serial.println("WiFi status: " + String(WiFi.status()));
    WiFi.disconnect();
    return false;
  }
  Serial.println("Connected to " + String(wifiConfig.ssid));
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

//configure the access point of the esp8266
void configureSoftAP() {
  String APSSID = "Sonoff_" + String(ESP.getChipId());
  Serial.println("Configuring AP: " + APSSID);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(APSSID.c_str(), NULL, 1, 0, 1);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(ip);
  lastAPConnection = millis();
  //digitalWrite(APLed, HIGH);
}

bool configureMDNS() {
  if (wifiConfig.wifiSettings.mdnsEnabled) {
    if (String(wifiConfig.mdnsName) == "") {
      Serial.println("No MDNS name set");
      return false;
    }
    Serial.println("Starting mdns on " + String(wifiConfig.mdnsName) + ".local");
    if (!MDNS.begin(wifiConfig.mdnsName))
      return false;
      MDNS.addService("http", "tcp", 80);
      return true;
  }
  return true;
}

//initialize the webwebServer on port 80
void configureServer() {
  Serial.println("Configuring webserver");
  LittleFS.begin();

  //listDir("/");

  if (webServer != NULL) {
    closeServer();
  }
  
  webServer = new ESP8266WebServer(80);
  webServer->on("/getAllSettings", HTTP_GET, getAllSettings);
  webServer->on("/getMaxInputFields", HTTP_GET, getMaxInputFields);
  webServer->on("/getSSIDList", HTTP_GET, getSSIDList);
  webServer->on("/getWiFiSettings", HTTP_GET, getWiFiSettings);
  webServer->on("/setWiFiSettings", HTTP_GET, setWiFiSettings);
  webServer->on("/getRelayState", HTTP_GET, getRelayState);
  webServer->on("/setRelayState", HTTP_GET, setRelayState);
  webServer->on("/toggleRelayState", HTTP_GET, toggleRelayState);
  webServer->on("/getMQTTSettings", HTTP_GET, getMQTTSettings);
  webServer->on("/setMQTTSettings", HTTP_GET, setMQTTSettings);
  webServer->on("/getMDNSSettings", HTTP_GET, getMDNSSettings);
  webServer->on("/setMDNSSettings", HTTP_GET, setMDNSSettings);
  webServer->on("/getSWVer", HTTP_GET, getSWVer);
  webServer->on("/restart", HTTP_GET, restartESP);
  webServer->onNotFound(sendFile); //handle everything except the above things
  webServer->begin();
  Serial.println("WebwebServer started");
}

void closeServer() {
  if (webServer != NULL) {
    delete webServer;
    webServer = NULL;
  }
}

bool configureMQTT() {
  if (mqttClient != NULL) {
    closeMQTT();
  }

  if (!wifiConfig.wifiSettings.mqttEnabled)
    return true;
  
  if (String(wifiConfig.mqttName) == "" || String(wifiConfig.mqttHost) == "" || String(wifiConfig.mqttTopic) == "")
    return false;

  Serial.println("Configuring MQTT");
  Serial.println("Host: " + String(wifiConfig.mqttHost) + ", port: " + String(wifiConfig.mqttPort));
  mqttClient = new PubSubClient(wifiClient);
  mqttClient->setServer(wifiConfig.mqttHost, wifiConfig.mqttPort);
  mqttClient->setCallback(mqttCallback);
  return true;
}

bool mqttReconnect() {
  if (mqttClient == NULL)
    return false;

  int retries = 0;
  if (!mqttClient->connected()) {
    Serial.println("MQTT: attempting to reconnect to host " + String(wifiConfig.mqttHost) + " using name " + String(wifiConfig.mqttName));
    if (mqttClient->connect(wifiConfig.mqttName)) {
      Serial.println("MQTT connected. Subscribing to topic " + String(wifiConfig.mqttTopic) );
      mqttClient->subscribe(wifiConfig.mqttTopic);
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  switch (payload[0]) { //only handle 0 and 1, ignore everything else
    case '0': setRelay(false); break;
    case '1': setRelay(true); break;
    case 't': toggleRelay(); break;
    default: break;
  }
}

void mqttReportState(bool state) {
  if (mqttClient != NULL) {
    if (mqttClient->connected()) {
      mqttClient->publish(wifiConfig.mqttReportTopic, state ? "1" : "0");
    }
  }
}

void closeMQTT() {
  if (mqttClient != NULL) {
    delete mqttClient;
    mqttClient = NULL;
  }
}

// OTA functions

void configureOTA() {
  if (httpUpdater == NULL)
    httpUpdater = new ESP8266HTTPUpdateServer();
  httpUpdater->setup(webServer);
  
  Serial.println("Web updater configured. Load new firmware on " + WiFi.localIP().toString() + "/update");
  
  //doet het niet...
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  /*ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
      
      LittleFS.end();
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");*/
}

void handleNetwork() {
  //handle OTA
  //ArduinoOTA.handle();
  MDNS.update();
  
  //handle WiFi server
  if (webServer != NULL)
    webServer->handleClient();

  //handle MQTT
  if (mqttClient != NULL) {
    if (!mqttClient->connected()) {
      unsigned long now = millis();
      if (now > (lastMQTTReconnect + retryInterval)) {
        lastMQTTReconnect = now;
        mqttReconnect();
      }
    }
    else
      mqttClient->loop();
  }
}
