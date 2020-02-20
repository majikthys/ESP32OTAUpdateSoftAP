## Overview

This is just a test project, exploring how to do OTA Update in SOFT AP mode from an arduino perspective.

This is substantially different than other samples of this work in the following ways:

1. This implementation requires no javascript
1. There is no hookie password authentication
1. During setup, this implementation sets the ESP32 IP address to a definitive address (important for writing instructions).

### To use

1. Set parameters in softapConfig.h
1. Compile and flash to board
1. Change VERSION in softapConfig.h
1. Compile and export binary using arduino IDE (DO NOT FLASH BOARD)
1. Connect you computer to the WiFi SSID presented by ESP32
1. Navigate with you browser to "http://192.168.42.1"
1. Make note of version number displayed
1. Upload new binary (created in step 4)
1. Reconnect your computer to WiFi SSID present by ESP32
1. Navigate with you browser to "http://192.168.42.1" and refresh page
1. Observe new version number

