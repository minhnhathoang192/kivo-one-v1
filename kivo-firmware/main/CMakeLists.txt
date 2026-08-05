idf_component_register(
    SRCS
        "app_main.c"

    REQUIRES
        kivo_common
        kivo_board
        kivo_diagnostics
        freertos
        esp_common
        esp_system
)