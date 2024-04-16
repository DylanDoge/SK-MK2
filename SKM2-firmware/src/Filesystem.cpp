#include "Filesystem.h"

Filesystem::Filesystem(bool empty) {}

String Filesystem::init()
{
    if (LittleFS.begin()) 
    {
        this->failed = false;
        Serial.println("LittleFS initialized.");
        return "LittleFS Initialized";
    }

    Serial.println("LittleFS initialization failed.");
    // Reformat LittleFS
    if (LittleFS.format()) {
        Serial.println("LittleFS formatted.");
        this->failed = true;
        return "LittleFS failed: formated";
    } 
    Serial.println("LittleFS format failed.");
    this->failed = true;
    return "LittleFS failed: formated failed!";
    
}

// Halt program if FS has failed.
void Filesystem::status()
{
    if (this->failed) while (1) yield(); // Stay here twiddling thumbs waiting
}

void Filesystem::removeCurrentImgFile()
{
    if (LittleFS.exists("/cover.jpg")) {
        Serial.println("Removing old cover image.");
        LittleFS.remove("/cover.jpg");
    }
}