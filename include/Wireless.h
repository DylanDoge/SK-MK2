#ifndef WIRELESS_H
#define WIRELESS_H

#include <Arduino.h>
#include <WiFi.h>

class Wireless
{
private:
    char SSID[65];
    char PSK[65];
    const short connectAttemptPeriod = 20;
    const char spotifyAPI_URL[31] = "https://api.spotify.com/v1/me/";
public:
    Wireless() {}
    
    Wireless(const char ssid[65], const char password[65]);

    bool connect();

    void settings();
};

#endif