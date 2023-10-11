#include <Arduino.h>
#include "DeviceSettings.h"
#include "Spotify.h"
#include "Wireless.h"
#include "Requests.h"
#include "Display.h"
#include "Filesystem.h"


Spotify spotify(clientId, clientSecret, refreshToken);
Wireless wireless(ssid, password);
Requests requests(digicert_root_ca);
Display display(true);
Filesystem filesystem(true);

bool imgLoaded;

void acquireAccessToken();

void acquireAccessToken()
{
    int retry = 0;
    do {
        requests.begin("https://accounts.spotify.com/api/token");
        requests.addFetchAuthHeader(spotify.getClientAuthEnc());
        short response = requests.send("POST", "grant_type=refresh_token&refresh_token=" + String(spotify.getRefreshToken()));
        if (spotify.handleAccessTokenResponse(response)) {break;}
        retry++;
    }
    while (retry < 2);
    spotify.deserializeAccessToken(requests.getResponse());
}

void refreshPlaybackInfo()
{
    int retry = 0;
    short response;
    do {
        requests.begin("https://api.spotify.com/v1/me/player?market=SE&additional_types=episode");
        requests.addAuthHeader(spotify.getAccessToken());
        response = requests.send("GET", "");
        if (response == 401)
        {
            acquireAccessToken();
        }
        if (spotify.handleRefreshPlayerInfoResponse(response)) {break;}
        retry++;
    }
    while (retry < 2);
    if (response == 200) {
        spotify.deserializePlayerState(requests.getResponse());
        requests.clearResponse();
    }
}

bool display_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if ( y >= display.getHeight() ) return 0;
    // This function will clip the image block rendering automatically at the TFT boundaries
    display.pushImg(x, y, w, h, bitmap);
    return 1; // Return 1 to decode next block
}

void displayUpdate(void * parameters)
{
    long prevTime;
    long oneSecTimeout;
    int refreshTime = 1000;
    for (;;)
    {
        if ((millis() - oneSecTimeout) >= refreshTime)
        {
            if (prevTime > spotify.getProgressMs())
            {
                display.clearProgressBar();
            }
            spotify.setMsToMinuteAndSec();
            display.showPlayPauseIcon(spotify.getIsPlaying());
            display.showProgressTime(spotify.getProgressMin(), spotify.getProgressSec());
            display.showProgressBar(spotify.getProgressMs(), spotify.getDurationMs());
            if (spotify.getProgressMs()+refreshTime < spotify.getDurationMs() && spotify.getIsPlaying())
            {
                spotify.addTimeToProgressMs(refreshTime);
            }
            prevTime = spotify.getProgressMs();
            oneSecTimeout = millis();
        }
        if (spotify.getUpdateTrack())
        {
            spotify.setMsToMinuteAndSec();
            display.showTitleName(spotify.getItemTitle(), spotify.getArtistsNames());
            display.showDurationTime(spotify.getDurationMin(), spotify.getDurationSec());
            spotify.toggleUpdateTrack();
        }
        if (imgLoaded)
        {
            display.showImage();
            imgLoaded = false;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void spotifyUpdate(void * parameters)
{
    long updateTimeout;
    for (;;) 
    {
        // sensorCheck();
        if ((millis() - updateTimeout) >= 3000)
        {
            int t = millis();
            refreshPlaybackInfo();
            Serial.print("Time to refreshPlaybackInfo: ");
            Serial.println(millis()-t);
            updateTimeout = millis();
        }
        if (spotify.getUpdateImage())
        {
            // Serial.println("Changing Image!");
            filesystem.removeCurrentImgFile();
            // int t = millis();
            requests.getFile(spotify.getImageURL(), "/cover.jpg");
            // Serial.println(millis()-t);
            spotify.toggleUpdateImage();
            imgLoaded = true;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Spotify Knob MK2 ESP-32 by DylanDoge");
    display.init();
    display.bootPOSTInfo("Spotify Knob MK2", 0);
    // Optimal 800ms to see
    // delay(250);
    TJpgDec.setCallback(display_output);
    display.bootPOSTInfo("Initializing LittleFS", 10);
    display.bootPOSTInfo(filesystem.init(), 0);
    filesystem.status();
    
    display.bootPOSTInfo("Connecting WiFi...", 75);
    wireless.settings();
    wireless.connect();
    
    display.bootPOSTInfo("Fetching Spotify Auth", 125);
    acquireAccessToken();

    // requests.begin("https://api.spotify.com/v1/me/player/play");
    // requests.addAuthHeader(spotify.getAccessToken());
    // int reply = requests.send("PUT", "");
    
    display.bootPOSTInfo("Retrieving from API", 190);
    refreshPlaybackInfo();
    // spotify.printTest();

    display.bootPOSTInfo("Starting...", 250);
    // delay(250);
    display.clear();
    display.showTitleName(spotify.getItemTitle(), spotify.getArtistsNames());
    display.showProgressBar(spotify.getProgressMs(), spotify.getDurationMs());
    display.showPlayPauseIcon(spotify.getIsPlaying());
    
    spotify.setMsToMinuteAndSec();
    display.showProgressTime(spotify.getProgressMin(), spotify.getProgressSec());
    display.showDurationTime(spotify.getDurationMin(), spotify.getDurationSec());
    
    filesystem.removeCurrentImgFile();
    if (spotify.getIsPlaying()) {
        display.showImageLoading();
    }
    requests.getFile(spotify.getImageURL(), "/cover.jpg");
    display.showImage();

    xTaskCreatePinnedToCore(displayUpdate, "Display Update Loop", 40000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(spotifyUpdate, "Spotify Refresh Loop", 7000, NULL, 2, NULL, 1);
}


void loop()
{
    
}