#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <LittleFS.h>

class Filesystem
{
private:
    bool failed = false;
public:
    Filesystem() {}

    Filesystem(bool empty)
    {

    }

    String init()
    {
        if (!LittleFS.begin()) 
        {
            Serial.println("LittleFS initialization failed.");
            // Reformat LittleFS
            if (LittleFS.format()) {
                Serial.println("LittleFS formatted.");
                this->failed = true;
                return "LittleFS failed: formated";
            } 
            else
            {
                Serial.println("LittleFS format failed.");
                this->failed = true;
                return "LittleFS failed: formated failed!";
            }
        }
        this->failed = false;
        Serial.println("LittleFS initialized.");
        return "LittleFS Initialized";
    }

    // Halt program if FS has failed.
    void status()
    {
        if (this->failed)
        {
            while (1) yield(); // Stay here twiddling thumbs waiting
        }
    }

    void removeCurrentImgFile()
    {
        if (LittleFS.exists("/cover.jpg")) {
            Serial.println("Removing old cover image.");
            LittleFS.remove("/cover.jpg");
        }
    }
};

#endif