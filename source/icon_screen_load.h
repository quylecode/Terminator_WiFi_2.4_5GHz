#ifndef ICON_SCREEN_LOAD_H
#define ICON_SCREEN_LOAD_H

#include <Arduino.h>

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

extern const lv_img_dsc_t screen_load_0;
extern const lv_img_dsc_t screen_load_1;
extern const lv_img_dsc_t screen_load_2;
extern const lv_img_dsc_t screen_load_3;
extern const lv_img_dsc_t screen_load_4;
extern const lv_img_dsc_t screen_load_5;

#endif
