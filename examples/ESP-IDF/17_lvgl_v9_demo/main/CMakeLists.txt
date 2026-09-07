idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_lvgl_adapter rgb_lcd_port lvgl
    WHOLE_ARCHIVE
    )
    
if(TARGET __idf_lvgl__lvgl)
    target_compile_options(__idf_lvgl__lvgl PRIVATE -Wno-format) 
endif() 