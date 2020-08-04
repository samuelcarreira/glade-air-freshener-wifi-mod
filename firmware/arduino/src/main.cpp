#include <Arduino.h>
#include <stdio.h>

#include <NTPClient.h>

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <EEPROM.h>
#include <Ticker.h>
#include <time.h>

#include <ArduinoLog.h>
#include <DHTesp.h>

#include "settings.h"
#include "utils.h"
#include "version_defs.h"
#include "Stack.h"

// --- function declarations

void writeEEPROMDefaultValues();
void blinkLED();
void triggerDevice();
void eachSecondTask();
bool checkTriggerSettings();
void disableTriggerPin();
void triggerDeviceButtonRequest();
void triggerDeviceAction();
void handleNotFound();
void handleRoot();
void sendSettingsWebRequest();
void sendDHTWebRequest();
void sendInfoWebRequest();
void saveSettingsWebRequest();
void sendTriggerLogWebRequest();
String formatedJSONArray(bool *ptrArray, uint8_t size);
// void onStationConnected(const WiFiEventSoftAPModeStationConnected evt);
// void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected evt);

// -------------------------------

// WiFiServer server(80);
ESP8266WebServer server(80);
WiFiEventHandler stationModeConnectedHandler;
WiFiEventHandler stationModeDisconnectedHandler;
WiFiEventHandler stationModeAuthModeChangedHandler;
WiFiEventHandler stationModeDHCPTimeoutHandler;

Ticker ledBlinker;
Ticker triggerInterval;
Ticker eachSecond;
Ticker ticker1;

WiFiUDP ntpUDP;

NTPClient ntpTime(ntpUDP, NTP_SERVER, 0, UPDATE_NTP_INTERVAL_MS);

DHTesp dht;
TempAndHumidity dhtReading;

volatile bool deviceIsActiveState = false;  // true if device is active/on

Stack <unsigned long> triggerLog(10); // save the last 10 triggers (epochTime)

void blinkLED() {
    int state = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !state);
}

void eachSecondTask() {
    Serial.println(ntpTime.getFormattedTime());
}

void disableTriggerPin() {
    Log.notice(F("Device off" CR));
    digitalWrite(TRIGGER_PIN, LOW);

    deviceIsActiveState = false;

    return;
}

void triggerDevice() {
    if (deviceIsActiveState) {
        return;
    }

    if (checkTriggerSettings()) {
        Log.notice(F("Timer trigger. Device on" CR));

        triggerDeviceAction();
    }
}

void triggerDeviceAction() {
    deviceIsActiveState = true;

    digitalWrite(TRIGGER_PIN, HIGH);

    ticker1.once(10, disableTriggerPin);

    triggerLog.push(ntpTime.getEpochTime());
}

void triggerDeviceWebRequest() {
    if (deviceIsActiveState) {
        server.send(204, "text/plain");
        return;
    }

    Log.notice(F("Web request to trigger. Device on" CR));

    triggerDeviceAction();

    server.send(200, "text/plain", "1");
}

void triggerDeviceButtonRequest() {
    if (deviceIsActiveState) {
        return;
    }

    Log.notice(F("Button press. Device on" CR));

    triggerDeviceAction();
}

bool checkTriggerSettings() {
    if (!settings.active) {
        Log.notice(F("Device inactive!" CR));
        return false;
    }

    int dayW = ntpTime.getDay();
    int hour = ntpTime.getHours();

    if (!settings.days[dayW]) {
        Log.notice(F("Set to not run on this day: %d!" CR), dayW);
        return false;
    }

    if (!settings.hours[hour]) {
        Log.notice(F("Set to not run on this hour: %d!" CR), hour);
        return false;
    }

    return true;
}

void onStationConnected(const WiFiEventStationModeConnected &evt) {
    Log.notice(F("SSID %s" CR), evt.ssid.c_str());
    Log.notice(F("BSSID %X:%X:%X:%X:%X:%X" CR), evt.bssid[0], evt.bssid[1], evt.bssid[2], evt.bssid[3], evt.bssid[4], evt.bssid[5]);
    Log.notice(F("Channel %d" CR), WiFi.channel());
    Log.notice(F("RSSI %d dBm" CR), WiFi.RSSI());
    Log.notice(F("IP %s" CR), WiFi.localIP().toString().c_str());
    Log.notice(F("MAC %s" CR), WiFi.macAddress().c_str());
    Log.notice(F("Gateway %s" CR), WiFi.gatewayIP().toString().c_str());
    Log.notice(F("Subnet Mask %s" CR), WiFi.subnetMask().toString().c_str());
    Log.notice(F("DNS %s" CR), WiFi.dnsIP().toString().c_str());
    Log.notice(F("Hostname %s" CR), WiFi.hostname().c_str());

    if (ledBlinker.active()) {
        ledBlinker.detach();
        digitalWrite(LED_BUILTIN, LOW);  // turn on LED
    }
}

