extern bool imgLoaded_OK;
extern int trackLength;
extern int trackSec;
extern int trackMin;
extern int currentSec;
extern int currentMin;
extern int currentTime;
extern int deviceVolume;
extern String spotifyDeviceName;

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);
  
  return 1; // Return 1 to decode next block
}

void tftPrintCover(String urlToJPG) {
  // uint32_t t = millis();
  imgLoaded_OK = getFile(urlToJPG, "/cover.jpg"); // Note name preceded with "/"

  // t = millis() - t;
  // if (loaded_ok) { Serial.print(t); Serial.println(" ms to download"); }
}

void tftClearProgressBar() {
  tft.fillRect(0, 302, 320, 2, TFT_BLACK); // Clear progressbar
}

void timeFormater(String Selector) {
  if (Selector == "totalTime") {
    float secP = trackLength/1000;
    int minutes = secP/60;
    int sec = round(secP-(minutes*60));
    
    trackMin = minutes;
    trackSec = sec;

  } else if (Selector == "currentTime") {
    float secP = currentTime/1000;
    int minutes = secP/60;
    int sec = round(secP-(minutes*60));
    
    currentSec = sec;
    currentMin = minutes;
  }
}

void tftPrintTrackLength() {
  timeFormater("totalTime");
  const int tftWidth = tft.width();
  tft.fillRect((tftWidth-30), 309, 25, 10, TFT_BLACK);
  tft.setCursor((tftWidth-30), 309);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  if (trackMin >= 10) {
    tft.setCursor((tftWidth-35), 309);
  }
  tft.print(String(trackMin));

  tft.print(":");
  if (trackSec < 10) {
      tft.print("0");
  }
  tft.print(String(trackSec));
}

void tftDrawSeperator() {
  // tft.drawLine(92, 136, 92, 154, TFT_WHITE);
}

void tftPrintCurrentTime() {
  const int tftWidth = tft.width();
  timeFormater("currentTime");
  tft.fillRect((tftWidth-72), 309, 25, 10, TFT_BLACK);
  tft.setCursor((tftWidth-72), 309);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  if (currentMin >= 10) {
    tft.setCursor((tftWidth-77), 309);
  }
  tft.print(String(currentMin));
  
  tft.print(":");
  if (currentSec < 10) {
      tft.print("0");
  }
  tft.print(String(currentSec));
  tft.print(" /");
}

void tftPrintSongName(String Title, String Artist) {
  const int tftWidth = tft.width();
  tftClearProgressBar();
  Serial.print(Title);
  tft.setTextSize(2); //changed
  tft.fillRect(0, 320, tftWidth, 21, TFT_BLACK); // Clear Name title row
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(6, 327);
  tft.println(Title);

  tft.setTextColor(0xce59);
  tft.fillRect(0, 350, tftWidth, 20, TFT_BLACK); // Clear Artist Name row
  tft.setCursor(6, 350);
  tft.setTextSize(1);
  tft.println(Artist);
  tftPrintCurrentTime();
  tftPrintTrackLength();
  // tft.fillRect(81, 135, 11, 25, TFT_BLACK); // Clear Artist Name row
  tftDrawSeperator();
}

void tftPrintProgressBar() {
  int currentProgress = map(currentTime, 0, trackLength, 13, 320);
  tft.fillRect(0, 302, currentProgress, 2, 0x07FF);
}

void tftPrintVolumeBar() {
  // int volumeProgress = map(deviceVolume, 0, 100, 300, 0);
  tft.drawLine(319, 280, 319, 50, 0xffff);
}

void tftPrintDevice() {
  tft.setCursor(7,465);
  tft.print(spotifyDeviceName);
}

void removeCoverArt() {
  if (LittleFS.exists("/cover.jpg") == true) {
    Serial.println("Removing old cover image.");
    LittleFS.remove("/cover.jpg");
  }
}

void tftBootScreenUserInfo(String message, int xProgress) {
  tft.drawRoundRect(30,300,250,10,4,0xffff);
  tft.fillRect(30,200,290,90,0x0000);
  tft.setCursor(30,230);
  tft.setTextSize(2);
  tft.setTextColor(0xffff);
  tft.print(message);
  tft.fillRoundRect(30,300, xProgress,10,4,0x07FF);
}

void tftPlayPauseIcon(bool playing) {
  tft.fillRect(3,310,8,10,0x0000);
  if (playing) {
    tft.fillTriangle(5,310,5,319,9,314,0xffff);
  } else {
    tft.fillRect(4,310,2,10,0xffff);
    tft.fillRect(9,310,2,10,0xffff);
  }
}