idf_component_register(
    SRCS
        "board_init.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        esp_common
        esp_system
        nvs_flash
)