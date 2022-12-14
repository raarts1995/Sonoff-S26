#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
//#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Ticker.h>
/*
 * 1 MB flash
 * 26 MHz chrystal
 * reset method: no dtr
 * 
 * board: itead sonoff s20
 * CPU freq: 80MHz
 * Flash size: 1MB (FS 64KB OTA: ~470KB)
 * lwIP: v2 lower memory 
 * VTables: Flash
 * MMU 32KB cache 32KB RAM (balanced)
 * 
 * Pinning module:
 * 1st short cable: module TX (usb serial RX)
 * 2nd short cable: module RX (usb serial TX)
 * 1st long cable : module 3V3
 * 2nd long cable : module GND
 * 
 * /------\
 * | O  O |
 * \------/
 * 
 */

#define SW_VER "1.0"
#define COMPILE_DATE __DATE__ " " __TIME__

#define lastConnectedTimeout 10 * 60 * 1000 //10 min * 60 secs * 1000 millisecs

#define relay 12
#define button 0
#define led 13

#define blinkInterval 0.1
#define connectingBlinkInterval 0.3 / blinkInterval
#define apBlinkInterval 1 / blinkInterval
#define connectedBlinks 5

WiFiClient wifiClient;
ESP8266WebServer* webServer = NULL;
PubSubClient* mqttClient = NULL;
ESP8266HTTPUpdateServer* httpUpdater = NULL;

Ticker blinker;

typedef enum {
  bOff,
  bConnecting,
  bAp,
  bConnected
} blinkState_t;
blinkState_t blinkState = bOff;
blinkState_t prevBlinkState = bOff;
uint8_t blinkCtr = 0;

bool prevBtnPress = false;
unsigned long lastAPConnection = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(button, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);

  setRelay(false);

  Serial.begin(115200);

  blinker.attach(blinkInterval, blinkLed);
  readEEPROM();
  initWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:
  handleNetwork();

  //if the AP button in't pressed, the current wifi mode isn't STA and the timout time has passed -> restart
  if ((WiFi.getMode() != WIFI_STA) && ((lastAPConnection + lastConnectedTimeout) < millis())) {
    Serial.println("Last ap connection was more than 10 minutes ago. Restart.");
    ESP.restart();
  }

  //reinit if the wifi connection is lost
  if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("Lost connection. Trying to reconnect.");
    blinkState = bOff;
    closeServer();
    closeMQTT();
    initWiFi();
  }

  //handle button presses (button is inverted)
  bool btnPress = !digitalRead(button);
  if (btnPress && !prevBtnPress) {
    Serial.println("Button pressed");
    toggleRelay();
  }
  prevBtnPress = btnPress;
}

void setRelay(bool state) {
  Serial.println("Set relay " + String(state ? "on" : "off"));
  digitalWrite(relay, state);
  mqttReportState(state);
}

bool getRelay() {
  return digitalRead(relay);
}

void toggleRelay() {
    setRelay(!getRelay());
}

void blinkLed() {
  bool curLedState = getLed();
  
  if (prevBlinkState != blinkState) {
    blinkCtr = 0;
    curLedState = false;
  }
  
  switch (blinkState) {
    case bOff:
      setLed(LOW);
      break;
    case bConnecting:
    case bAp:
      if (blinkCtr >= (blinkState == bConnecting ? connectingBlinkInterval : apBlinkInterval)) {
        setLed(!getLed());
        blinkCtr = 0;
      }
      else
        blinkCtr++;
      break;
    case bConnected:
      if (!curLedState && (blinkCtr < connectedBlinks)) {
        setLed(HIGH);
        blinkCtr++;
      }
      else
      setLed(LOW);
      break;
  }
  
  prevBlinkState = blinkState;
}

void setLed(bool state) {
      digitalWrite(led, !state);
}

bool getLed() {
  return !digitalRead(led);
}
