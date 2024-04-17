# Volume Knob MK2 for Spotify
> A Spotify Web API embedded project to display Spotify information and control it with an interface.
Written for a ESP32-S3 using Arduino Framework on PlatformIO.

This project was a solution for an inexpensive way to modify and view a user's Spotify playback state and being a standalone device. Offloading from the listening device was crucial for having an independent device that can fully function on only an internet connection via Wi-Fi and power. This was to make the device as portable as possible, and to be used off the desk and as a controller for Spotify, i.e. the living room. Having it tethered to a computer, could have benefits as to use its superior computational power to fetch data faster and handle higher data conversions such as font glyphs. However, the project’s goal is to stay as an IOT and an embedded device and using a computer may reduce the challenges of memory constraints.


## Features
* Displaying near real time information of Spotify playback state on a 320 (RGB) (H) x 480 (V) display.
* Current song playing cover art, displays at 300 (RGB) x 300 px.
* Change playback state (play/pause, next, previous, like and shuffle) via hardware: 5 switches + 1 switch on rotary encoder.
* Change volume of Spotify via hardware: 1 rotary encoder.
* Shows saved tracks from Spotify on the display (9 tracks per page) and ability to add songs in it to the queue.
* Efficiency and stability: reducing API calls when Spotify player is inactive, memory optimization, power and handling special Spotify response cases.

## Documentation
The project documentation is in SKM2@Docs (markdown conversion soon):
[Google Docs](https://docs.google.com/document/d/1gdPdVMs15nirWZm48ST1pJ0AiLPySUujLyLp2ialMa4/).

## Setup
See section *Setup* at the [SKM2@Docs](https://docs.google.com/document/d/1gdPdVMs15nirWZm48ST1pJ0AiLPySUujLyLp2ialMa4/) documentation for a step by step guide.

## Usage
To use the device, there are switches and a rotary encoder that controls a Spotify player on another device. There are five switches at the bottom, which trigger an action on each of them. Listing the actions from left to right on the switches are previous track, next track, like song, shuffle, and cycle tabs. The rotary encoder by default increases volume by one for each step, if rotated clockwise, or decreases by one if rotated counterclockwise. If equipped with a rotary encoder with a built-in switch, by default it will have toggle for play/pause as an action.

## Credits
[ArduinoJSON](https://github.com/bblanchon/ArduinoJson) Library by Benoît Blanchon.  
[TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) Library by Bodmer, TFT_eSPI Authors and Adafruit Industries.  
[TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder) Library by Bodmer and ChaN.