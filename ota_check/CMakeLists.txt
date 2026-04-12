idf_component_register(
    SRCS "src/ota_check.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_http_client esp_crt_bundle freertos cJSON zigbee_ota
)
