idf_component_register(
    SRCS "src/board_led.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_rmt esp_timer
)
