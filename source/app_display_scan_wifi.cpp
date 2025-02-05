#include "app_display_scan_wifi.h"
#include <lvgl.h>
#include "WiFi.h"
#include "wifi_conf.h"
#include "wifi_util.h"
#include "wifi_structures.h"

typedef enum {
  NONE,
  NETWORK_SEARCHING,
  NETWORK_CONNECTED_POPUP,
  NETWORK_CONNECTED,
  NETWORK_CONNECT_FAILED
} network_status_t;
network_status_t network_status = NONE;

static uint8_t wifi_scan_init = 0;
static uint8_t number_wifi_scan = 0;

static std::vector<wifi_scan_result_t> wifi_scan_results;
static std::vector<wifi_scan_result_t> wifi_scan_results_last;

static TaskHandle_t wifi_scan_task_handler = NULL;

static lv_obj_t *wifi_scan_main;
static lv_obj_t *wifi_scan_label;
static lv_obj_t *wifi_scan_close_btn;
static lv_obj_t *wifi_scan_switch;
static lv_obj_t *wifi_scan_reload;
static lv_obj_t *wifi_scan_list;
static lv_obj_t *wifi_scan_spinner_load;

static lv_timer_t *wifi_scan_timer;

static uint8_t wifi_scan_flag = 0;
static uint8_t wifi_scan_have_wifi_flag = 0;
static uint8_t wifi_scan_found_networks = 0;

static void app_display_scan_wifi_show_list_found(void);
static void app_display_scan_wifi_show_list_avaible(void);
static void wifi_scan_network_create_task(void);
static void wifi_scan_btn_event_cb(lv_event_t *e);
static void wifi_scan_task(void *pvParameters);
static void timer_for_network(lv_timer_t *timer);

rtw_result_t wifi_scan_result_handler(rtw_scan_handler_result_t *scan_result) {
  rtw_scan_result_t *record;
  if (scan_result->scan_complete == 0) {
    record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;
    wifi_scan_result_t result;
    result.ssid = String((const char *)record->SSID.val);
    result.channel = record->channel;
    result.rssi = record->signal_strength;
    memcpy(&result.bssid, &record->BSSID, 6);
    char bssid_str[] = "XX:XX:XX:XX:XX:XX";
    snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", result.bssid[0], result.bssid[1], result.bssid[2], result.bssid[3], result.bssid[4], result.bssid[5]);
    result.bssid_str = bssid_str;
    wifi_scan_results.push_back(result);
  }
  return RTW_SUCCESS;
}

uint8_t app_display_scan_wifi_get_active(void) {
  if (wifi_scan_init == 1) {
    return lv_obj_has_flag(wifi_scan_main, LV_OBJ_FLAG_HIDDEN) ? 0 : 1;
  } else {
    return 1;
  }
}

void app_display_scan_wifi_on(void) {
  lv_obj_clear_flag(wifi_scan_main, LV_OBJ_FLAG_HIDDEN);
}

void app_display_scan_wifi_off(void) {
  if (wifi_scan_init == 1) {
    lv_obj_del_async(wifi_scan_main);
    wifi_scan_init = 0;
    /* wifi_scan_flag = 0; */
  }
}

