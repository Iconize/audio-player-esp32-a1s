# esp32-a1s-audio-player

## Environment set up 
### Install ESP-IDF v5.3

https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#get-started-setup-esp-idf

1. inside Step 1 click ont the ```ESP32``` link
2. scroll down and click on the ```Windows Installer```
3. click on ```Windows Installer Download```
4. select the ```Uiversal Online Installer```
5. run the installer
6. select ```Download ESP-IDF```
7. select ```v5.3.1```
8. leave the default install folder as is.
9. select ```full installation```
10. now hope that the internet in the office is fast enough (it's a 50/50 change)

### Install ESP-ADF
ESP-ADF is ESP's audio development framework. 

1. Install ```git``` if you haven't got it yet: https://github.com/git-guides/install-git
2. Install ESP-ADF (Step 2): https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-2-get-esp-adf
3. Set up your environment (Step 3): https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-3-set-up-the-environment

(run the ```export.bat``` within the esp-idf terminal)
 
### Adding the custom ESP-32-A1S audio board to adf

1. clone or download the .zip https://github.com/trombik/esp-adf-component-ai-thinker-esp32-a1s
2. copy the ```esp-adf-component-ai-thinker-esp32-a1s\components\ai-thinker-esp32-a1s``` folder to ```esp\esp-adf\components\audio_board```
3. navigate to: ```{PATH TO GITHUB FOLDER}\esp32-a1s-audio-player```
4. run:
```bash
idf.py menuconfig
```
5. Go to ```(Top) → Audio HAL → Audio board → Custom audio board```. If the board hase been detected you should see ```Custom Audio Board``` if not; make a symbolic to the folder in ```esp\esp-adf\components\audio_board``` first.
6. Now select the correct board to use: ```(Top) → Custom Audio Board → Select a custom audio board (AI Thinker...) → Ai Thinker ESP32 A1S Audio Kit (ES8388, variant 5)```
7. press S to save, press escape, and Q to quit the menuconfig.


try to build and flash the project:
```bash
idf.py build flash monitor
```

connect with your device through wifi to ```Music Player``` password: ```password123```

if not redirected automatically go to this ip in your browser.
```bash
http://10.10.0.1/
```
