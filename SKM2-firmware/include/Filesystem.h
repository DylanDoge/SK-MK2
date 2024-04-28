#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include <LittleFS.h>

class Filesystem
{
private:
    bool failed = false;
public:
    bool imgLoaded;
    bool imgDownloaded;
    Filesystem() {}

    Filesystem(bool empty);

    String init();
    // Halt program if FS has failed.
    void status();

    bool removeCurrentImgFile();

    bool exist(String &path);
};

#endif