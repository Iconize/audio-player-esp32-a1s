#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "wav_decoder.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"

#include "sdcard_list.h"
#include "sdcard_scan.h"
#include "cJSON.h"

#include "es8388.h"

#include "wifi.hpp"

static const char *TAG = "AUDIO";

namespace Audio {

    extern audio_pipeline_handle_t pipeline;
    extern audio_element_handle_t i2s_stream_writer, mp3_decoder, wav_decoder, fatfs_stream_reader;
    extern playlist_operator_handle_t sdcard_list_handle;

    const char* get_file_extension(const char* filename);
    audio_element_handle_t select_decoder(const char* url);

    esp_err_t pipeline_show_song();
    esp_err_t pipeline_play();
    esp_err_t pipeline_pause();
    esp_err_t pipeline_next();
    esp_err_t pipeline_previous();
    esp_err_t pipeline_load(char *url=NULL);

    static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);

    void init();

    // Callback functions
    void sdcard_url_save_cb(void *user_data, char *url);

}

#endif // AUDIO_HPP
