#include "Wireless.h"

Wireless::Wireless(const char ssid[65], const char password[65])
{
    memcpy(this->SSID, ssid, strlen(ssid)+1);
    memcpy(this->PSK, password, strlen(password)+1);
}

void Wireless::connect()
{
    if (WiFi.status() == WL_CONNECTED) {return;}

    WiFi.begin(SSID, PSK);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print(".");
        delay(connectAttemptPeriod);
    }

    Serial.print("\nWiFi Connected! Local IP: ");
    Serial.println(WiFi.localIP());
}

void Wireless::settings()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
}