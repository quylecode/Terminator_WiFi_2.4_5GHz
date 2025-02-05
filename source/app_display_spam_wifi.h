#ifndef APP_DISPLAY_SPAM_WIFI_H
#define APP_DISPLAY_SPAM_WIFI_H

#include <Arduino.h>
#include <lvgl.h>
#include "WiFi.h"
#include "wifi_conf.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "wifi_cust_tx.h"

uint8_t app_display_spam_wifi_get_active(void);
void app_display_spam_wifi_build(void);
void app_display_spam_wifi_on(void);
void app_display_spam_wifi_off(void);
void wifi_spam_set_spam_style(uint8_t style);
void wifi_spam_set_spam_number(uint8_t number);
void wifi_spam_set_length_name_wifi(uint8_t length);

#endif