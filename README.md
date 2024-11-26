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

### install ESP-ADF

Now do step 2 of the guide
https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-2-get-esp-adf

install ```git``` if you haven't

Follow step 3 of the guide
https://docs.espressif.com/projects/esp-adf/en/latest/get-started/index.html#step-3-set-up-the-environment

(run the ```export.bat``` within the esp-idf terminal)
 
### Adding the custom ESP-32-A1S audio board to adf

1. clone or download the .zip https://github.com/trombik/esp-adf-component-ai-thinker-esp32-a1s
   put the ```esp-adf-component-ai-thinker-esp32-a1s``` folder inside ```esp\esp-adf\components\audio_board```
3. open the pipeline_play_sdcard_music folder and run the symbolic_link_maker 2.bat
4. open ESP-IDF and navigate to: ```{PATH TO GITHUB FOLDER}\esp32-a1s-audio-player-exercise\Pipeline_sdcard_mp3_control```
5. run:
```bash
idf.py menuconfig
```
6. Select: ```Audio HAL -> Audio board -> Custom audio board```

if symbolic_link_maker worked you should see ```Custom Audio Board``` if not exit and run symbolic_link_maker.bat first

7. Select: ```Custom Audio Board -> Select a custom audio board (AI Thinker...) -> Ai Thinker ESP32 A1S Audio Kit (ES8388, variant 5)```

8. Select: ```Custom Audio Board -> Use on-board SD card```

9. press escape a couple of times and ```y``` to apply the changes.

### Change the get_i2s_function
change the ```get_i2s_function``` in ```ai_thinker_esp32_a1s/board_pins_config.c```
from:
```c++
esp_err_t get_i2s_pins(i2s_port_t port, board_i2s_pin_t *i2s_config)
{
    AUDIO_NULL_CHECK(TAG, i2s_config, return ESP_FAIL);
    if (port == I2S_NUM_0) {
        i2s_config->mck_io_num = GPIO_NUM_0;
#if defined(CONFIG_AI_THINKER_ESP32_A1S_ES8388_VARIANT_7)
        i2s_config->bck_io_num = GPIO_NUM_5;
#elif defined(CONFIG_AI_THINKER_ESP32_A1S_ES8388_VARIANT_5)
        i2s_config->bck_io_num = GPIO_NUM_27;
#endif
        i2s_config->ws_io_num = GPIO_NUM_25;
        i2s_config->data_out_num = GPIO_NUM_26;
        i2s_config->data_in_num = GPIO_NUM_35;
    } else if (port == I2S_NUM_1) {
        i2s_config->bck_io_num = -1;
        i2s_config->ws_io_num = -1;
        i2s_config->data_out_num = -1;
        i2s_config->data_in_num = -1;
    } else {
        memset(i2s_config, -1, sizeof(board_i2s_pin_t));
        ESP_LOGE(TAG, "i2s port %d is not supported", port);
        return ESP_FAIL;
    }

    return ESP_OK;
}
````
to:
```c++
esp_err_t get_i2s_pins(int port, board_i2s_pin_t *i2s_config)
{
    AUDIO_NULL_CHECK(TAG, i2s_config, return ESP_FAIL);
    if (port == 0) {
        i2s_config->mck_io_num = GPIO_NUM_0;
#if defined(CONFIG_AI_THINKER_ESP32_A1S_ES8388_VARIANT_7)
        i2s_config->bck_io_num = GPIO_NUM_5;
#elif defined(CONFIG_AI_THINKER_ESP32_A1S_ES8388_VARIANT_5)
        i2s_config->bck_io_num = GPIO_NUM_27;
#endif
        i2s_config->ws_io_num = GPIO_NUM_25;
        i2s_config->data_out_num = GPIO_NUM_26;
        i2s_config->data_in_num = GPIO_NUM_35;
    } else if (port == 1) {
        i2s_config->bck_io_num = -1;
        i2s_config->ws_io_num = -1;
        i2s_config->data_out_num = -1;
        i2s_config->data_in_num = -1;
    } else {
        memset(i2s_config, -1, sizeof(board_i2s_pin_t));
        ESP_LOGE(TAG, "i2s port %d is not supported", port);
        return ESP_FAIL;
    }

    return ESP_OK;
}
```


try to build and flash the project:
```bash
idf.py build flash monitor
```

connect with your device through wifi to ```Music Player``` password: ```password123```

if not redirected automatically go to this ip in your browser.
```bash
http://10.10.0.1/
```
