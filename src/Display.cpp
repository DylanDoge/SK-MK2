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
    // tft.setRotation(2);
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
    tft.drawRoundRect(30,300,250,10,4,TFT_WHITE);
    tft.fillRect(30,200,290,90,TFT_BLACK);
    tft.setCursor(30,230);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.print(message);
    tft.fillRoundRect(30,300, xProgress,10,4,0x07FF);
}

void Display::clearProgressBar()
{
    tft.fillRect(361, 22, 262, 2, TFT_BLACK);
}

void Display::showPlayPauseIcon(bool is_playing)
{
    short startX = 6;
    short startY = 361;
    tft.fillRect(startX,startY,7,11,TFT_BLACK);
    if (is_playing)
    {
        tft.fillTriangle(startX,startY,startX,371,startX+5,365, TFT_WHITE);
    } 
    else 
    {
        tft.fillRect(startX,startY,2,10, TFT_WHITE);
        tft.fillRect(startX+5,startY,2,10, TFT_WHITE);
    }
}

void Display::showTitleName(const char* title, const String artistsName)
{
    const int tftWidth = tft.width();
    tft.setTextColor(0xce59);
    tft.fillRect(0, 420+5, tftWidth, 8, TFT_BLACK); // Clear Artist Name row
    tft.setCursor(6, 420+5);
    tft.setTextSize(1);
    tft.println(artistsName);    

    // tft.loadFont("FreeSans-24", LittleFS);
    tft.setTextColor(TFT_WHITE);
    tft.fillRect(0, 386+5, tftWidth, 24, TFT_BLACK); // Clear Name title row
    tft.setCursor(6, 387+5);
    tft.setTextSize(2);
    tft.println(title);
    // tft.unloadFont();
}

void Display::showImage(String path, const unsigned int x, const unsigned int y)
{
    TJpgDec.drawFsJpg(x, y, path, LittleFS);
}

void Display::showProgressTime(const short progressMin, const short progressSec)
{
    const int tftWidth = tft.width();
    tft.setTextFont(1);
    tft.fillRect(22, 368, 29, 7, TFT_BLACK);
    tft.setCursor(22, 368);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    // if (progressMin >= 10)
    // {
    //     tft.setCursor((tftWidth-77), 309);
    // }
    tft.print(String(progressMin));
    
    tft.print(":");
    if (progressSec < 10) 
    {
        tft.print("0");
    }
    tft.print(String(progressSec));
}

