idf_component_register(
    SRCS "src/zigbee_core.cpp"
         "src/zigbee_button.cpp"
         "src/zgp_stub.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_zigbee_lib nvs_flash driver
)
