idf_component_register(SRCS "can.c" 
                        INCLUDE_DIRS "." "../i2c" "../gpio" "../io_extension"
                        REQUIRES driver esp_driver_twai esp_driver_gpio esp_driver_i2c i2c io_extension gpio
                    )