void Display::showDurationTime(const long durationMin, const long durationSec)
{
    const int tftWidth = tft.width();
    tft.fillRect(256, 368, 28, 7, TFT_BLACK);
    tft.setCursor(261, 368);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    if (durationMin >= 10)
    {
        tft.setCursor(256, 368);
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
    long progress = 0;
    if (progressMs != durationMs)
    {
        progress = map(progressMs, 0, durationMs, 22, 284-22);
    }
    tft.fillRect(22, 361, progress, 2, 0x07FF);
}

void Display::showProgressBarBackground()
{
    tft.fillRect(22, 361, 284-22, 2, 0x6B6D);
}

void Display::showImageLoading()
{
    tft.setCursor(100,170);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.print("Loading...");
}

// Mathmatically get x,y from dividing circle chunks to one
void Display::showVolume(const unsigned short volume)
{
    const String volumeStr = String(volume);
    const unsigned short xShift = 2;
    tft.fillRect(301+xShift, 78, 20, 7, TFT_BLACK);
    if (volume < 10)
    {
        tft.setCursor(309+xShift,78);
    } 
    else if (volume < 100)
    {
        tft.setCursor(305+xShift,78);
    }
    else
    {
        tft.setCursor(301+xShift,78);
    }
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.print(volumeStr);

    int volumeProgress = map(volume, 0, 100, 309, 96);
    if (volume < previousVolume)
    {
        tft.fillRect(307+xShift, 96, 4, volumeProgress-96, TFT_BLACK);
    }
    tft.fillRect(307+xShift, volumeProgress, 4, 309-volumeProgress, TFT_WHITE);

    int volumeKnobStep = floor(volume/volumeStep);
    if (volumeKnobStep != floor(this->previousVolume/volumeStep))
    {
        tft.fillCircle(volKnobCenterPosX, volKnobCenterPosY, 4, TFT_BLACK);
        switch (volumeKnobStep)
        {
        case 0:
            tft.drawLine(volKnobCenterPosX, volKnobCenterPosY, 306, 334, volKnobArrowColor);
            break;
        case 1:
            tft.drawFastHLine(volKnobCenterPosX-4, volKnobCenterPosY, 5, volKnobArrowColor);
            break;
        case 2:
            tft.drawLine(volKnobCenterPosX, volKnobCenterPosY, 306, 328, volKnobArrowColor);
            break;
        case 3:
            tft.drawFastVLine(volKnobCenterPosX, volKnobCenterPosY-4, 5, volKnobArrowColor);
            break;
        case 4:
            tft.drawLine(volKnobCenterPosX, volKnobCenterPosY, 312, 328, volKnobArrowColor);
            break;
        case 5:
            tft.drawFastHLine(volKnobCenterPosX, volKnobCenterPosY, 5, volKnobArrowColor);
            break;
        case 6:
            tft.drawLine(volKnobCenterPosX, volKnobCenterPosY, 312, 334, volKnobArrowColor);
            break;
        default:
            tft.drawFastVLine(volKnobCenterPosX, volKnobCenterPosY, 5, volKnobArrowColor);
            break;
        }
    }
    
    this->previousVolume = volume;
}

void Display::drawVolumeIconCircle()
{
    // 8 steps bottom reserved for null
    tft.drawCircle(volKnobCenterPosX, volKnobCenterPosY, 5, TFT_WHITE);
}

void Display::showShuffle(bool state)
{
    const unsigned startXPos = 295;
    const unsigned startYPos = 360;

    const unsigned short width = 17;
    const unsigned short height = 16;

    for (int row = 0; row < height; row++)
    {
        for (int collumn = 0; collumn < width; collumn++)
        {
            if (BMPArray[row][collumn] && state)
            {
                tft.drawPixel(startXPos+collumn, startYPos+row, TFT_WHITE);
            }
            else if (BMPArray[row][collumn])
            {
                tft.drawPixel(startXPos+collumn, startYPos+row, 0x73ae);
            }
        }
    }
}

void Display::square()
{
    tft.fillRect(0, 0, 32, 32, TFT_WHITE);
}

void Display::showTabs()
{
    uint8_t squareLength = 15;
    uint8_t topTextPadding = 4;
    uint8_t leftTextPadding = 5;
    tft.fillRect(tft.width()-squareLength, 0, squareLength, squareLength*2+1, TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(tft.width()-squareLength+leftTextPadding, topTextPadding);
    switch (currentTab)
    {
        // Tab 1: Player
        case 1:
            tft.fillRect(tft.width()-squareLength, 0, squareLength, squareLength, 0x5aab);
            tft.setTextColor(TFT_WHITE);
            tft.print("P");
            tft.setTextColor(TFT_BLACK);
            break;
        // Tab 2: Library
        case 2:
            tft.fillRect(tft.width()-squareLength, squareLength+1, squareLength, squareLength, 0x5aab);
            tft.setTextColor(TFT_BLACK);
            tft.print("P");
            tft.setTextColor(TFT_WHITE);
            break;
        default:
            break;
    }
    // Print last tab
    tft.setCursor(tft.width()-squareLength+leftTextPadding, topTextPadding+1+squareLength);
    tft.print("L");
    // Reset text Color
    tft.setTextColor(TFT_WHITE);
}

void Display::swapTab()
{
    if (!tabChanged)
    {
        tabChanged = true;
        if (currentTab < tabsCount)
        {
            currentTab++;
            return;
        }
        currentTab = 1;
    }
}

void Display::showLibrary(String *tracksTitles, String *trackArtists, unsigned int currentPage, unsigned int totalTracks, unsigned int totalPages)
{
    tft.fillRect(0, 0, 300, 52+300, TFT_BLACK);
    tft.setTextSize(1);
    for (int i = 0; i < 10; i++)
    {
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(5, (i*30)+16);
        tft.print(tracksTitles[i]);
        tft.setTextColor(0xb596);
        tft.setCursor(5, (i*30)+26);
        tft.print(trackArtists[i]);
    }
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(100, 22+300);
    tft.print("page: ");
    tft.print(currentPage);
    tft.print("/");
    tft.print(totalPages);
}

void Display::loadingLibrary()
{
    tft.fillRect(0, 0, 300, 52+300, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(90, 30+150);
    tft.print("Loading...");
}

void Display::clearVolumeAndTabs()
{
    tft.fillRect(300, 0, 20, 52+300, TFT_BLACK);
}

void Display::showSelectedTrack(const unsigned int selectedIndex)
{
    // for (int i = -1; i < 10; i++)
    // {
    // }
    tft.drawRect(0, (30*this->lastSelectedIndex)+10, 300, 30, TFT_BLACK);
    if (selectedIndex <= 9 && selectedIndex >= 0)
    {
        this->lastSelectedIndex = selectedIndex;
        tft.drawRect(0, (30*selectedIndex)+10, 300, 30, TFT_WHITE);
    }
}

void Display::showCurrentVersion()
{
    tft.setCursor(0, 470);
    tft.setTextSize(1);
    tft.setTextColor(0xef7d);
    tft.print("Spotify MK2 v2.11b BETA");
    tft.setTextColor(TFT_WHITE);
}