void app_display_scan_wifi_build(void) {
  if (wifi_scan_init == 1) {
    if (wifi_scan_flag == 1) {
      app_display_scan_wifi_show_list_avaible();
    }
    return;
  }
  wifi_scan_init = 1;

  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  wifi_scan_main = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(wifi_scan_main, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(wifi_scan_main, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(wifi_scan_main, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(wifi_scan_main, &style_background, 0);

  wifi_scan_label = lv_label_create(wifi_scan_main);
  lv_label_set_text(wifi_scan_label, LV_SYMBOL_WIFI " WiFi Scan");
  lv_obj_align(wifi_scan_label, LV_ALIGN_TOP_LEFT, 0, 0);

  wifi_scan_close_btn = lv_btn_create(wifi_scan_main);
  lv_obj_set_size(wifi_scan_close_btn, 30, 30);
  lv_obj_align(wifi_scan_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(wifi_scan_close_btn, wifi_scan_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol = lv_label_create(wifi_scan_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  wifi_scan_switch = lv_switch_create(wifi_scan_main);
  lv_obj_add_event_cb(wifi_scan_switch, wifi_scan_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_size(wifi_scan_switch, 35, 20);
  lv_obj_align_to(wifi_scan_switch, wifi_scan_label, LV_ALIGN_RIGHT_MID, 40, 0);

  wifi_scan_reload = lv_btn_create(wifi_scan_main);
  lv_obj_add_event_cb(wifi_scan_reload, wifi_scan_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_size(wifi_scan_reload, 25, 25);
  lv_obj_align_to(wifi_scan_reload, wifi_scan_switch, LV_ALIGN_RIGHT_MID, 32, 0);
  lv_obj_add_state(wifi_scan_reload, LV_STATE_DISABLED);

  lv_obj_t *btn_refresh_symbol = lv_label_create(wifi_scan_reload); /*Add a label to the button*/
  lv_label_set_text(btn_refresh_symbol, LV_SYMBOL_REFRESH);         /*Set the labels text*/
  lv_obj_center(btn_refresh_symbol);

  wifi_scan_list = lv_list_create(wifi_scan_main);
  lv_obj_set_size(wifi_scan_list, 290, 170);
  lv_obj_align_to(wifi_scan_list, wifi_scan_label, LV_ALIGN_TOP_LEFT, 0, 30);

  wifi_scan_spinner_load = lv_spinner_create(wifi_scan_main, 1000, 60);
  lv_obj_set_size(wifi_scan_spinner_load, 50, 50);
  lv_obj_center(wifi_scan_spinner_load);
  lv_obj_add_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);

  if (wifi_scan_flag == 1) {
    app_display_scan_wifi_show_list_avaible();
    lv_obj_add_state(wifi_scan_switch, LV_STATE_CHECKED);
    lv_obj_clear_state(wifi_scan_reload, LV_STATE_DISABLED);
  }
}

static void wifi_scan_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_scan_close_btn) {
      if (lv_obj_has_state(wifi_scan_switch, LV_STATE_CHECKED)) {
        if (wifi_scan_flag == 1)
          lv_obj_clean(wifi_scan_list);
      }
      lv_obj_add_flag(wifi_scan_main, LV_OBJ_FLAG_HIDDEN);
    } else if (btn == wifi_scan_switch) {
      if (lv_obj_has_state(btn, LV_STATE_CHECKED)) {
        lv_obj_clear_state(wifi_scan_reload, LV_STATE_DISABLED);
        if (wifi_scan_task_handler == NULL) {
          network_status = NETWORK_SEARCHING;
          wifi_scan_network_create_task();
          wifi_scan_timer = lv_timer_create(timer_for_network, 1000, NULL);
          lv_list_add_text(wifi_scan_list, "WiFi: Looking for Networks...");
          /*Create a spinner*/
          lv_obj_clear_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);
        }
      } else {
        wifi_scan_have_wifi_flag = 0;
        wifi_scan_flag = 0;
        lv_obj_add_state(wifi_scan_reload, LV_STATE_DISABLED);
        if (wifi_scan_task_handler != NULL) {
          network_status = NONE;
          vTaskDelete(wifi_scan_task_handler);
          wifi_scan_task_handler = NULL;
        }
        lv_obj_clean(wifi_scan_list);
        lv_obj_add_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);
      }
    }
    if (btn == wifi_scan_reload) {
      if (wifi_scan_task_handler == NULL) {
        network_status = NETWORK_SEARCHING;
        wifi_scan_network_create_task();
        wifi_scan_timer = lv_timer_create(timer_for_network, 1000, NULL);
        lv_obj_clean(wifi_scan_list);
        lv_list_add_text(wifi_scan_list, "WiFi: Looking for Networks...");
        /*Create a spinner*/
        lv_obj_clear_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

static void timer_for_network(lv_timer_t *timer) {
  LV_UNUSED(timer);

  switch (network_status) {
    case NETWORK_SEARCHING:
      app_display_scan_wifi_show_list_found();
      break;
    case NETWORK_CONNECTED_POPUP:
      break;
    case NETWORK_CONNECTED:
      break;
    case NETWORK_CONNECT_FAILED:
      break;
    default:
      break;
  }
}

static void app_display_scan_wifi_show_list_avaible(void) {
  lv_obj_clean(wifi_scan_list);
  lv_obj_add_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);
  lv_list_add_text(wifi_scan_list, wifi_scan_results_last.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");

  char text[50];
  for (uint8_t i = 0; i < wifi_scan_results_last.size() && i < number_wifi_scan; i++) {
    if (wifi_scan_results_last[i].ssid != NULL) {
      sprintf(text, "%s ( %d )", (char *)String(wifi_scan_results_last[i].ssid).c_str(), wifi_scan_results_last[i].channel);
      lv_list_add_btn(wifi_scan_list, LV_SYMBOL_WIFI, (char *)text);
    }
  }
}

bool isEqual(std::vector<wifi_scan_result_t> &v1, std::vector<wifi_scan_result_t> &v2) {

  // If size is different
  if (v1.size() != v2.size()) return false;

  // Checking all elements of vector equal or not
  for (uint8_t i = 0; i < v1.size(); i++) {
    if (v1[i].ssid != v2[i].ssid) return false;
  }
  return true;
}

static void app_display_scan_wifi_show_list_found(void) {
  if (wifi_scan_results.size() != 0) {
    if (wifi_scan_found_networks != wifi_scan_results.size()) {
      if (!isEqual(wifi_scan_results, wifi_scan_results_last)) {
      } else return;
    } else return;
  } else return;

  if (lv_obj_has_flag(wifi_scan_main, LV_OBJ_FLAG_HIDDEN) == false) {
    lv_obj_clean(wifi_scan_list);
    lv_obj_add_flag(wifi_scan_spinner_load, LV_OBJ_FLAG_HIDDEN);
    lv_list_add_text(wifi_scan_list, wifi_scan_results.size() > 1 ? "WiFi: Found Networks" : "WiFi: Not Found!");

    printf("----- WiFi Scan -----\r\n");
    char text[50];
    for (uint8_t i = 0; i < wifi_scan_results.size() && i < number_wifi_scan; i++) {
      if (wifi_scan_results[i].ssid != NULL) {
        sprintf(text, "%s ( %d )", (char *)String(wifi_scan_results[i].ssid).c_str(), wifi_scan_results[i].channel);
        lv_list_add_btn(wifi_scan_list, LV_SYMBOL_WIFI, (char *)text);
      }
      printf("Wifi: %s Channel: %d RSSI: %d BSSID: %02X:%02X:%02X:%02X:%02X:%02X \r\n", wifi_scan_results[i].ssid == NULL ? "Hidden Network" : (char *)String(wifi_scan_results[i].ssid).c_str(), wifi_scan_results[i].channel, wifi_scan_results[i].rssi, wifi_scan_results[i].bssid[0], wifi_scan_results[i].bssid[1], wifi_scan_results[i].bssid[2], wifi_scan_results[i].bssid[3], wifi_scan_results[i].bssid[4], wifi_scan_results[i].bssid[5]);
    }
  }
  wifi_scan_have_wifi_flag = 1;
  wifi_scan_flag = 1;
  wifi_scan_results_last = wifi_scan_results;
  wifi_scan_found_networks = wifi_scan_results.size();
  if (wifi_scan_task_handler != NULL) {
    network_status = NONE;
    vTaskDelete(wifi_scan_task_handler);
    wifi_scan_task_handler = NULL;
  }
  lv_timer_del(wifi_scan_timer);
}

static void wifi_scan_network_create_task(void) {
  xTaskCreate(wifi_scan_task,
              "wifi_scan_task",
              1024,
              NULL,
              3,
              &wifi_scan_task_handler);
}

static void wifi_scan_task(void *pvParameters) {
  uint8_t i = 0;
  while (1) {
    wifi_scan_results.clear();
    if (wifi_scan_networks(wifi_scan_result_handler, NULL) == RTW_SUCCESS) {
      Serial.print(" done!\n");
      vTaskDelay(10000);
    } else {
      Serial.print(" failed!\n");
      WiFi.disconnect();
      wifi_off();
      vTaskDelay(10000);
      wifi_on(RTW_MODE_AP);
      Serial.print(" start wifi done!\n");
      vTaskDelay(10000);
    }
  }
}

bool app_display_scan_wifi_get_state(void) {
  return wifi_scan_have_wifi_flag;
}
void app_display_scan_wifi_get_results(std::vector<wifi_scan_result_t> &scan_results) {
  scan_results = wifi_scan_results_last;
}

void wifi_scan_set_number_wifi_load(uint8_t number) {
  number_wifi_scan = number;
}
