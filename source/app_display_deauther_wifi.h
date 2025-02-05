#ifndef APP_DISPLAY_DEAUTHER_WIFI_H
#define APP_DISPLAY_DEAUTHER_WIFI_H

#include <Arduino.h>
#include <lvgl.h>
#include "app_display_scan_wifi.h"
#include "app_display_settings.h"
#include "vector"
#include "WiFi.h"
#include "wifi_conf.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "wifi_cust_tx.h"

struct config_wifi_deauther {
  /* WiFi Deauther */
  uint8_t deauth_reason;
  uint8_t frames_per_deauth;
  uint8_t num_wifi_deauth;
  uint8_t num_wifi_list;
  /* */
};
typedef config_wifi_deauther config_wifi_deauther_t;

uint8_t app_display_deauther_wifi_get_active(void);
void app_display_deauther_wifi_build(void);
void app_display_deauther_wifi_on(void);
void app_display_deauther_wifi_off(void);

void deauther_wifi_set_deauth_reason(uint8_t deauth_reason);
void deauther_wifi_set_frames_per_deauth(uint8_t frames_per_deauth);
void deauther_wifi_set_num_wifi_deauth(uint8_t num_wifi_deauth);
void deauther_wifi_set_num_wifi_list(uint8_t num_wifi_list);

#endif
