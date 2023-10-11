#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <TJpg_Decoder.h>
#include <TFT_eSPI.h>              // Hardware-specific library
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>

WiFiClientSecure client;
HTTPClient https;
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

// Load tabs attached to this sketch
#include "DeviceSettings.h"
#include "Spotify/List_LittleFS.h"
#include "Spotify/Web_Fetch.h"
#include "Spotify/TFTDisplay.h"

// SPOTIFY API global variables
bool is_playing;
bool imgLoaded_OK;
bool playerInactive;
int SpotifyVolume;
int deviceVolume;
int previousVolume;
int currentTime;
int previousTime;
int trackLength;
int currentSec;
int currentMin;
int trackSec;
int trackMin;
String spotifyDeviceID;
String spotifyDeviceName;
String currentArtist;
String previousArtist;
String currentTrackName;
String previousTrackName;
String currentArtURL;
String previousArtURL;

// ROTARY ENCODER pins
#define RotCLK 26
#define RotDT 25
#define RotSW 21

// ROTARY ENCODER global variables
int currentStateCLK;
int lastStateCLK;

// TFT-SCREEN Variables
long TimeOfLastCheckScreen = 0;
long LastSecTFT = 0;

// Declare functions
void initWiFi();
String HTTPSpotify(const char*, String, String);
void acquireToken();
void sensorCheckLoop(void*);
void HTTPUpdateLoop(void*);
void TFTMetadataLoop(void*);
void sensorCheck();
void spotify_test();
void check_rotary();
void spotify_volume(int);
void spotify_seekToPosition(String);
void UpdateSpotify();
String jsonParserToken(String);

void listLittleFS(void);
bool tft_output(int16_t, int16_t, uint16_t, uint16_t, uint16_t*);
void tftPrintCover(String);
void tftClearProgressBar();
void timeFormater(String);
void tftPrintTrackLength();
void tftDrawSeperator();
void tftPrintCurrentTime();
void tftPrintSongName(String, String);
void tftPrintProgressBar();
void tftDrawPauseIcon();
void removeCoverArt();
void tftBootScreenUserInfo(String, int);
bool getFile(String, String);
void tftPrintVolumeBar();
void tftPrintDevice();

void initWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(20);
  }

  Serial.println("WiFi Connected! Local IP: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

String HTTPSpotify(const char* method, String uri, String body) {
  String auth = "Bearer " + accessToken;
  String select = "https://api.spotify.com/v1/me/" + uri;

  Serial.print("[HTTPS] begin...\n");
  if (https.begin(client, select)) {
      https.addHeader("Accept", "application/json");
      https.addHeader("Authorization", auth);
      https.addHeader("Content-Type", "application/json");
      https.addHeader("Content-Length", "0");
      int httpCode = https.sendRequest(method, body);

      if (httpCode > 0) {
          if (httpCode == 200 || httpCode == 204) {
              Serial.printf("[HTTPS] %s... code: %d\n", method, httpCode);
              if (method == "GET" && httpCode != 204) {
                  String jsonResponse = https.getString();
                  return jsonResponse;
              } else if (method == "GET" && httpCode == 204) {
                  playerInactive = true;
              }
          } else if (httpCode == 401) {
              Serial.println("Token Expired, acquiring new one.");
              acquireToken();
            
          } else {
              Serial.println("[HTTPS] error: ");
              Serial.println(httpCode);
          }
      } else {
          Serial.printf("[HTTPS] %s... failed, error %s\n", method, https.errorToString(httpCode).c_str());
      }

  } else {
      Serial.printf("[HTTPS] Unable to connect\n");
  }
  return "";
}

void acquireToken() {
    String authorizationRaw = clientId + ":" + clientSecret;
    int t = millis();
    String authorization = base64::encode(authorizationRaw);
    t = millis() - t;
    Serial.print(t); Serial.println(" ms to encode base64");

    if (https.begin(client, "https://accounts.spotify.com/api/token")) {
        https.addHeader("Accept", "application/json");
        https.addHeader("Authorization", "Basic "+ authorization);
        https.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String body = "grant_type=refresh_token&refresh_token=" + refreshToken;
        int httpCode = https.POST(body);

        if (httpCode > 0 && httpCode == 200) {
            String jsonResponse = https.getString();
            accessToken = jsonParserToken(jsonResponse);
            Serial.println("[HTTPS] POST Token acquired!");
        } else {
            Serial.println("Failed to get token");
            Serial.println(httpCode);
        }

    } else {
      Serial.println("[HTTPS] Could not begin/connect!");
    }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Spotify Knob ESP-32 by DylanDoge");

  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextWrap(false);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);
  // TJpgDec.setJpgScale(2); //150x150 spotify img

  tftBootScreenUserInfo("Initializing LittleFS", 10);
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed.");
    // Reformat LittleFS
    if (LittleFS.format()) {
      tftBootScreenUserInfo("LittleFS failed: formated", 10);
      Serial.println("LittleFS formatted.");
    } else {
      tftBootScreenUserInfo("LittleFS failed: formated failed!", 10);
      Serial.println("LittleFS format failed.");
    }
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  
  Serial.println("LittleFS initialized.");
  tftBootScreenUserInfo("Connecting WiFi...", 75);
  initWiFi();
  // pinMode(4, INPUT_PULLUP); //Next Track GPIO4
  // pinMode(0, INPUT_PULLUP); //Previous Track GPIO0
  // pinMode(2, INPUT_PULLUP); //Skip 10s GPIO2
  // pinMode(15, INPUT_PULLUP); //Rewind 10s GPIO15

  // pinMode(26, INPUT);   // Set encoder pins as inputs
  // pinMode(25, INPUT);
  // pinMode(RotSW, INPUT_PULLUP); //Play/Pause GPIO14
  // lastStateCLK = digitalRead(RotCLK);
  
  // delay(500);
  client.setCACert(digicert_root_ca);
  tftBootScreenUserInfo("Fetching Spotify Auth", 125);
  acquireToken();
  tftBootScreenUserInfo("Retrieving from API", 190);
  UpdateSpotify();
  // listLittleFS();
  // delay(100);
  deviceVolume = SpotifyVolume;
  previousArtURL = currentArtURL;
  previousArtist = currentArtist;
  previousTrackName = currentTrackName;
  previousTime = currentTime;
  tftBootScreenUserInfo("Starting...", 250);
  // delay(100);
  removeCoverArt();
  tftPrintCover(currentArtURL);
  tft.fillScreen(TFT_BLACK);
  tftPrintSongName(currentTrackName, currentArtist);
  tftPrintTrackLength();
  // tftPrintVolumeBar();

  // xTaskCreatePinnedToCore(
  //   sensorCheckLoop, // function name
  //   "Sensor", // task name
  //   3000, // stack size
  //   NULL, // task parameters
  //   1, // task priority
  //   NULL, // task handle
  //   0 // Core
  // );
  xTaskCreatePinnedToCore(
    HTTPUpdateLoop, // function name
    "All HTTP actions", // task name
    60000, // stack size
    NULL, // task parameters
    2, // task priority
    NULL, // task handle
    1 // Core
  );
  xTaskCreatePinnedToCore(
    TFTMetadataLoop, // function name
    "Metadata Update", // task name
    7000, // stack size
    NULL, // task parameters
    1, // task priority
    NULL, // task handle
    0 // Core
  );
  
}