void onStationDisconnected(const WiFiEventStationModeDisconnected &evt) {
    Log.notice(F("Wi-Fi Station Disconnected." CR));

    if (!ledBlinker.active()) {
        ledBlinker.attach(0.3, blinkLED);  // blink LED
    }

    // Serial.println(evt.reason);
}

void onStationDHCPTimeout() {
    Log.warning(F("DHCP Timeout!" CR));
}

void onStationAuthModeChanged(const WiFiEventStationModeAuthModeChanged &) {
    Log.warning(F("Auth failed!" CR));
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    digitalWrite(LED_BUILTIN, HIGH);  // turn off LED
    digitalWrite(TRIGGER_PIN, LOW);

    Serial.begin(115200);
    EEPROM.begin(512);

    ledBlinker.attach(0.3, blinkLED);

    delay(2000);

    while (!Serial && !Serial.available()) {
    }

    /* LOG_LEVEL_SILENT, LOG_LEVEL_FATAL, LOG_LEVEL_ERROR, LOG_LEVEL_WARNING,
       LOG_LEVEL_NOTICE, LOG_LEVEL_TRACE, LOG_LEVEL_VERBOSE */
    Log.begin(LOG_LEVEL_VERBOSE, &Serial);

    Log.trace(F("v.%s" CR), VERSION_STR);

    Log.trace(F("Reading EEPROM..." CR));

    uint32_t controlCRCvalue = EEPROM.get(0, controlCRCvalue);

    settings = EEPROM.get(sizeof(uint32_t), settings);

    uint32_t settingsCRCvalue = calculateCRC32((uint8_t *)&settings, sizeof(settings));

    Log.notice(F("Control CRC: %x" CR), controlCRCvalue);
    Log.notice(F("EEPROM CRC: %x" CR), settingsCRCvalue);  //  2882105725  482676549

    if (controlCRCvalue != settingsCRCvalue) {
        writeEEPROMDefaultValues();
    }

    Log.notice(F("Connecting to Wi-Fi network" CR));

    // WiFi.persistent(false); // Don't save WiFi configuration in flash

    // Connect to WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.hostname(myHOSTNAME);
    WiFi.begin(mySSID, myPASSWORD);

    stationModeConnectedHandler = WiFi.onStationModeConnected(&onStationConnected);
    stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationDisconnected);
    stationModeDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(&onStationDHCPTimeout);
    stationModeAuthModeChangedHandler = WiFi.onStationModeAuthModeChanged(&onStationAuthModeChanged);

    uint8_t counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        // Log.notice(F("."));

        if (++counter > 15) {
            ESP.restart();
        }
    }

    if (!MDNS.begin(myHOSTNAME)) {
        Log.error(F("Error setting up MDNS responder!" CR));

        while (1) {
            delay(1000);
        }
    } else {
        Log.notice(F("mDNS responder started: http://%s.local/ " CR), myHOSTNAME);

        String ipaddress = WiFi.localIP().toString();
        Log.notice(F("IP Address: http://%s/" CR), &ipaddress);
    }

    // Start TCP (HTTP) server
    server.begin();
    Log.notice(F("TCP server started" CR));

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);

    ntpTime.begin();

    triggerInterval.attach(settings.interval, triggerDevice);
    eachSecond.attach(1, eachSecondTask);

    ticker1.once(10, triggerDevice);  // run check task on startup but wait 10 seconds to get ntp time

    dht.setup(DHT_PIN, DHTesp::DHT11);

    server.on("/", handleRoot);
    server.on("/trigger", HTTP_GET, triggerDeviceWebRequest);
    server.on("/dht", HTTP_GET, sendDHTWebRequest);
    server.on("/getsettings", HTTP_GET, sendSettingsWebRequest);
    server.on("/getinfo", HTTP_GET, sendInfoWebRequest);
    server.on("/gettriggerlog", HTTP_GET, sendTriggerLogWebRequest);
    server.on("/savesettings", HTTP_POST, saveSettingsWebRequest);
    server.onNotFound(handleNotFound);


    Log.notice(F("HTTP server started" CR));
}

void writeEEPROMDefaultValues() {
    Log.notice(F("Writing default settings values..." CR));

    settings = {
        active : true,
        days : {true, true, true, true, true, true, true},
        hours : {false, false, false, false, false, false, false, false,
                 true, true, true, true, true, true, true, true,
                 true, true, true, true, true, true, false, false},
        interval : 60
    };

    uint32_t crcValue = calculateCRC32((uint8_t *)&settings, sizeof(settings));
    Log.notice(F("New EEPROM CRC: %x" CR), crcValue);

    EEPROM.put(0, crcValue);
    EEPROM.put(sizeof(uint32_t), settings);
    EEPROM.commit();

    // Serial.println(crcValue, HEX); 3155185815 BC105097
}

