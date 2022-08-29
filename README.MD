![main](https://i.imgur.com/4tgFZRJ.jpg)

# coinLED
coinLED is a sleek desktop crypto price tracker powered with an USB-C. It's my first bigger project, even though it works, it lacks polish in terms of parts choice and code quality.
This repo serves as an ** open archive of an closed-source ** project developed in the beginning of 2021.

## Features

The display allows for viewing up to 6 different crypto at once. Supports Binance and CoinGecko price API updating around once per minute.
Upon powering up for the first time, it sets up an WiFi Access point where you can enter network details for further connections. 
The device features a config portal running on the local network, an IP is displayed upon pressing a special button.

## Electrical Design
![pcb](https://i.imgur.com/e0q99V5.png)

Design is based around two ESP-32 MCU's wired with I2C, the first being used for wireless communication, second for driving a HUB75 64x64 LED matrix. They're powered via an USB-C port with an 3.3V LDO.


## Mechanical Design
The frame is 3D-printed out of black PETG with an addition of plexiglass covering the display. 


## Software design
Designed to be used with PlatformIO IDE. This repo features two build environments, each for one of the ESP32's. In addition to uploading software, the NET ESP32 requires building and uploading filesystem image, where the static website contents are hosted.