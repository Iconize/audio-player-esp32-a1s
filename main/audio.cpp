#include "audio.hpp"

namespace Audio {
    audio_pipeline_handle_t pipeline = NULL;
    audio_element_handle_t i2s_stream_writer = NULL;
    audio_element_handle_t mp3_decoder = NULL;
    audio_element_handle_t wav_decoder = NULL;
    audio_element_handle_t fatfs_stream_reader = NULL;
    playlist_operator_handle_t sdcard_list_handle = NULL;
}

// Add helper function to get file extension
const char* Audio::get_file_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

// Add function to select appropriate decoder
audio_element_handle_t Audio::select_decoder(const char* url) {
    const char* ext = get_file_extension(url);
    if (strcasecmp(ext, "mp3") == 0) {
        return mp3_decoder;
    } else if (strcasecmp(ext, "wav") == 0) {
        return wav_decoder;
    }
    return NULL;
}


esp_err_t Audio::pipeline_play(){
    if(audio_element_get_state(i2s_stream_writer) == AEL_STATE_RUNNING){
        audio_pipeline_pause(pipeline);
        ESP_LOGI(TAG, "Pipeline paused");
        return ESP_OK;
    }
    else if(audio_element_get_state(i2s_stream_writer) == AEL_STATE_PAUSED){
        audio_pipeline_resume(pipeline);
        ESP_LOGI(TAG, "Pipeline resumed");
        return ESP_OK;
    }
    else if (audio_element_get_state(i2s_stream_writer) == AEL_STATE_INIT) {
        audio_pipeline_run(pipeline);
        ESP_LOGI(TAG, "Pipeline started");
        pipeline_show_song();
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t Audio::pipeline_pause(){
    if(audio_element_get_state(i2s_stream_writer) == AEL_STATE_RUNNING){
        audio_pipeline_pause(pipeline);
        ESP_LOGI(TAG, "Pipeline paused");
        return ESP_OK;
    } 
    else if(audio_element_get_state(i2s_stream_writer) == AEL_STATE_PAUSED){
        audio_pipeline_stop(pipeline);
        ESP_LOGI(TAG, "Pipeline stopped");
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t Audio::pipeline_load(char *url) {
    
    // Stop and cleanup current playback
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    
    ESP_LOGW(TAG, "URL: %s", url);
    
    // Get appropriate decoder
    audio_element_handle_t decoder = select_decoder(url);
    if (!decoder) {
        ESP_LOGE(TAG, "Unsupported file format");
        return ESP_FAIL;
    }
    
    // Reset elements before reconfiguring
    audio_element_reset_state(fatfs_stream_reader);
    audio_element_reset_state(mp3_decoder);
    audio_element_reset_state(wav_decoder);
    audio_element_reset_state(i2s_stream_writer);
    
    // Reconfigure pipeline links
    audio_pipeline_unlink(pipeline);
    const char *link_tag[3];
    link_tag[0] = "file";
    link_tag[1] = (decoder == mp3_decoder) ? "mp3" : "wav";
    link_tag[2] = "i2s";
    
    esp_err_t link_err = audio_pipeline_link(pipeline, &link_tag[0], 3);
    if (link_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to link pipeline elements");
        return ESP_FAIL;
    }
    
    // Set new URI and reset pipeline
    audio_element_set_uri(fatfs_stream_reader, url);
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    
    // Run pipeline with error checking
    esp_err_t run_err = audio_pipeline_run(pipeline);
    if (run_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to run pipeline");
        return ESP_FAIL;
    }
    
    pipeline_show_song();
    return ESP_OK;
}

esp_err_t Audio::pipeline_next() {
    char *url = NULL;
    
    esp_err_t err = sdcard_list_next(sdcard_list_handle, 1, &url);
    if (err != ESP_OK || url == NULL) {
        ESP_LOGE(TAG, "Failed to get next song URL");
        return ESP_FAIL;
    }
    
    return pipeline_load(url);
}

esp_err_t Audio::pipeline_previous(){
    char *url = NULL; // Initialized here
    
    // Check if the URL is successfully retrieved
    esp_err_t err = sdcard_list_prev(sdcard_list_handle, 1, &url);
    if (err != ESP_OK || url == NULL) {
        ESP_LOGE(TAG, "Failed to get previous song URL");
        return ESP_FAIL;
    }

    return pipeline_load(url);
}

esp_err_t Audio::pipeline_show_song(){
    char *previous = NULL;
    char *current = NULL;
    char *next = NULL;
    char *buf = NULL;

    
    esp_err_t ret = ESP_OK;

    auto strip_path = [](const char *file_path) -> const char* {
        const char *filename = strrchr(file_path, '/');
        return filename ? filename + 1 : file_path;
    };

    if(sdcard_list_current(sdcard_list_handle, &current) == ESP_FAIL){
        current = strdup(""); // Allocate an empty string
        ret = ESP_FAIL;
    } else {
        current = strdup(strip_path(current));
    }

    if(sdcard_list_next(sdcard_list_handle, 1, &next) == ESP_FAIL){ // skipping from previous to current
        next = strdup(""); // Allocate an empty string
        ret = ESP_FAIL;
    } else {
        next = strdup(strip_path(next));
    }

    if(sdcard_list_prev(sdcard_list_handle, 2, &previous) == ESP_FAIL){
        previous = strdup(""); // Allocate an empty string
        ret = ESP_FAIL;
    } else {
        previous = strdup(strip_path(previous));
    }


    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "previous", previous);
    cJSON_AddStringToObject(json, "current", current);
    cJSON_AddStringToObject(json, "next", next);

    char *json_str = cJSON_PrintUnformatted(json);

    if(sdcard_list_next(sdcard_list_handle, 1, &buf)){ // go to the original position
        free(buf);
    }

    Wifi::send_websocket_message(websocket_req_handle, json_str);

    cJSON_Delete(json);  // Free the JSON object
    free(json_str);      // Free the JSON string
    free(previous);
    free(current);
    free(next);

    return ret;
}

static esp_err_t Audio::input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    /* Handle touch pad events to start, pause, resume, finish current song, and adjust volume */
    audio_board_handle_t board_handle = (audio_board_handle_t) ctx;
    int player_volume;
    audio_hal_get_volume(board_handle->audio_hal, &player_volume);

    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_VOLUP: {
                ESP_LOGI(TAG, "[ * ] [Vol+] input key event");
                player_volume += 10;
                if (player_volume > 100) {
                    player_volume = 100;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
            }
            case INPUT_KEY_USER_ID_VOLDOWN: {
                ESP_LOGI(TAG, "[ * ] [Vol-] input key event");
                player_volume -= 10;
                if (player_volume < 0) {
                    player_volume = 0;
                }
                audio_hal_set_volume(board_handle->audio_hal, player_volume);
                ESP_LOGI(TAG, "[ * ] Volume set to %d %%", player_volume);
                break;
            }
            case 4: {
                ESP_LOGI(TAG, "[ * ] [Prev] input key event");
                pipeline_previous();
                break;
            }
            case 2: {
                ESP_LOGI(TAG, "[ * ] [Next] input key event");
                pipeline_next();
                break;
            }
            case 3: {
                ESP_LOGI(TAG, "[ * ] [Play] input key event");
                pipeline_play();
                break;
            }
            case 1: {
                ESP_LOGI(TAG, "[ * ] [Stop] input key event");
                pipeline_pause();
                break;
            }
        }
    }

    return ESP_OK;
}

void Audio::sdcard_url_save_cb(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

void Audio::init(){
    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);
    audio_board_sdcard_init(set, SD_MODE_1_LINE);

    ESP_LOGI(TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);

    const char *file_types[] = {"wav", "mp3"};

    for (int i = 0; i < sizeof(file_types)/sizeof(file_types[0]); i++) {
        sdcard_scan(sdcard_url_save_cb, "/sdcard", 1, &file_types[i], 1, sdcard_list_handle);
    }
    sdcard_list_show(sdcard_list_handle);

    ESP_LOGI(TAG, "[1.3] Create fatfs stream to read data from sdcard");
    char *url = NULL;

    if (sdcard_list_current(sdcard_list_handle, &url) == ESP_FAIL){
        ESP_LOGE(TAG, "No files found on SD card");
        return;
    }

    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
    audio_element_set_uri(fatfs_stream_reader, url);

    ESP_LOGI(TAG, "[2.0] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[2.1] Initialize ES8388");

    // Basic initialization
    es8388_write_reg(ES8388_MASTERMODE, 0x00);      // Slave mode
    es8388_write_reg(ES8388_CHIPPOWER, 0x00);       // Power up all systems

    // // DAC settings
    es8388_write_reg(ES8388_DACCONTROL1, 0x18);     // I2S format, 16-bit
    es8388_write_reg(ES8388_DACCONTROL2, 0x02);     // Normal operation
    es8388_write_reg(ES8388_DACCONTROL3, 0x00);     // Unmute DAC, no digital boost
    
    // // Clear DAC volume attenuation for both channels
    es8388_write_reg(ES8388_DACCONTROL4, 0x00);     // Left DAC volume
    es8388_write_reg(ES8388_DACCONTROL5, 0x00);     // Right DAC volume

    es8388_write_reg(ES8388_DACCONTROL24, 0x21);  // LOUT1 volume
    es8388_write_reg(ES8388_DACCONTROL25, 0x21);  // ROUT1 volume
    es8388_write_reg(ES8388_DACCONTROL26, 0x21);  // LOUT2 volume
    es8388_write_reg(ES8388_DACCONTROL27, 0x21);  // ROUT2 volume

    audio_hal_set_volume(board_handle->audio_hal, 50);

    ESP_LOGI(TAG, "[3.0] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);

    ESP_LOGI(TAG, "[4.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline_cfg.rb_size = 2048;
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[4.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    i2s_stream_set_clk(i2s_stream_writer, 48000, 16, 2);

    ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    ESP_LOGI(TAG, "[4.3] Create wav decoder");
    wav_decoder_cfg_t  wav_dec_cfg  = DEFAULT_WAV_DECODER_CONFIG();
    wav_decoder = wav_decoder_init(&wav_dec_cfg);

    ESP_LOGI(TAG, "[4.4] Select initial decoder");
    audio_element_handle_t initial_decoder = select_decoder(url);

    ESP_LOGI(TAG, "[4.5] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline, wav_decoder, "wav");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    if (!initial_decoder) {
        ESP_LOGE(TAG, "Unsupported initial file format");
        return;
    }

    ESP_LOGI(TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->decoder-->i2s_stream-->[codec_chip]");
    const char *link_tag[3] = {"file", 
                                (initial_decoder == mp3_decoder) ? "mp3" : "wav",  
                                "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.1] Listen for all pipeline events");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGW(TAG, "[6.0] Press the keys to control music player:");
    ESP_LOGW(TAG, "      [Vol-] or [Vol+] to adjust volume.");

    const int MAX_RETRY_COUNT = 3;
    int error_count = 0;
    
    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        
        if (ret != ESP_OK) {
            error_count++;
            ESP_LOGE(TAG, "[ * ] Event interface error : %d (attempt %d/%d)", 
                    ret, error_count, MAX_RETRY_COUNT);
            
            if (error_count >= MAX_RETRY_COUNT) {
                ESP_LOGE(TAG, "[ * ] Too many event interface errors, reinitializing...");
                
                // Clean up existing event interface
                audio_pipeline_remove_listener(pipeline);
                audio_event_iface_destroy(evt);
                
                // Reinitialize event interface
                audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
                evt = audio_event_iface_init(&evt_cfg);
                if (evt == NULL) {
                    ESP_LOGE(TAG, "[ * ] Failed to reinitialize event interface");
                    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait before retrying
                    continue;
                }
                
                audio_pipeline_set_listener(pipeline, evt);
                error_count = 0;
            }
            
            // Add delay before retry to prevent tight loop
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        // Reset error count on successful event
        error_count = 0;
        
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) {
            // Handle music info from either decoder
            if ((msg.source == (void *)mp3_decoder || msg.source == (void *)wav_decoder)
                && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
                audio_element_info_t music_info = {0};
                audio_element_getinfo((audio_element_handle_t)msg.source, &music_info);
                ESP_LOGI(TAG, "[ * ] Received music info from decoder, sample_rates=%d, bits=%d, ch=%d",
                         music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer, &music_info);
                i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, 
                                 music_info.bits, music_info.channels);
                continue;
            }
        }
        
        // Add a small delay to prevent watchdog timeout
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    ESP_LOGI(TAG, "[7.0] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, mp3_decoder);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    //audio_pipeline_unregister(pipeline, rsp_handle);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    sdcard_list_destroy(sdcard_list_handle);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(mp3_decoder);
    audio_element_deinit(wav_decoder);
    periph_service_destroy(input_ser);
    esp_periph_set_destroy(set);
}

