#ifndef APP_DISPLAY_SCAN_WIFI_H
#define APP_DISPLAY_SCAN_WIFI_H

#include <Arduino.h>
#include "stdio.h"
#include "vector"

typedef struct {
  String ssid;
  String bssid_str;
  uint8_t bssid[6];
  short rssi;
  uint8_t channel;
} wifi_scan_result_t;

uint8_t app_display_scan_wifi_get_active(void);
void app_display_scan_wifi_build(void);
void app_display_scan_wifi_on(void);
void app_display_scan_wifi_off(void);
bool app_display_scan_wifi_get_state(void);
void app_display_scan_wifi_get_results(std::vector<wifi_scan_result_t> &scan_results);
void wifi_scan_set_number_wifi_load(uint8_t number);

#endif
