#include "Wireless.h"

Wireless::Wireless(const char ssid[65], const char password[65])
{
    memcpy(this->SSID, ssid, strlen(ssid)+1);
    memcpy(this->PSK, password, strlen(password)+1);
}

bool Wireless::connect()
{
    if (WiFi.status() == WL_CONNECTED) return true;

    WiFi.begin(SSID, PSK);
    Serial.print("Connecting to WiFi ..");
    unsigned long connectTimeStart = millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        if (millis()-connectTimeStart > 30000) return false;
        delay(connectAttemptPeriod);
    }

    Serial.print("\nWiFi Connected! Local IP: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Wireless::settings()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
}