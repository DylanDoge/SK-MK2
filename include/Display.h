#ifndef DISPLAY_H
#define DISPLAY_H
#define DISABLE_ALL_LIBRARY_WARNINGS

#include <Arduino.h>
#include <TFT_eSPI.h>       // Hardware-specific library
#include <TJpg_Decoder.h>
#include "Sprites.h" 

// #define TFT_BLACK 0xf7de
// #define TFT_WHITE 0x0000

class Display
{
private:
    TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
    short previousVolume = 101;
    const float volumeStep = 16.6666;  // rounded 100/6. Steps between 1 and 7
    const short volKnobCenterPosX = 309;
    const short volKnobCenterPosY = 331;
    const int volKnobArrowColor = 0xF800;
    unsigned short tabsCount = 2;
public:
    unsigned short currentTab = 1;
    bool tabChanged;
    bool updateTabs;
    bool updateToSelectedTrack;

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

    void showImage(String path, const unsigned int x, const unsigned int y);

    // DRY! FIX! both showTime should call one function.
    void showProgressTime(const short progressMin, const short progressSec);

    void showDurationTime(const long durationMin, const long durationSec);

    void showProgressBar(const long progressMs, const long durationMs);

    void showProgressBarBackground();

    void showImageLoading();

    void showVolume(const unsigned short volume);

    void drawVolumeIconCircle();

    void showShuffle(bool);

    void square();

    void showTabs();

    void swapTab();

    void showLibrary(String *tracksTitles, String *trackArtists, unsigned int currentPage, unsigned int totalTracks, unsigned int totalPages);

    void loadingLibrary();

    void clearVolumeAndTabs();

    void showSelectedTrack(const unsigned int selectedIndex);
};
#endif