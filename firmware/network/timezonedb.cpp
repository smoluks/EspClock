#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <hardware/h/systick.hpp>

bool GetTimezoneInfo(uint32_t* gmtOffset) {
    timestamp_uS_t timestamp = GetTimestamp();
    
    const char* url = "http://api.timezonedb.com/v2.1/get-time-zone?key=BPNX5ILV8X68&format=json&by=zone&zone=Europe/Belgrade";
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // Use ArduinoJson to parse
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        return false;
    }

    *gmtOffset = doc["gmtOffset"] | 0;

    ESP_LOGI("timezonedb", "gmtOffset %d, request time %d us", *gmtOffset, GetTimestamp() - timestamp);
    return true;
}