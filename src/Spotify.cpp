#include "Spotify.h"

Spotify::Spotify(const char ID[33], const char secret[33], const char refresh[132])
{
    if (strlen(ID)+strlen(secret)+strlen(refresh)+3 != 198) {}
    // Assign Auth variables.
    memcpy(this->clientID, ID, strlen(ID)+1);
    memcpy(this->clientSecret, secret, strlen(secret)+1);
    memcpy(this->clientRefresh, refresh, strlen(refresh)+1);
    encodeClientAuth();
}

void Spotify::printTest()
{
    // Serial.println(clientID);
    // Serial.println(clientSecret);
    // Serial.println(clientRefresh);
    Serial.println(this->deviceName);
    Serial.println(this->deviceID);
    Serial.println(this->image_url);
    Serial.println(this->item_name);
    Serial.println(this->item_ID);
    Serial.println(this->is_playing);
    Serial.println(this->volumeProcent);
    Serial.println(this->artists_name);
}

void Spotify::encodeClientAuth()
{
    const String clientCredAdded = String(clientID) + ":" + String(clientSecret);
    this->clientAuthBase64 = base64::encode(clientCredAdded);
}

String Spotify::getClientAuthEnc()
{
    return this->clientAuthBase64;
}

char* Spotify::getRefreshToken()
{
    return this->clientRefresh;
}

String Spotify::getAccessToken()
{
    return this->access_token;
}

String Spotify::getArtistsNames()
{
    return this->artists_name;
}

String Spotify::getImageURL()
{
    return this->image_url;
}

char* Spotify::getItemTitle()
{
    return this->item_name;
}

char* Spotify::getItemID()
{
    return this->item_ID;
}

bool Spotify::getIsPlaying()
{
    return this->is_playing;
}

bool Spotify::getUpdateTrack()
{
    return this->updateTrack;
}

bool Spotify::getUpdateImage()
{
    return this->updateImage;
}

void Spotify::toggleUpdateTrack()
{
    this->updateTrack = !this->updateTrack;
}

void Spotify::toggleUpdateImage()
{
    this->updateImage = !this->updateImage;
}

void Spotify::addTimeToProgressMs(unsigned long time)
{
    this->progress_ms += time;
}

unsigned long Spotify::getProgressMs()
{
    return this->progress_ms;
}

unsigned long Spotify::getDurationMs()
{
    return this->duration_ms;
}

short Spotify::getDurationMin()
{
    return this->duration_min;
}

short Spotify::getDurationSec()
{
    return this->duration_sec;
}

short Spotify::getProgressMin()
{
    return this->progress_min;
}

short Spotify::getProgressSec()
{
    return this->progress_sec;
}


bool Spotify::handleAccessTokenResponse(short response)
{
    bool okResponse = false;
    if (response == 400)
    {
        Serial.println("Failed to Authenticate. Verify Credentials.");
        while (1) yield();
    }
    if (response == 200)
    {
        okResponse = true;
    }
    return okResponse;
}

bool Spotify::handleRefreshPlayerInfoResponse(short response)
{
    bool okResponse = false;
    if (response == 401)
    {
        Serial.println("Invalid or expired token. Reauthing...");
    }
    if (response == 204)
    {
        Serial.println("Not playing content.");
        this->is_playing = false;
        const char name[12] = "Not playing";
        memcpy(this->item_name, name, strlen(name)+1);
        okResponse = true;
    }
    if (response == 200)
    {
        okResponse = true;
    }
    return okResponse;
}

void Spotify::deserializeAccessToken(char *data)
{
    StaticJsonDocument<16> filter;
    filter["access_token"] = true;

    StaticJsonDocument<384> doc;
    DeserializationError error = deserializeJson(doc, data, DeserializationOption::Filter(filter));
    
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    const char *access_token = doc["access_token"];
    this->access_token = String(access_token);
    doc.clear();
}

