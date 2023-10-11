#include "Display.h"

Display::Display(bool swapBytes)
{
    TJpgDec.setSwapBytes(swapBytes);
}

void Display::init()
{
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    tft.setTextWrap(false);
}

int Display::getHeight()
{
    return tft.height();
}

void Display::clear()
{
    return tft.fillScreen(TFT_BLACK);
}

void Display::pushImg(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
    tft.pushImage(x, y, w, h, bitmap);
}

void Display::bootPOSTInfo(String message, int xProgress)
{
    tft.drawRoundRect(30,300,250,10,4,0xffff);
    tft.fillRect(30,200,290,90,0x0000);
    tft.setCursor(30,230);
    tft.setTextSize(2);
    tft.setTextColor(0xffff);
    tft.print(message);
    tft.fillRoundRect(30,300, xProgress,10,4,0x07FF);
}

void Display::clearProgressBar()
{
    tft.fillRect(0, 302, 320, 2, TFT_BLACK);
}

void Display::showPlayPauseIcon(bool is_playing)
{
    tft.fillRect(3,310,8,10,0x0000);
    if (is_playing)
    {
        tft.fillTriangle(5,310,5,319,9,314,0xffff);
    } 
    else 
    {
        tft.fillRect(4,310,2,10,0xffff);
        tft.fillRect(9,310,2,10,0xffff);
    }
}

void Display::showTitleName(const char* title, const String artistsName)
{
    const int tftWidth = tft.width();
    clearProgressBar();
    // Serial.print(title);
    tft.setTextSize(2); //changed
    tft.fillRect(0, 320, tftWidth, 23, TFT_BLACK); // Clear Name title row
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(6, 327);
    tft.println(title);

    tft.setTextColor(0xce59);
    tft.fillRect(0, 350, tftWidth, 20, TFT_BLACK); // Clear Artist Name row
    tft.setCursor(6, 350);
    tft.setTextSize(1);
    tft.println(artistsName);
    // tft.fillRect(81, 135, 11, 25, TFT_BLACK); // Clear Artist Name row
    
}

void Display::showImage()
{
    TJpgDec.drawFsJpg(10, 0, "/cover.jpg", LittleFS);
}

// DRY! FIX! both showTime should call one function.
void Display::showProgressTime(const short progressMin, const short progressSec)
{
    const int tftWidth = tft.width();
    tft.fillRect((tftWidth-72), 309, 25, 10, TFT_BLACK);
    tft.setCursor((tftWidth-72), 309);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    if (progressMin >= 10)
    {
        tft.setCursor((tftWidth-77), 309);
    }
    tft.print(String(progressMin));
    
    tft.print(":");
    if (progressSec < 10) 
    {
        tft.print("0");
    }
    tft.print(String(progressSec));
    tft.print(" /");
}

void Display::showDurationTime(const long durationMin, const long durationSec)
{
    const int tftWidth = tft.width();
    tft.fillRect((tftWidth-30), 309, 25, 10, TFT_BLACK);
    tft.setCursor((tftWidth-30), 309);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    if (durationMin >= 10)
    {
        tft.setCursor((tftWidth-35), 309);
    }
    tft.print(String(durationMin));

    tft.print(":");
    if (durationSec < 10) 
    {
        tft.print("0");
    }
    tft.print(String(durationSec));
}

void Display::showProgressBar(const long progressMs, const long durationMs)
{
    int progress = map(progressMs, 0, durationMs, 13, 320);
    tft.fillRect(0, 302, progress, 2, 0x07FF);
}

void Display::showImageLoading()
{
    tft.setCursor(100,170);
    tft.setTextSize(2);
    tft.setTextColor(0xffff);
    tft.print("Loading...");
}