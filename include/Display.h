#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <TJpg_Decoder.h>   

class Display
{
private:
    TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
public:
    Display() {}

    Display(bool swapBytes);

    void init();

    int getHeight();

    void clear();

    void pushImg(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

    void bootPOSTInfo(String message, int xProgress);

    void clearProgressBar();

    void showPlayPauseIcon(bool is_playing);

    void showTitleName(const char* title, const String artistsName);

    void showImage();

    // DRY! FIX! both showTime should call one function.
    void showProgressTime(const short progressMin, const short progressSec);

    void showDurationTime(const long durationMin, const long durationSec);

    void showProgressBar(const long progressMs, const long durationMs);

    void showImageLoading();
};
#endif