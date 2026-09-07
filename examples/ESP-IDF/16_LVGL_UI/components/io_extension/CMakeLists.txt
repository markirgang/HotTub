idf_component_register(SRCS "io_extension.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver i2c gpio esp_driver_i2c esp_driver_gpio
                    )
