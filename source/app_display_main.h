#ifndef APP_DISPLAY_MAIN_H
#define APP_DISPLAY_MAIN_H

#include <Arduino.h>
#include <lvgl.h>
#include "WDT.h"
#include "map"
#include "app_display_scan_wifi.h"
#include "app_display_deauther_wifi.h"
#include "app_display_spam_wifi.h"
#include "app_display_settings.h"

void app_display_main_build(void);
void app_display_main_on(void);
void app_display_main_home(void);
uint8_t get_ram_usage(void);
void main_set_show_fps_ram(uint8_t show);

#endif