void loop() {
  // spotify_test();
  vTaskDelay(1);
}

void sensorCheckLoop(void * parameters) {
  for (;;) {
    check_rotary();  // Rotary Encoder check routine below
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void HTTPUpdateLoop(void * parameters) {
  for (;;) {
    // sensorCheck();
    if ((millis() - TimeOfLastCheckScreen) > 3000) {
      UpdateSpotify();
      TimeOfLastCheckScreen = millis();
    }

    // Download cover art (slow!)
    if (currentArtURL != previousArtURL) {
      removeCoverArt();
      tftPrintDevice();
      Serial.println(currentArtURL);
      tftPrintCover(currentArtURL);
      previousArtURL = currentArtURL;
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void TFTMetadataLoop(void * parameters) {
  for (;;) {
    if (currentTime < previousTime) {
      tftClearProgressBar();
      // tftPrintCurrentTime();
    }

    // Update Song Title + Artist (fast)
    if (currentTrackName != previousTrackName) {
      tftPrintSongName(currentTrackName, currentArtist);
      previousTrackName = currentTrackName;
      previousArtist = currentArtist;
    }

    if (is_playing && (currentTime+1000 < trackLength)) {
      tftPrintCurrentTime();
      currentTime += 1000;
    }
    if (!is_playing) {
      tftPlayPauseIcon(false);
    } else {
      tftPlayPauseIcon(true);
    }

    tftPrintProgressBar();
    previousTime = currentTime;

    if (imgLoaded_OK) {
      int t = millis();
      TJpgDec.drawFsJpg(10, 0, "/cover.jpg", LittleFS);
      imgLoaded_OK = false;
      t = millis() - t;
      Serial.print(t); Serial.println(" ms to draw to TFT");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void sensorCheck() {
    if (digitalRead(RotSW) == LOW) {
      if (playerInactive) {
          String body = "{\"device_ids\":[\"" + spotifyDeviceID + "\"]}";
          HTTPSpotify("PUT", "player", body);
          playerInactive = false;
      }
      if (is_playing) {
        Serial.println("Switch triggered Pausing!");
        HTTPSpotify("PUT", "player/pause", "");
        is_playing = 0;
      } else {
        Serial.println("Switch triggered Playing!");
        HTTPSpotify("PUT", "player/play", "");
        is_playing = 1;
      }

    }

    if (digitalRead(2) == LOW) {
      Serial.println("Switch triggered Skipping 10s!");
      spotify_seekToPosition("skip");
      UpdateSpotify();
    }
    
    if (digitalRead(15) == LOW) {
      Serial.println("Switch triggered Rewinding 10s!");
      spotify_seekToPosition("rewind");
      UpdateSpotify();
    }

    if (digitalRead(4) == LOW) {
      Serial.println("Switch triggered Next Track");
      HTTPSpotify("POST", "player/next", "");
    } 

    if (digitalRead(0) == LOW) {
      Serial.println("Switch triggered Previous Track");
      HTTPSpotify("POST", "player/previous", "");
    } 

    if ((millis() - LastSecTFT) > 500) {
      // check if volume changed every 1 seconds
      if (abs(deviceVolume - previousVolume) >= 5 ) {
        spotify_volume(deviceVolume); // create task
        previousVolume = deviceVolume;
        LastSecTFT = millis();
      }
    }
}

void spotify_test() {
  long startTime = millis();
  for (int i = 0; i <= 100; i += 5) {
      String volumeURI = "player/volume?volume_percent=" + String(i);
      HTTPSpotify("PUT", volumeURI, "");
  }
  Serial.println(millis()-startTime);
  https.end();
}

void check_rotary() {
  String currentDir = "";
  currentStateCLK = digitalRead(RotCLK);  // Read the current state of CLK
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
      if (digitalRead(RotDT) != currentStateCLK) {
          if (deviceVolume + 2 <= 100) {
            deviceVolume += 2;
            currentDir = "CCW";
          }
      
      } else {
          if (deviceVolume - 2 >= 0) {
            deviceVolume -= 2;
            currentDir = "CW";
          }
        
      }
      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Counter: ");
      Serial.println(deviceVolume);
  }
  lastStateCLK = currentStateCLK;
}

void spotify_volume(int usrVol) {
  String volumeURI = "player/volume?volume_percent=" + String(usrVol);
  HTTPSpotify("PUT", volumeURI, "");
}

void spotify_seekToPosition(String str) {
  int newTime;
  if (str == "skip") {
      newTime = currentTime + 10000;
  } else if (str == "rewind") {
      int skipTime = currentTime - 10000;
      newTime = skipTime;
    if (skipTime <= 0) {
        newTime = 0;
    }
    
  } else {
    return;
  }

  String Time = String(newTime);
  String seekLink = "player/seek?position_ms=" + Time;
  HTTPSpotify("PUT", seekLink, "");
}

void UpdateSpotify() {
  Serial.println("JSON Begin");

  String jsonResponse = HTTPSpotify("GET", "player?market=SE", "");
  DynamicJsonDocument filter(400);
  filter["device"]["volume_percent"] = true;
  filter["device"]["is_private_session"] = true;
  filter["device"]["id"] = true;
  filter["device"]["name"] = true;
  filter["progress_ms"] = true;
  filter["is_playing"] = true;
  filter["currently_playing_type"] = true;

  JsonObject filter_item = filter.createNestedObject("item");
  filter_item["duration_ms"] = true;
  filter_item["name"] = true;
  filter_item["artists"][0]["name"] = true;
  filter_item["album"]["images"] = true;

  DynamicJsonDocument doc(1536);
  DeserializationError error = deserializeJson(doc, jsonResponse, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  if (doc["device"]["is_private_session"]) {
    Serial.println("Private");
    return;
  }
  
  String trackType = doc["currently_playing_type"];
  if (trackType != "track") {
    Serial.println("Not playing track");
    return;
  }

  currentTime = doc["progress_ms"]; // 1319 ms
  JsonObject item = doc["item"];

  int i = 0;
  for (JsonObject album_image : doc["item"]["album"]["images"].as<JsonArray>()) {
    const char* album_image_url = album_image["url"];
    if (i == 1) {
      currentArtURL = String(album_image_url);
    }
    i ++;
  }

  const char* deviceID = doc["device"]["id"];
  const char* deviceName = doc["device"]["name"];
  const char* artist = item["artists"][0]["name"]; // "Emile Mosseri"
  const char* trackNameP = item["name"]; // "Jacob and the Stone"
  currentArtist = String(artist);
  currentTrackName = String(trackNameP);
  spotifyDeviceID = String(deviceID);
  spotifyDeviceName = String(deviceName);
  trackLength = item["duration_ms"]; // 98560 ms
  is_playing = doc["is_playing"]; // true
  SpotifyVolume = doc["device"]["volume_percent"];
  Serial.println("JSON Finish");
  Serial.println(spotifyDeviceID);
  Serial.println(spotifyDeviceName);

  if (!is_playing) {
      Serial.println("No track playing");
  }
}

String jsonParserToken(String data) {
  StaticJsonDocument<16> filter;
  filter["access_token"] = true;

  StaticJsonDocument<384> doc;
  DeserializationError error = deserializeJson(doc, data, DeserializationOption::Filter(filter));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return "";
  }
  String token = doc["access_token"];
  return token;
}