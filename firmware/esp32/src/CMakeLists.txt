idf_component_register(
    SRCS "main.c"
    REQUIRES
        matrix_display
        hub75_output
        esp_driver_uart
        esp_timer
)