void handleNotFound() {
    String message = "Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    switch (server.method()) {
        case HTTP_HEAD:
            message += "HEAD";
            break;
        case HTTP_ANY:
            message += "ANY";
            break;
        case HTTP_GET:
            message += "GET";
            break;
        case HTTP_POST:
            message += "POST";
            break;
        case HTTP_PUT:
            message += "PUT";
            break;
        case HTTP_PATCH:
            message += "PATCH";
            break;
        case HTTP_DELETE:
            message += "DELETE";
            break;
        case HTTP_OPTIONS:
            message += "OPTIONS";
            break;
    }
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}

void handleRoot() {
    String html = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Test</title></head><body><p>Test</p><img src=\"http://10.0.1.17/ambientador/image.png\"><script src=\"http://10.0.1.17/ambientador/script.js\"></script></body></html>";

    server.send(200, "text/html", html);
}

void sendSettingsWebRequest() {
    const uint8_t arraylen = 250;
    char json[arraylen];

    String jsonDaysArray = formatedJSONArray(settings.days, sizeof(settings.days));
    String jsonHoursArray = formatedJSONArray(settings.hours, sizeof(settings.hours));

    snprintf(json, arraylen, "{\"active\": %s, \"days\": %s, \"hours\": %s, \"interval\": %d}", settings.active ? "true" : "false", jsonDaysArray.c_str(), jsonHoursArray.c_str(), settings.interval);

    server.send(200, "text/json", json);
}

String formatedJSONArray(bool *ptrArray, uint8_t size) {
    String jsonArray = "[";

    for (uint8_t i = 0; i < size; i++) {
        jsonArray += ptrArray[i] ? "true" : "false";
        if (i < size - 1) {
            jsonArray += ", ";
        }
    }
    jsonArray += "]";

    return jsonArray;
}

void sendTriggerLogWebRequest() {
    String json = "{\"history\": [";

    for (uint8_t i = 0; i < triggerLog.size; i++) {
        json += triggerLog.array[i];

        if (i < triggerLog.size - 1) {
            json += ", ";
        }
    }
    json += "]}";
    
    server.send(200, "text/json", json);
}

void sendDHTWebRequest() {
    dhtReading = dht.getTempAndHumidity();

    Serial.println(dht.getStatus());
    Serial.println(dht.getStatusString());

    const uint8_t arraylen = 25;
    char json[arraylen];

    snprintf(json, arraylen, "{\"t\": %d,\"h\": %d}", (int)dhtReading.temperature, (int)dhtReading.humidity);

    server.send(200, "text/json", json);

    Log.trace(F("Temperature: %d C | Humidity: %d %%" CR), (int)dhtReading.temperature, (int)dhtReading.humidity);
}

void sendInfoWebRequest() {    
    const uint8_t arraylen = 150;
    char json[arraylen];


    snprintf(json, arraylen, "{\"firmware\": %s, \"chipID\": \"%08X\", \"freeHeap\": %d, \"IP\": \"%s\", \"MAC\": \"%s\", \"signal\": %d }", 
        VERSION_STR,
        ESP.getChipId(), 
        ESP.getFreeHeap(),
        WiFi.localIP().toString().c_str(),
        WiFi.softAPmacAddress().c_str(),
        WiFi.RSSI());

    server.send(200, "text/json", json);
} 

void saveSettingsWebRequest() {
    uint32_t settingsCRCvalue = calculateCRC32((uint8_t *)&settings, sizeof(settings));

    settings_t tempSettings = settings;

    uint16_t interval = server.arg("interval").toInt();

    if (interval < 30 || interval > 65000) {
        interval = 600;  // 10 minutes
        Log.warning(F("Invalid interval: %d" CR), interval);
    }

    if (settings.interval != interval) {
        tempSettings.interval = interval;

        triggerInterval.detach();
        triggerInterval.attach(interval, triggerDevice);

        Log.trace(F("New interval: %d s" CR), interval);
    }

    

    bool active = server.arg("active") == "true" ? true : false;
    tempSettings.active = active;

    uint32_t tempSettingsCRCvalue = calculateCRC32((uint8_t *)&tempSettings, sizeof(tempSettings));

    Log.trace(F("Current|New settings CRC: %x | %x" CR), settingsCRCvalue, tempSettingsCRCvalue);

    if (settingsCRCvalue == tempSettingsCRCvalue) {
        Log.trace(F("No settings changed!" CR));
    } else {
        Log.trace(F("Settings changed! Saving to EEPROM..." CR));

        settings = tempSettings;

        EEPROM.put(0, tempSettingsCRCvalue);
        EEPROM.put(sizeof(uint32_t), settings);
        EEPROM.commit();
    }

    server.send(200, "text/html", "OK");  // OK
}

void loop() {
    if (!digitalRead(BUTTON_PIN)) {
        triggerDeviceButtonRequest();
    }

    server.handleClient();
    ntpTime.update();
    MDNS.update();
}
