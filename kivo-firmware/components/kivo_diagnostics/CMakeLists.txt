idf_component_register(
    SRCS
        "diagnostics.c"

    INCLUDE_DIRS
        "include"

    REQUIRES
        kivo_common
        esp_common
        esp_system
        esp_app_format
        heap
        spi_flash
)