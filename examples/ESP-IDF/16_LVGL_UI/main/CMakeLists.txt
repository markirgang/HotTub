file(GLOB_RECURSE srcs *.c
                   ui/*.c
                   ui/components/*.c
                   ui/screens/*.c
                   user/add_password/*.c
                   user/wifi/*.c
                   user/rs485/*.c
                   user/can/*.c
                   user/pwm/*.c
                   user/sd_card/*.c
                   )
set(include_dirs 
   . 
   ui
   user/add_password
   user/wifi
   user/rs485
   user/can
   user/pwm
   user/sd_card
   )
idf_component_register( SRCS ${srcs}
                        INCLUDE_DIRS ${include_dirs}
                        REQUIRES mbedtls driver
                        )
if(TARGET __idf_lvgl__lvgl)
    target_compile_options(__idf_lvgl__lvgl PRIVATE -Wno-format)
endif()