void Spotify::deserializePlayerState(char *data)
{
    StaticJsonDocument<272> filter;
    filter["device"] = true;

    JsonObject filter_item = filter.createNestedObject("item");
    filter_item["images"] = true;
    filter_item["artists"] = true;
    filter_item["album"]["images"] = true;
    filter_item["id"] = true;
    filter_item["name"] = true;
    filter_item["duration_ms"] = true;
    filter_item["is_local"] = true;
    filter_item["show"]["name"] = true;
    filter["shuffle_state"] = true;
    filter["progress_ms"] = true;
    filter["duration_ms"] = true;
    filter["is_playing"] = true;
    filter["currently_playing_type"] = true;

    StaticJsonDocument<1536> doc;
    DeserializationError error = deserializeJson(doc, data, DeserializationOption::Filter(filter));

    if (error) 
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    if (doc["device"]["is_private_session"]) {
        Serial.println("I: User is in private session");
        this->isPrivateSession = doc["device"]["is_private_session"];
        doc.clear();
        return;
    }
    
    // Assuming 4 values episode, track, ad, unknown.
    String trackType = doc["currently_playing_type"];
    if (trackType == "ad" || trackType == "unknown") {
        Serial.println("I: Not playing track");
        doc.clear();
        return;
    }

    if (doc["item"] == nullptr)
    {
        return;
    }

    JsonObject device = doc["device"];
    const char *device_id = device["id"]; // "8f24f87fcc6bc55a89d00db77931fcf7a45e715d"
    memcpy(this->deviceID, device_id, strlen(device_id)+1);
    const char *device_name = device["name"]; // "DYLANS-PC"
    memcpy(this->deviceName, device_name, strlen(device_name)+1);

    this->deviceActive = device["is_active"]; // true
    this->isRestricted = device["is_restricted"]; // false
    this->supportsVolume = device["supports_volume"]; // true
    this->volumeProcent = device["volume_percent"]; // 100

    this->shuffle_state = doc["shuffle_state"]; // false
    this->progress_ms = doc["progress_ms"]; // 22032
    this->is_playing = doc["is_playing"]; // true
    
    //EPISODE SWITCH
    if (trackType == "track")
    {
        JsonObject item = doc["item"];
        const char *id = item["id"]; // "59eTbibtvvBIGj27eN3zqq"
        if (String(this->item_ID) == String(id))
        {
            doc.clear();
            return;
        }
        memcpy(this->item_ID, id, strlen(id)+1);
        this->updateTrack = true;
        
        JsonArray imagesArray = doc["item"]["album"]["images"];
        JsonObject imageURL = imagesArray[1];
        const char *album_image_url = imageURL["url"];
        this->updateImage = (String(this->image_url) != String(album_image_url)) ? 1 : 0;
        memcpy(this->image_url, album_image_url, strlen(album_image_url)+1);

        int artistNameLength = 0;
        String artistsCombined = "";
        for (JsonObject artist : item["artists"].as<JsonArray>())
        {
            const char *artist_id = artist["id"]; // "436sYg6CZhNefQJogaXeK0", "2tGi3flDueKAWtThOESXo5", ...
            const char *curArtist_name = artist["name"]; // "Camille Saint-Saëns", "Stéphane Tétreault", "Fabien ...
            String curArtistName = String(curArtist_name);
            artistNameLength += curArtistName.length()+1;
            if (artistNameLength < 45)
            {
                artistsCombined = artistsCombined + curArtistName + " ";
                // Serial.println(artistsCombined);
            }
        }
        this->artists_name = artistsCombined;

        this->duration_ms = item["duration_ms"]; // 214986
        this->is_local = item["is_local"]; // false
        
        const char *name = item["name"]; // "Le Cygne"
        memcpy(this->item_name, name, strlen(name)+1);
        
    } else if (trackType == "episode")
    {
        JsonObject item = doc["item"];

        const char *id = item["id"]; // "59eTbibtvvBIGj27eN3zqq"
        if (String(this->item_ID) == String(id))
        {
            doc.clear();
            return;
        }
        memcpy(this->item_ID, id, strlen(id)+1);
        this->updateTrack = true;

        JsonArray imagesArray = doc["item"]["images"];
        JsonObject imageURL = imagesArray[1];
        const char *image_url  = imageURL["url"];
        this->updateImage = (String(this->image_url) != String(image_url)) ? 1 : 0;
        memcpy(this->image_url, image_url, strlen(image_url)+1);

        this->duration_ms = item["duration_ms"];
        const char *name = item["name"]; // "Le Cygne"
        memcpy(this->item_name, name, strlen(name)+1);

        const char* show_name = item["show"]["name"]; // "Gaslight"
        this->artists_name = String(show_name);
        this->is_local = item["is_local"]; // false
    }
    
    doc.clear();
}

// DRY mf! FIX!
void Spotify::setMsToMinuteAndSec()
{
    const long durationTime = this->duration_ms;
    const long progressTime = this->progress_ms;

    float secP = durationTime/1000;
    int minutes = secP/60;
    int sec = round(secP-(minutes*60));
    
    this->duration_min = minutes;
    this->duration_sec = sec;

    secP = progressTime/1000;
    minutes = secP/60;
    sec = round(secP-(minutes*60));
    
    this->progress_min = minutes;
    this->progress_sec = sec;
}