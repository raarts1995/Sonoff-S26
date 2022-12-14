/*
   EEPROM memory address location
   0: 0xEE to indicate used EEPROM
   1: settings (unprotected wifi network, manual control)
   2 - 5: high temperature trigger
   6 - 9: low temperature trigger
   10: length of ssid
   11: length of password
   12 - ...: ssid
   ... - ...: password
*/

#define MAX_SSID_LENGTH 32
#define MAX_PASS_LENGTH 32
#define MAX_MDNS_LENGTH 32
#define MAX_MQTT_NAME_LENGTH 32
#define MAX_MQTT_HOST_LENGTH 64
#define MAX_MQTT_TOPIC_LENGTH 64

typedef struct {
  uint8_t mdnsEnabled : 1;
  uint8_t mqttEnabled : 1;
  uint8_t mqttReportEnabled : 1;
} wifiSettings_t;

typedef struct {
  wifiSettings_t wifiSettings;
  char ssid[MAX_SSID_LENGTH];
  char pass[MAX_PASS_LENGTH];
  
  char mdnsName[MAX_MDNS_LENGTH];

  char mqttName[MAX_MQTT_NAME_LENGTH];
  char mqttHost[MAX_MQTT_HOST_LENGTH];
  uint16_t mqttPort;
  char mqttTopic[MAX_MQTT_TOPIC_LENGTH];
  char mqttReportTopic[MAX_MQTT_TOPIC_LENGTH];
} wifiConfig_t;

wifiConfig_t wifiConfig;

bool settingsChanged = false;

uint16_t eepromSize = sizeof(wifiConfig) + 1;

void writeEEPROM() {
  Serial.println("Writing EEPROM");
  EEPROM.begin(eepromSize);
  EEPROM.write(0, sizeof(wifiConfig) & 0xFF);
  EEPROM.write(1, (sizeof(wifiConfig) >> 8) & 0xFF);

  writeBytes(2, (byte*)&wifiConfig, sizeof(wifiConfig));

  EEPROM.commit();
  EEPROM.end();
}

void readEEPROM() {
  Serial.println("Reading EEPROM");
  EEPROM.begin(eepromSize);
  uint16_t readWifiConfigSize = EEPROM.read(0) | (EEPROM.read(1) << 8);
  if (readWifiConfigSize == sizeof(wifiConfig)) {
    readBytes(2, (byte*)&wifiConfig, sizeof(wifiConfig));
  }
  else {
    Serial.println("Invalid EEPROM");
    Serial.printf("Expected size: %d, read size: %d\n", sizeof(wifiConfig), readWifiConfigSize);
  }
  EEPROM.end();
}

void writeBytes(int addr, byte* data, int len) {
  for (int i = 0; i < len; i++)
    EEPROM.write(addr + i, data[i]);
}

void readBytes(int addr, byte* data, int len) {
  for (int i = 0; i < len; i++)
    data[i] = EEPROM.read(addr + i);
}
