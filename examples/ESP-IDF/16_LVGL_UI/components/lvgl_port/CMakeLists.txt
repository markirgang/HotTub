idf_component_register(SRCS "lvgl_port.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver esp_lcd gpio i2c io_extension rgb_lcd_port touch lvgl__lvgl
                        )
