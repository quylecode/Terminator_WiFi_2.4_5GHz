#ifndef APP_DISPLAY_SETTINGS_H
#define APP_DISPLAY_SETTINGS_H

#include <Arduino.h>
#include <lvgl.h>
#include "app_display_deauther_wifi.h"
#include "app_display_spam_wifi.h"
#include "app_display_scan_wifi.h"
#include "app_display_lock.h"
#include "app_display_main.h"

uint8_t app_display_settings_get_active(void);
void app_display_settings_load_config(void);
void app_display_settings_build(void);
void app_display_settings_on(void);
void app_display_settings_off(void);

#endif
