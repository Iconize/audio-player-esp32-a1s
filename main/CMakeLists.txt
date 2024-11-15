idf_component_register(
    SRCS "main.cpp" "wifi.cpp" "audio.cpp" "../components/esp32-wifi-manager/src/http_app.c" "cJSON.c" ${COMPONENT_SRCS}
    INCLUDE_DIRS "." "../include" ${COMPONENT_ADD_INCLUDEDIRS}

)

set(COMPONENT_ADD_INCLUDEDIRS "." "../include")
