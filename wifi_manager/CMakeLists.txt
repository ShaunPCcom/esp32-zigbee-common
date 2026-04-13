if(NOT "${IDF_TARGET}" STREQUAL "esp32c6")
    idf_component_register(INCLUDE_DIRS "include")
    return()
endif()

idf_component_register(
    SRCS "src/wifi_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_netif esp_event espressif__mdns nvs_flash lwip
    PRIV_REQUIRES esp_coex
)
