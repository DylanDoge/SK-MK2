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
    char linked_track_ID[23];
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
    bool clientVolumeChanged = false;
    bool shuffleStateChanged = false;

    // GET player info refreshTime
    unsigned short refreshTime = 1500;
public:
    bool clientActionChange;
    bool togglePlaying;
    bool beginNext;
    bool beginPrev;
    bool toggleShuffle;
    bool beginLike;

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

    char* getLinkedID();

    unsigned short getRefreshTime();

    bool getIsPlaying();

    bool getShuffleState();

    bool getShuffleStateChange();

    bool getUpdateTrack();

    bool getUpdateImage();
    
    /*----------------------------------------
     *   Volume for Rotary Encoder
     *----------------------------------------*/
    /* Changes or retrevies boolean state for clientVolumeChanged.
     *
     * Example: if volume is changed clientVolumeChanged will 
     * become true, and toggles to false only after sending it to
     * the Spotify API. 
    */
    bool getClientVolumeChanged();

    void toggleClientVolumeChanged();

    void enableClientVolumeChanged();

    void enableShuffleChanged();

    void toggleUpdateTrack();

    void toggleUpdateImage();

    void toggleIsPlaying();

    void toggleShuffleState();

    void setVolume(short newVolume);

    void addTimeToProgressMs(unsigned long time);

    unsigned long getProgressMs();

    unsigned long getDurationMs();

    short getDurationMin();
    
    short getDurationSec();

    short getProgressMin();

    short getProgressSec();

    short getVolumeProcent();

    bool handleAccessTokenResponse(short response);
    
    bool handleRefreshPlayerInfoResponse(short response);
    
    void deserializeAccessToken(char *data);

    void deserializePlayerState(char *data);

    void setMsToMinuteAndSec();

    bool getRewinedTrack();

    void toggleRewindedTrack();
};

#endif