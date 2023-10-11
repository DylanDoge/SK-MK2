#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <Arduino.h>
#include <base64.h>
#include <ArduinoJson.h>
// #include "Display.h"

class Spotify
{
private:
    // Class attributes
    bool deviceActive;
    bool supportsVolume;
    bool isPrivateSession;
    bool isRestricted;
    short volumeProcent;
    // Terminating character in char array. Limited to 30 chars
    char deviceName[31];
    // Assuming all device IDs are 40 char long.
    char deviceID[41];

    bool shuffle_state;
    unsigned long progress_ms;
    unsigned long duration_ms;
    // Set max length for artists names to fit on display.
    String artists_name;
    // Asuming url is always 64 char long.
    char image_url[65];
    // Asuming all track IDs are 22 char long.
    char item_ID[23];
    bool is_local;
    // Set max length for track name.
    char item_name[82];
    
    // Currently playing types: track, ad, episode or unknown.
    char currently_playing_type[8];
    bool is_playing;

    short progress_sec;
    short progress_min;
    short duration_min;
    short duration_sec;

    // Spotify Auth
    char clientID[33];
    char clientSecret[33];
    char clientRefresh[132];

    String authorization[88];
    String access_token;
    String clientAuthBase64;

    bool updateImage;
    bool updateTrack;
    bool rewinedTrack;
public:
    Spotify() {}
    Spotify(const char ID[33], const char secret[33], const char refresh[132]);
    
    void printTest();

    void encodeClientAuth();

    String getClientAuthEnc();

    char* getRefreshToken();

    String getAccessToken();

    String getArtistsNames();

    String getImageURL();
    
    char* getItemTitle();

    char* getItemID();

    bool getIsPlaying();

    bool getUpdateTrack();

    bool getUpdateImage();

    void toggleUpdateTrack();

    void toggleUpdateImage();

    void addTimeToProgressMs(unsigned long time);

    unsigned long getProgressMs();

    unsigned long getDurationMs();

    short getDurationMin();
    
    short getDurationSec();

    short getProgressMin();

    short getProgressSec();

    bool handleAccessTokenResponse(short response);
    
    bool handleRefreshPlayerInfoResponse(short response);
    
    void deserializeAccessToken(char *data);

    void deserializePlayerState(char *data);

    void setMsToMinuteAndSec();
};

#endif