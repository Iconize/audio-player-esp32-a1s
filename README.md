# esp32-a1s-audio-player

This repo describes a 
## Code build environment set up

### Install ESP-ADF
ESP-ADF is ESP's audio development framework. 

1. Install ```git``` if you haven't got it yet: https://github.com/git-guides/install-git
2. Install ESP-ADF (Step 2): https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#quick-start
3. Set up your environment (Step 3): https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-3-set-up-the-environment
 
### Using the ESP-32-A1S audio board
We use the ai-thinker-esp32-a1s PCB board [https://www.tinytronics.nl/en/development-boards/microcontroller-boards/with-wi-fi/ai-thinker-esp32-audio-kit-with-wi-fi-and-bluetooth] with E8388 audio processor. https://docs.ai-thinker.com/en/esp32-audio-kit 

NB: An overview of the different ai-thinker-esp32-a1s board variants can be found here: https://github.com/pschatzmann/arduino-audiokit/wiki/GPIO-Overview-by-Selected-Board 


Optional: To add the specific `ai_thinker (ES8388) 2957 3478` board to your ESP-ADF setup, do the following:
1. clone or download the .zip from https://github.com/trombik/esp-adf-component-ai-thinker-esp32-a1s
2. copy the ```esp-adf-component-ai-thinker-esp32-a1s\components\ai-thinker-esp32-a1s``` folder to ```esp\esp-adf\components\audio_board```

Check if your project has the correct board configured in the `menuconfig` settings file:
1. navigate to: ```{PATH TO THIS GITHUB FOLDER}\esp32-a1s-audio-player```
2. run:
```bash
idf.py menuconfig
```
3. Go to ```(Top) → Audio HAL → Audio board → Custom audio board```. If the board hase been detected you should see ```Custom Audio Board``` if not; make a symbolic to the folder in ```esp\esp-adf\components\audio_board``` first.
4. Now select the correct board to use: ```(Top) → Custom Audio Board → Select a custom audio board (AI Thinker...) → Ai Thinker ESP32 A1S Audio Kit (ES8388, variant 5)```
5. press S to save, press escape, and Q to quit the menuconfig.

## Build and check your code
To build the code, and check for errors, run:
```bash
idf.py build
```

## Building check & flash code to the board
Connect the UART micro USB port of the AI thinker ESP32 A1S board (next to the SD card slot) to a USB port on your computer.
Then run:
```bash
idf.py build flash monitor
```

## Playing music
Connect with your AI thinker ESP32 A1S board through WiFi

1. Power your AI thinker ESP32 A1S board
2. On any WiFi capable device, connect with the WiFi network that the board exposes ```Music Player``` password: ```playmusic``` (or not required)

if not redirected automatically go to this ip in your browser.
```bash
http://10.10.0.1/
```
