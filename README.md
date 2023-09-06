# ESP32 LED STRIP (with OTA updates)

This project utilizes ESP32 to drive a WS2812 LED strip with multiple modes, listening to commands on local web server. Based on my Arduino ESP32 OTA template [found here](https://github.com/kiko283/arduino-esp32-ota).

I have been running this project in my home for more than 7 years, but I just now find the time to upload to git.

I have developed an android app for sending the commands, but the ESP32 has a web server and API so commands can be send by HTTP requests from any device within the network.

The `processCommand` function needs improvement perhaps, but this was adapted from a previous version running on Arduino nano receiving commands via IR.

***NOTE: `Secret.h` file is not in the repository for obvious security reasons, but I have provided a `Secret.example.h` to help users set it up locally.***

Sorry for the quick Readme, don't really have time to explain everything. Though, source code should be pretty much clean and easy to understand. Maybe if I have more free time later on, I'll update it with more detailed explanation.

*NOTE2: Source code for the LEDStrip class is a modified (hopefully improved) version of something I found on the internet. It was long time ago, if someone can point out the original source author, I will gladly update this readme to give credit. Or perhaps maybe I used some example from the NeoPixelBus library (if they provide any). I honestly don't remember, sorry.*
