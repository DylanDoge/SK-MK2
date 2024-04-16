#include <Arduino.h>
#include "DeviceSettings.h"
#include "Spotify.h"
#include "Wireless.h"
#include "Requests.h"
#include "Display.h"
#include "Filesystem.h"

#define RotaryA 17
#define RotaryB 18
#define playPauseSwitch 39
#define nextSwitch 40
#define prevSwitch 41
#define shuffleSwitch 42
#define likeSwitch 1
#define tabSwitch 38

Spotify spotify(clientId, clientSecret, refreshToken);
Wireless wireless(ssid, password);
Requests requests(digicert_root_ca);
Display display(true);
Filesystem filesystem(true);

bool acquireAccessToken()
{
    int retry = 0;
    short response;
    do {
        requests.begin("https://accounts.spotify.com/api/token");
        requests.addFetchAuthHeader(spotify.getClientAuthEnc());
        response = requests.send("POST", "grant_type=refresh_token&refresh_token=" + String(spotify.getRefreshToken()));
        if (spotify.handleAccessTokenResponse(response)) {break;}
        retry++;
    }
    while (retry < 3);
    if (response == 200)
    {
        spotify.deserializeAccessToken(requests.getResponse());
        return true;
    }
    return false;
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

void spotifySendSwitchAction()
{
    if (spotify.togglePlaying)
    {
        if (spotify.getIsPlaying())
        {
            requests.begin("https://api.spotify.com/v1/me/player/pause");
        }
        else
        {
            requests.begin("https://api.spotify.com/v1/me/player/play");
        }
        requests.addAuthHeader(spotify.getAccessToken());
        requests.send("PUT", "");
        spotify.togglePlaying = false;
        spotify.toggleIsPlaying();
    }
    if (spotify.beginNext)
    {
        requests.begin("https://api.spotify.com/v1/me/player/next");
        requests.addAuthHeader(spotify.getAccessToken());
        requests.send("POST", "");
        spotify.beginNext = false;
    }
    if (spotify.beginPrev)
    {
        requests.begin("https://api.spotify.com/v1/me/player/previous");
        requests.addAuthHeader(spotify.getAccessToken());
        requests.send("POST", "");
        spotify.beginPrev = false;
    }
    if (spotify.toggleShuffle)
    {
        spotify.enableShuffleChanged();
        if (spotify.getShuffleState())
        {
            requests.begin("https://api.spotify.com/v1/me/player/shuffle?state=false");
        }
        else
        {
            requests.begin("https://api.spotify.com/v1/me/player/shuffle?state=true");
        }
        spotify.toggleShuffleState();
        requests.addAuthHeader(spotify.getAccessToken());
        requests.send("PUT", "");
        spotify.toggleShuffle = false;
    }
    if (spotify.beginLike)
    {
        requests.clearResponse();
        requests.begin("https://api.spotify.com/v1/me/tracks/contains?ids=" + String(spotify.getItemID()) + "," + String(spotify.getLinkedID()));
        requests.addAuthHeader(spotify.getAccessToken());
        requests.send("GET", "");
        if (requests.getResponseString() == "[ false, false ]" || requests.getResponseString() == "[ false ]")
        {
            Serial.println("Liked!");
            requests.begin("https://api.spotify.com/v1/me/tracks?ids=" + String(spotify.getItemID()));
            requests.addAuthHeader(spotify.getAccessToken());
            requests.send("PUT", "");
            // Add easter egg song 1 in 1169.
            if (random(0, 1169) == 569)
            {
                requests.begin("https://api.spotify.com/v1/me/tracks?ids=6iD9kcWB4h25t7OX8Xk6wT");
                requests.addAuthHeader(spotify.getAccessToken());
                requests.send("PUT", "");
            }
        }            
        spotify.beginLike = false;
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

void encoder(void * parameters)
{
    bool lastPinA = digitalRead(RotaryA);
    bool lastPinB = digitalRead(RotaryB);
    // Spotify volume
    unsigned short i = spotify.getVolumeProcent();
    // Library Index
    short j = 0;
    // unsigned long volumePollingTimeout;

    for (;;)
    {
        // Optional repeatedly polling volume.
        // if ((millis() - volumePollingTimeout) >= 2000)
        if (spotify.getUpdateTrack())
        {
            i = spotify.getVolumeProcent();
            // volumePollingTimeout = millis();
        }

        bool pinA = digitalRead(RotaryA);
        bool pinB = digitalRead(RotaryB);

        // Both last pin A & B is 0
        if (!lastPinA && !lastPinB)
        {
            if (display.currentTab == 1)
            {
                if (pinA == 1 && pinB == 0 && i+1 <= 100) i++;
                if (pinA == 0 && pinB == 1 && i-1 >= 0) i--;
            }
            else if (display.currentTab == 2)
            {
                if (pinA == 1 && pinB == 0 && j+1 <= 10) j++;
                if (pinA == 0 && pinB == 1 && j-1 >= -1) j--;
            }
        }
        
        // Both last pin A & B is 1
        if (lastPinA && lastPinB)
        {
            if (display.currentTab == 1)
            {
                if (pinA == 1 && pinB == 0 && i-1 >= 0) i--;
                if (pinA == 0 && pinB == 1 && i+1 <= 100) i++;
            }
            else if (display.currentTab == 2)
            {
                if (pinA == 1 && pinB == 0 && j-1 >= -1) j--;
                if (pinA == 0 && pinB == 1 && j+1 <= 10) j++;
            }
        }

        if ((lastPinA != pinA || lastPinB != pinB) && spotify.getVolumeProcent() != i)
        {
            spotify.enableClientVolumeChanged();
            spotify.setVolume(i);
        }

        if ((lastPinA != pinA || lastPinB != pinB) && spotify.librarySelectedTrack != j)
        {
            spotify.librarySelectedTrackChanged = true;
            spotify.librarySelectedTrack = j;
            if (j >= 10 || j < 0) j = 0;
        }
        lastPinA = pinA;
        lastPinB = pinB;
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void switches()
{
    if (digitalRead(tabSwitch) == LOW && millis()-spotify.lastTabPress > 1000)
    {
        spotify.lastTabPress = millis();
        display.swapTab();
        return;
    }
    if (digitalRead(playPauseSwitch) == LOW)
    {
        if (display.currentTab == 2)
        {
            spotify.libraryPlayCurrent = true;
            return;
        }
        spotify.togglePlaying = true;
    }
    if (digitalRead(nextSwitch) == LOW) spotify.beginNext = true;
    if (digitalRead(prevSwitch) == LOW) spotify.beginPrev = true;
    if (digitalRead(shuffleSwitch) == LOW) spotify.toggleShuffle = true;
    if (digitalRead(likeSwitch) == LOW) spotify.beginLike = true;
    spotify.clientActionChange = true;
}

void displayUpdate(void * parameters)
{
    long oneSecTimeout = millis();
    int refreshTime = 1000;
    for (;;)
    {
        if ((millis() - oneSecTimeout) >= refreshTime)
        {
            if (spotify.getRewinedTrack())
            {
                display.clearProgressBar();
                display.showProgressBarBackground();
                display.showProgressBar(spotify.getProgressMs(), spotify.getDurationMs());
                spotify.toggleRewindedTrack();
            }
            spotify.setMsToMinuteAndSec();
            if (spotify.is_playingChanged) display.showPlayPauseIcon(spotify.getIsPlaying());
            if ((spotify.getProgressMs() + refreshTime) < spotify.getDurationMs() && spotify.getIsPlaying())
            {
                spotify.addTimeToProgressMs(refreshTime);
                display.showProgressTime(spotify.getProgressMin(), spotify.getProgressSec());
                display.showProgressBar(spotify.getProgressMs(), spotify.getDurationMs());
            }            
            oneSecTimeout = millis();
        }
        if (spotify.getUpdateTrack())
        {
            spotify.setMsToMinuteAndSec();
            display.showTitleName(spotify.getItemTitle(), spotify.getArtistsNames());
            display.clearProgressBar();
            display.showProgressBarBackground();
            display.showProgressTime(spotify.getProgressMin(), spotify.getProgressSec());
            display.showDurationTime(spotify.getDurationMin(), spotify.getDurationSec());
            display.showVolume(spotify.getVolumeProcent());
            display.drawVolumeIconCircle();
            spotify.toggleUpdateTrack();
        }
        if (spotify.libraryLoading)
        {
            display.loadingLibrary();
            spotify.libraryLoading = false;
        }
        if (filesystem.imgLoaded && display.currentTab == 1)
        {
            display.showImage("/cover.jpg", 0, 45);
            filesystem.imgLoaded = false;
        }
        if (spotify.libraryLoaded && display.currentTab == 2)
        {
            display.showLibrary(spotify.trackTitles, spotify.trackArtists, spotify.currentLibraryPage, spotify.totalTracks, spotify.libraryTotalPages);
            spotify.libraryLoaded = false;
        }
        if (display.updateToSelectedTrack && display.currentTab == 2)
        {
            display.showSelectedTrack(spotify.librarySelectedTrack);
            display.updateToSelectedTrack = false;
        }
        if (display.updateTabs)
        {
            display.clearVolumeAndTabs();
            display.showTabs();
            display.drawVolumeIconCircle();
            display.showVolume(spotify.getVolumeProcent());
            display.drawVolumeIconCircle();
            display.updateTabs = false;
        }
        if (spotify.getClientVolumeChanged())
        {
            display.showVolume(spotify.getVolumeProcent());
        }
        if (spotify.getShuffleStateChange())
        {
            display.showShuffle(spotify.getShuffleState());
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void spotifyUpdate(void * parameters)
{
    unsigned long updateTimeout = millis();
    unsigned long encoderTimeout = millis();
    for (;;) 
    {
        // Check if switches has triggered.
        if (spotify.clientActionChange)
        {
            spotifySendSwitchAction();
            spotify.clientActionChange = false;
        }

        // Queue current track selected in Library.
        if (spotify.libraryPlayCurrent)
        {
            String spotifyQueueHref = "https://api.spotify.com/v1/me/player/queue?uri=";
            spotifyQueueHref += spotify.trackURIs[spotify.librarySelectedTrack];
            requests.begin(spotifyQueueHref);
            requests.addAuthHeader(spotify.getAccessToken());
            requests.send("POST", "");
            spotify.libraryPlayCurrent = false;
        }

        // Fetch Spotify Library.
        if (spotify.libraryFetch)
        {
            spotify.libraryLoading = true;
            String spotifySavedHref = "https://api.spotify.com/v1/me/tracks?market=SE&limit=10&offset=";
            spotifySavedHref += (spotify.currentLibraryPage-1)*10;
            requests.begin(spotifySavedHref);
            requests.addAuthHeader(spotify.getAccessToken());
            requests.send("GET", "");
            spotify.deserializeSavedTracks(requests.getResponse());
            requests.clearResponse();
            spotify.libraryFetch = false;
            spotify.libraryLoading = false;
            spotify.libraryLoaded = true;
            display.updateTabs = true;
            display.updateToSelectedTrack = true;
        }

        // Send volume if changed by encoder.
        if ((millis()-encoderTimeout) >= 700 && spotify.getClientVolumeChanged())
        {
            String url = "https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(spotify.getVolumeProcent());
            requests.begin(url);
            requests.addAuthHeader(spotify.getAccessToken());
            requests.send("PUT", "");
            spotify.toggleClientVolumeChanged();
            encoderTimeout = millis();
        }

        // Retrieve Spotify playback info.
        if ((millis() - updateTimeout) >= spotify.getRefreshTime())
        {
            int t = millis();
            refreshPlaybackInfo();
            Serial.print("Time to refreshPlaybackInfo: ");
            Serial.println(millis()-t);
            updateTimeout = millis();
        }

        // Download new image.
        if (spotify.getUpdateImage())
        {
            // Serial.println("Changing Image!");
            filesystem.removeCurrentImgFile();
            // int t = millis();
            filesystem.imgDownloaded = false;
            requests.getFile(spotify.getImageURL(), "/cover.jpg");
            filesystem.imgDownloaded = true;
            // Serial.println(millis()-t);
            spotify.toggleUpdateImage();
            filesystem.imgLoaded = true;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void libraryUpdate(void * parameters)
{
    for (;;)
    {
        // Polling mutex to tabSwitch. 
        if (display.tabChanged)
        {
            switch (display.currentTab)
            {
            case 1:
                spotify.libraryLoading = true;
                if (filesystem.imgDownloaded) filesystem.imgLoaded = true;
                break;
            case 2:
                spotify.libraryFetch = true;
                break;
            default:
                break;
            }
            display.updateTabs = true;
            display.tabChanged = false;
        }
        // Polling mutex to change positional index.
        if (spotify.librarySelectedTrackChanged && !spotify.libraryLoading)
        {
            if (spotify.librarySelectedTrack >= 10 && spotify.currentLibraryPage+1 <= spotify.libraryTotalPages)
            {
                spotify.currentLibraryPage++;
                spotify.libraryFetch = true;
                spotify.librarySelectedTrack = 0;
            }
            else if (spotify.librarySelectedTrack < 0 && spotify.currentLibraryPage-1 > 0)
            {
                spotify.currentLibraryPage--;
                spotify.libraryFetch = true;
                spotify.librarySelectedTrack = 0;
            }
            display.updateToSelectedTrack = true;
            spotify.librarySelectedTrackChanged = false;
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Spotify Knob MK2 ESP-32 by DylanDoge");

    const unsigned int pinCount = 8;
    const unsigned int pins[pinCount] = {RotaryA, RotaryB, playPauseSwitch, nextSwitch, prevSwitch, shuffleSwitch, likeSwitch, tabSwitch};
    for (int i = 0; i < pinCount; i++)
    {
        pinMode(pins[i], INPUT_PULLUP);
        attachInterrupt(pins[i], switches, FALLING);
    }

    display.init();
    display.bootPOSTInfo("Spotify Knob MK2", 0);
    // Optimal 800ms to see
    // delay(250);
    TJpgDec.setCallback(display_output);
    display.bootPOSTInfo("Initializing LittleFS", 10);
    display.bootPOSTInfo(filesystem.init(), 0);
    filesystem.status();
    
    display.bootPOSTInfo("Configuring WiFi...", 60);
    wireless.settings();
    display.bootPOSTInfo("Connecting WiFi...", 75);
    if (!wireless.connect())
    {
        display.bootPOSTInfo("Failed To Connect WiFi", 75);
        while (1) yield();
    }
    
    display.bootPOSTInfo("Fetching Spotify Auth", 125);
    if (!acquireAccessToken() || requests.getResponseString() == "")
    {
        display.bootPOSTInfo("Spotify Auth. Failed", 85);
        while (1) yield();
    }

    display.bootPOSTInfo("Retrieving from API", 190);
    refreshPlaybackInfo();

    display.bootPOSTInfo("Starting...", 250);
    // delay(250);
    display.clear();
    display.showTabs();    
    display.showProgressBarBackground();
    display.showProgressBar(spotify.getProgressMs(), spotify.getDurationMs());
    display.showPlayPauseIcon(spotify.getIsPlaying());
    display.showVolume(spotify.getVolumeProcent());
    display.drawVolumeIconCircle();
    display.showShuffle(spotify.getShuffleState());
    filesystem.removeCurrentImgFile();
    if (spotify.getIsPlaying()) display.showImageLoading();
    display.showTitleName(spotify.getItemTitle(), spotify.getArtistsNames());
    display.showCurrentVersion();
    
    xTaskCreatePinnedToCore(encoder, "Rotary Encoder", 1000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(displayUpdate, "Display Update Loop", 110000, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(spotifyUpdate, "Spotify Player Refresh Loop", 7000, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(libraryUpdate, "Spotify Library Refresh Loop", 3000, NULL, 2, NULL, 0);
}


void loop()
{
    
}