#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "Spotify.h"

class Input
{
private:
    Spotify spotify = Spotify();
    int RotaryA = -1;
    int RotaryB = -1;
    int playPauseSwitch;
    int nextSwitch;
    int prevSwitch;
    int shuffleSwitch;
    int likeSwitch;
public:
    Input() {}

    void AssignPins(int rotaryPinA, int rotaryPinB, int playPausePin, int nextPin, int prevPin, int shufflePin, int likeSwitchPin)
    {
        // Assign pins
        RotaryA = rotaryPinA;
        RotaryB = rotaryPinB;
        playPauseSwitch = playPausePin;
        nextSwitch = nextPin;
        prevSwitch = prevPin;
        shuffleSwitch = shufflePin;
        likeSwitch = likeSwitchPin;
    }

    void encoder()
    {
        bool lastPinA = digitalRead(RotaryA);
        bool lastPinB = digitalRead(RotaryB);
        unsigned short i = spotify.getVolumeProcent();
        // unsigned long volumePollingTimeout;
        for (;;)
        {
            // Optional repeatedly polling volume.
            // if ((millis() - volumePollingTimeout) >= 2000)
            if (spotify.getUpdateTrack())
            {
                i = spotify.getVolumeProcent();
                // volumePollingTimeout = millis();
            }

            bool pinA = digitalRead(RotaryA);
            bool pinB = digitalRead(RotaryB);

            if (lastPinA == 0 && lastPinB == 0)
            {
                if (pinA == 1 && pinB == 0 && i+1 <= 100)
                {
                    i++;
                    spotify.enableClientVolumeChanged();
                    spotify.setVolume(i);
                }
                if (pinA == 0 && pinB == 1 && i-1 >= 0)
                {
                    i--;
                    spotify.enableClientVolumeChanged();
                    spotify.setVolume(i);
                }
            }
            
            if (lastPinA == 1 && lastPinB == 1)
            {
                if (pinA == 1 && pinB == 0 && i-1 >= 0)
                {
                    i--;
                    spotify.enableClientVolumeChanged();
                    spotify.setVolume(i);
                }
                if (pinA == 0 && pinB == 1 && i+1 <= 100)
                {
                    i++;
                    spotify.enableClientVolumeChanged();
                    spotify.setVolume(i);
                }
            }
            lastPinA = pinA;
            lastPinB = pinB;
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
};



#endif