#include "app_display_main.h"

WDT wdt_restart;

static TaskHandle_t time_live_task_handler = NULL;
static void time_live(void *pvParameters);

LV_IMG_DECLARE(background_main)

#define ver_res LV_HOR_RES
#define hor_res LV_VER_RES

static lv_obj_t *screen_wait_off;

static lv_obj_t *body_main;
static lv_obj_t *time_label;
static lv_obj_t *pin_label;

static lv_obj_t *msgbox_power;
static lv_obj_t *msgbox_restart;
static lv_obj_t *msgbox_clean_up_ram;
static lv_timer_t *message_timer;

static void scroll_event_cb(lv_event_t *e);
static void btn_roll_callback(lv_event_t *e);

static void build_msgbox_restart(void);
static void build_msgbox_power(void);
static void build_msgbox_information(void);

static void app_display_main_body_build(void);
static void app_display_main_status_bar_build(lv_obj_t *parent);
static void app_display_main_button_roll_build(lv_obj_t *parent);

static uint32_t time_last = 0;
static uint8_t status_click = 0;
static uint8_t screen_wait_off_init = 0;
static uint8_t show_fps_ram_active = 0;

static const char *btns[] = { "Yes", "No", "" };

/**
 * Translate the object as they scroll
 */

/* Button roll */
static lv_obj_t *cont;
static lv_obj_t *btn_power;
static lv_obj_t *btn_wifi;
static lv_obj_t *btn_deauther;
static lv_obj_t *btn_wifi_spam;
static lv_obj_t *btn_ble;
static lv_obj_t *btn_restart;
static lv_obj_t *btn_usb;
static lv_obj_t *btn_bat;
static lv_obj_t *btn_charge;
static lv_obj_t *btn_sd;
static lv_obj_t *btn_clear_ram;
static lv_obj_t *btn_settings;
static lv_obj_t *btn_information;

#define LV_USE_MONITOR_CUSTOM 1

typedef struct {
  uint32_t perf_last_time;
  uint32_t elaps_sum;
  uint32_t frame_cnt;
  uint32_t fps_sum_cnt;
  uint32_t fps_sum_all;
#if LV_USE_LABEL
  lv_obj_t *perf_label;
#endif
} perf_monitor_t;

typedef struct {
  uint32_t mem_last_time;
#if LV_USE_LABEL
  lv_obj_t *mem_label;
#endif
} mem_monitor_t;

static perf_monitor_t perf_monitor;
static mem_monitor_t mem_monitor;
static lv_disp_t *dis;

// https://imgonline.tools/invert
// https://ezgif.com/resize/ezgif-6-b8873b075b.jpg
// https://lvgl.io/tools/imageconverter
uint8_t get_ram_usage(void) {
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  return mon.used_pct;
}

void check_ram_empty(uint8_t pct) {
  if (pct > 50) {
    if (!app_display_scan_wifi_get_active()) {
      app_display_scan_wifi_off();
    }
    if (!app_display_deauther_wifi_get_active()) {
      app_display_deauther_wifi_off();
    }
    if (!app_display_spam_wifi_get_active()) {
      app_display_spam_wifi_off();
    }
    if (!app_display_settings_get_active()) {
      app_display_settings_off();
    }
  }
}

static void update_mem_usage(lv_timer_t *timer) {
  LV_UNUSED(timer);

#if LV_USE_MONITOR_CUSTOM && LV_USE_LABEL
  lv_obj_t *perf_label = perf_monitor.perf_label;
  if (perf_label == NULL) {
    perf_label = lv_label_create(lv_layer_sys());
    lv_obj_set_style_bg_opa(perf_label, LV_OPA_50, 0);
    lv_obj_set_style_bg_color(perf_label, lv_color_black(), 0);
    lv_obj_set_style_text_color(perf_label, lv_color_white(), 0);
    lv_obj_set_style_pad_top(perf_label, 3, 0);
    lv_obj_set_style_pad_bottom(perf_label, 3, 0);
    lv_obj_set_style_pad_left(perf_label, 3, 0);
    lv_obj_set_style_pad_right(perf_label, 3, 0);
    lv_obj_set_style_text_align(perf_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_text(perf_label, "?");
    lv_obj_align(perf_label, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    perf_monitor.perf_label = perf_label;
  }

  if (lv_tick_elaps(perf_monitor.perf_last_time) < 300) {
    if (get_px_num_user() > 5000) {
      perf_monitor.elaps_sum += get_elaps_user();
      perf_monitor.frame_cnt++;
    }
  } else {
    perf_monitor.perf_last_time = lv_tick_get();
    uint32_t fps_limit;
    uint32_t fps;

    dis = lv_disp_get_default();
    if (dis->refr_timer) {
      fps_limit = 1000 / dis->refr_timer->period;
    } else {
      fps_limit = 1000 / LV_DISP_DEF_REFR_PERIOD;
    }

    if (perf_monitor.elaps_sum == 0) {
      perf_monitor.elaps_sum = 1;
    }
    if (perf_monitor.frame_cnt == 0) {
      fps = fps_limit;
    } else {
      fps = (1000 * perf_monitor.frame_cnt) / perf_monitor.elaps_sum;
    }
    perf_monitor.elaps_sum = 0;
    perf_monitor.frame_cnt = 0;
    if (fps > fps_limit) {
      fps = fps_limit;
    }
    perf_monitor.fps_sum_all += fps;
    perf_monitor.fps_sum_cnt++;
    uint32_t cpu = 100 - lv_timer_get_idle();
    lv_label_set_text_fmt(perf_label, "%" LV_PRIu32 "%% CPU ", cpu);
  }
#endif

#if LV_USE_MONITOR_CUSTOM && LV_USE_LABEL
  if (lv_tick_elaps(mem_monitor.mem_last_time) > 300) {
    mem_monitor.mem_last_time = lv_tick_get();
    lv_obj_t *mem_label = mem_monitor.mem_label;
    if (mem_label == NULL) {
      mem_label = lv_label_create(lv_layer_sys());
      lv_obj_set_style_bg_opa(mem_label, LV_OPA_50, 0);
      lv_obj_set_style_bg_color(mem_label, lv_color_black(), 0);
      lv_obj_set_style_text_color(mem_label, lv_color_white(), 0);
      lv_obj_set_style_pad_top(mem_label, 3, 0);
      lv_obj_set_style_pad_bottom(mem_label, 3, 0);
      lv_obj_set_style_pad_left(mem_label, 3, 0);
      lv_obj_set_style_pad_right(mem_label, 3, 0);
      lv_label_set_text(mem_label, "?");
      lv_obj_align(mem_label, LV_ALIGN_BOTTOM_LEFT, 0, 0);
      mem_monitor.mem_label = mem_label;
    }
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    lv_label_set_text_fmt(mem_label, "RAM %d %%", mon.used_pct);
    check_ram_empty(mon.used_pct);
  }
#endif
}

static void update_status_ram(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  check_ram_empty(mon.used_pct);
}

void app_display_main_build(void) {
  app_display_main_body_build();
  app_display_main_button_roll_build(body_main);
  app_display_main_status_bar_build(body_main);
}

void app_display_main_on(void) {
  lv_obj_clear_flag(body_main, LV_OBJ_FLAG_HIDDEN);

  xTaskCreate(time_live,
              "time_live",
              512,
              NULL,
              6,
              &time_live_task_handler);
}

static void time_live(void *pvParameters) {
  uint8_t hours = 0;
  uint8_t minus = 0;
  uint8_t seconds = 0;
  String tx_hours;
  String tx_minus;
  String tx_seconds;
  String tx_full;
  while (1) {
    seconds++;
    if (seconds == 60) {
      minus++;
      seconds = 0;
      if (minus == 60) {
        hours++;
        minus = 0;
      }
    }
    tx_seconds = (seconds < 10) ? "0" + String(seconds) : String(seconds);
    tx_minus = (minus < 10) ? "0" + String(minus) : String(minus);
    tx_hours = (hours < 10) ? "0" + String(hours) : String(hours);
    tx_full = String(tx_hours) + ":" + String(tx_minus) + ":" + String(tx_seconds);
    lv_label_set_text(pin_label, tx_full.c_str());
    vTaskDelay(1000);
  }
}

void app_display_main_home(void) {
  /*Update the buttons position manually for first*/
  lv_event_send(cont, LV_EVENT_SCROLL, NULL);
  /*Be sure the fist button is in the middle*/
  lv_obj_scroll_to_view(lv_obj_get_child(cont, 1), LV_ANIM_OFF);
}

static void app_display_main_body_build(void) {
  static lv_style_t style_background_body;
  lv_style_init(&style_background_body);
  lv_style_set_bg_color(&style_background_body, lv_palette_main(LV_PALETTE_LIGHT_BLUE));

  body_main = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(body_main, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(body_main, ver_res, hor_res);
  lv_obj_align(body_main, LV_ALIGN_CENTER, 0, 0);

  lv_obj_clear_flag(body_main, LV_OBJ_FLAG_SNAPABLE);
  lv_obj_set_scroll_dir(body_main, LV_DIR_NONE);
  lv_obj_set_scrollbar_mode(body_main, LV_SCROLLBAR_MODE_OFF);

  lv_obj_t *img;
  img = lv_img_create(body_main);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_src(img, &background_main);

  if (show_fps_ram_active) {
    lv_timer_create(update_mem_usage, 33, NULL);
  } else {
    lv_timer_create(update_status_ram, 300, NULL);
  }
}

static void app_display_main_status_bar_build(lv_obj_t *parent) {
  time_label = lv_label_create(parent);
  lv_obj_set_size(time_label, ver_res - 50, 30);
  lv_label_set_text(time_label, "Terminator v1.1.0");
  lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 0, -6);

  pin_label = lv_label_create(parent);
  lv_obj_align(pin_label, LV_ALIGN_TOP_RIGHT, 0, -6);
  lv_label_set_text(pin_label, "100% " LV_SYMBOL_BATTERY_FULL);
}

static void app_display_main_button_roll_build(lv_obj_t *parent) {
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0x000000));
  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0xffffff));

  cont = lv_obj_create(parent);
  /* lv_obj_add_style(cont, &style_background, 0); */
  lv_obj_set_size(cont, 175 + 10, 175 + 10);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
  lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_clip_corner(cont, true, 0);
  lv_obj_set_scroll_dir(cont, LV_DIR_VER);
  lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_border_width(cont, 0, 0);

  lv_obj_set_style_bg_img_src(cont, &background_main, 0);

  btn_power = lv_btn_create(cont);
  lv_obj_set_width(btn_power, lv_pct(100));
  lv_obj_set_height(btn_power, 35);
  lv_obj_add_event_cb(btn_power, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_power = lv_label_create(btn_power);
  lv_label_set_text_fmt(label_power, LV_SYMBOL_POWER " Sleep");
  lv_obj_add_style(btn_power, &style_btn, 0);
  lv_obj_align(label_power, LV_ALIGN_LEFT_MID, 0, 0);

  btn_wifi = lv_btn_create(cont);
  lv_obj_set_width(btn_wifi, lv_pct(100));
  lv_obj_set_height(btn_wifi, 35);
  lv_obj_add_event_cb(btn_wifi, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_wifi = lv_label_create(btn_wifi);
  lv_label_set_text_fmt(label_wifi, LV_SYMBOL_WIFI " WiFi Scan");
  lv_obj_add_style(btn_wifi, &style_btn, 0);
  lv_obj_align(label_wifi, LV_ALIGN_LEFT_MID, 0, 0);

  btn_deauther = lv_btn_create(cont);
  lv_obj_set_width(btn_deauther, lv_pct(100));
  lv_obj_add_event_cb(btn_deauther, btn_roll_callback, LV_EVENT_ALL, NULL);
  LV_IMG_DECLARE(wifi_deauther);
  lv_obj_t *img = lv_img_create(btn_deauther);
  lv_img_set_src(img, &wifi_deauther);
  lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_add_style(btn_deauther, &style_btn, 0);
  lv_obj_t *label_deauther = lv_label_create(btn_deauther);
  lv_label_set_text_fmt(label_deauther, " WiFi Deauther");
  lv_obj_align(label_deauther, LV_ALIGN_LEFT_MID, 20, 0);

  btn_wifi_spam = lv_btn_create(cont);
  lv_obj_set_width(btn_wifi_spam, lv_pct(100));
  lv_obj_add_event_cb(btn_wifi_spam, btn_roll_callback, LV_EVENT_ALL, NULL);
  LV_IMG_DECLARE(wifi_spam);
  lv_obj_t *img_wifi_spam = lv_img_create(btn_wifi_spam);
  lv_img_set_src(img_wifi_spam, &wifi_spam);
  lv_obj_align(img_wifi_spam, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_add_style(btn_wifi_spam, &style_btn, 0);
  lv_obj_t *label_wifi_spam = lv_label_create(btn_wifi_spam);
  lv_label_set_text_fmt(label_wifi_spam, " WiFi Spam");
  lv_obj_align(label_wifi_spam, LV_ALIGN_LEFT_MID, 20, 0);

  /*
  btn_ble = lv_btn_create(cont);
  lv_obj_set_width(btn_ble, lv_pct(100));
  lv_obj_t *label_ble = lv_label_create(btn_ble);
  lv_label_set_text_fmt(label_ble, LV_SYMBOL_BLUETOOTH " Bluetooth");
  lv_obj_add_style(btn_ble, &style_btn, 0);

  btn_usb = lv_btn_create(cont);
  lv_obj_set_width(btn_usb, lv_pct(100));
  lv_obj_t *label_usb = lv_label_create(btn_usb);
  lv_label_set_text_fmt(label_usb, LV_SYMBOL_USB " USB");
  lv_obj_add_style(btn_usb, &style_btn, 0);

  btn_sd = lv_btn_create(cont);
  lv_obj_set_width(btn_sd, lv_pct(100));
  lv_obj_t *label_sd = lv_label_create(btn_sd);
  lv_label_set_text_fmt(label_sd, LV_SYMBOL_SD_CARD " SD");
  lv_obj_add_style(btn_sd, &style_btn, 0);

  btn_bat = lv_btn_create(cont);
  lv_obj_set_width(btn_bat, lv_pct(100));
  lv_obj_t *label_bat = lv_label_create(btn_bat);
  lv_label_set_text_fmt(label_bat, LV_SYMBOL_BATTERY_FULL " Battery");
  lv_obj_add_style(btn_bat, &style_btn, 0);

  btn_charge = lv_btn_create(cont);
  lv_obj_set_width(btn_charge, lv_pct(100));
  lv_obj_t *label_charge = lv_label_create(btn_charge);
  lv_label_set_text_fmt(label_charge, LV_SYMBOL_CHARGE " Charge");
  lv_obj_add_style(btn_charge, &style_btn, 0);
  */

  btn_settings = lv_btn_create(cont);
  lv_obj_set_width(btn_settings, lv_pct(100));
  lv_obj_set_height(btn_settings, 35);
  lv_obj_add_event_cb(btn_settings, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_settings = lv_label_create(btn_settings);
  lv_label_set_text_fmt(label_settings, LV_SYMBOL_SETTINGS " Settings");
  lv_obj_add_style(btn_settings, &style_btn, 0);
  lv_obj_align(label_settings, LV_ALIGN_LEFT_MID, 0, 0);

  btn_clear_ram = lv_btn_create(cont);
  lv_obj_set_width(btn_clear_ram, lv_pct(100));
  lv_obj_set_height(btn_clear_ram, 35);
  lv_obj_add_event_cb(btn_clear_ram, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_clear_ram = lv_label_create(btn_clear_ram);
  lv_label_set_text_fmt(label_clear_ram, LV_SYMBOL_TRASH " Clean RAM");
  lv_obj_add_style(btn_clear_ram, &style_btn, 0);
  lv_obj_align(label_clear_ram, LV_ALIGN_LEFT_MID, 0, 0);

  btn_restart = lv_btn_create(cont);
  lv_obj_set_width(btn_restart, lv_pct(100));
  lv_obj_set_height(btn_restart, 35);
  lv_obj_add_event_cb(btn_restart, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_restart = lv_label_create(btn_restart);
  lv_label_set_text_fmt(label_restart, LV_SYMBOL_LOOP " Restart");
  lv_obj_add_style(btn_restart, &style_btn, 0);
  lv_obj_align(label_restart, LV_ALIGN_LEFT_MID, 0, 0);

  btn_information = lv_btn_create(cont);
  lv_obj_set_width(btn_information, lv_pct(100));
  lv_obj_set_height(btn_information, 35);
  lv_obj_add_event_cb(btn_information, btn_roll_callback, LV_EVENT_ALL, NULL);
  lv_obj_t *label_information = lv_label_create(btn_information);
  lv_label_set_text_fmt(label_information, LV_SYMBOL_WARNING " Information");
  lv_obj_add_style(btn_information, &style_btn, 0);
  lv_obj_align(label_information, LV_ALIGN_LEFT_MID, 0, 0);

  /*Update the buttons position manually for first*/
  lv_event_send(cont, LV_EVENT_SCROLL, NULL);
  /*Be sure the fist button is in the middle*/
  lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_OFF);
}

static void app_display_clean_component(void) {
  app_display_deauther_wifi_off();
  app_display_scan_wifi_off();
  app_display_spam_wifi_off();
  app_display_settings_off();
}

static void message_timer_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_obj_add_flag(msgbox_clean_up_ram, LV_OBJ_FLAG_HIDDEN);
  lv_obj_del_async(msgbox_clean_up_ram);
  lv_timer_del(message_timer);
}

static void btn_roll_callback(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (254 >= lv_obj_get_style_opa(btn, LV_PART_MAIN))
      return;
    if (btn == btn_power) {
      build_msgbox_power();
    }
    if (btn == btn_wifi) {
      app_display_scan_wifi_build();
      app_display_scan_wifi_on();
    }
    if (btn == btn_deauther) {
      app_display_deauther_wifi_build();
      app_display_deauther_wifi_on();
    }
    if (btn == btn_wifi_spam) {
      app_display_spam_wifi_build();
      app_display_spam_wifi_on();
    }
    if (btn == btn_settings) {
      app_display_settings_build();
      app_display_settings_on();
    }
    if (btn == btn_restart) {
      build_msgbox_restart();
    }
    if (btn == btn_information) {
      build_msgbox_information();
    }
    if (btn == btn_clear_ram) {
      app_display_clean_component();
      msgbox_clean_up_ram = lv_msgbox_create(lv_scr_act(), "Message                            ", "Clean up RAM Ok !", NULL, false);
      lv_obj_center(msgbox_clean_up_ram);
      lv_obj_set_size(msgbox_clean_up_ram, 210, 80);
      message_timer = lv_timer_create(message_timer_cb, 3000, NULL);
    }
  }
}

static void scroll_event_cb(lv_event_t *e) {
  lv_obj_t *cont = lv_event_get_target(e);

  lv_area_t cont_a;
  lv_obj_get_coords(cont, &cont_a);
  lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

  lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
  uint32_t i;
  uint32_t child_cnt = lv_obj_get_child_cnt(cont);
  for (i = 0; i < child_cnt; i++) {
    lv_obj_t *child = lv_obj_get_child(cont, i);
    lv_area_t child_a;
    lv_obj_get_coords(child, &child_a);

    lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

    lv_coord_t diff_y = child_y_center - cont_y_center;
    diff_y = LV_ABS(diff_y);

    /*Get the x of diff_y on a circle.*/
    lv_coord_t x;
    /*If diff_y is out of the circle use the last point of the circle (the radius)*/
    if (diff_y >= r) {
      x = r;
    } else {
      /*Use Pythagoras theorem to get x from radius and y*/
      uint32_t x_sqr = r * r - diff_y * diff_y;
      lv_sqrt_res_t res;
      lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
      x = r - res.i;
    }

    /*Translate the item by the calculated X coordinate*/
    lv_obj_set_style_translate_x(child, x, 0);

    /*Use some opacity with larger translations*/
    lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
  }
}

static void screen_wait_off_callback(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (status_click == 0) {
      time_last = millis();
      status_click = 1;
    } else if (status_click == 1) {
      if (millis() - time_last < 300) {
        lv_obj_add_flag(screen_wait_off, LV_OBJ_FLAG_HIDDEN);
        if (show_fps_ram_active) {
          lv_obj_set_style_text_color(perf_monitor.perf_label, lv_color_white(), 0);
          lv_obj_set_style_text_color(mem_monitor.mem_label, lv_color_white(), 0);
        }
      }
      status_click = 0;
    }
  }
}

static void build_screen_wait_off(void) {
  if (screen_wait_off_init) {
    lv_obj_clear_flag(screen_wait_off, LV_OBJ_FLAG_HIDDEN);
    if (show_fps_ram_active) {
      lv_obj_set_style_text_color(perf_monitor.perf_label, lv_color_black(), 0);
      lv_obj_set_style_text_color(mem_monitor.mem_label, lv_color_black(), 0);
    }
    return;
  }
  screen_wait_off_init = 1;
  static lv_style_t style_btn;
  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, lv_color_hex(0x000000));
  screen_wait_off = lv_btn_create(lv_scr_act());
  lv_obj_set_size(screen_wait_off, lv_pct(100), lv_pct(100));
  lv_obj_center(screen_wait_off);
  lv_obj_add_event_cb(screen_wait_off, screen_wait_off_callback, LV_EVENT_CLICKED, NULL);
  lv_obj_add_style(screen_wait_off, &style_btn, 0);
  if (show_fps_ram_active) {
    lv_obj_set_style_text_color(perf_monitor.perf_label, lv_color_black(), 0);
    lv_obj_set_style_text_color(mem_monitor.mem_label, lv_color_black(), 0);
  }
}

static void restart_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  char *text_btn = (char *)lv_msgbox_get_active_btn_text(obj);
  if (_strcmp(text_btn, "") == 0) {
  } else if (_strcmp(text_btn, "Yes") == 0) {
    wifi_off();
    wdt_restart.InitWatchdog(1);  // setup 5s watchdog
    wdt_restart.StartWatchdog();  // enable watchdog timer
    vTaskDelay(500);
  } else if (_strcmp(text_btn, "No") == 0) {
    lv_obj_add_flag(msgbox_restart, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clean(msgbox_restart);
  }
}

static void power_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  char *text_password = (char *)lv_msgbox_get_active_btn_text(obj);
  if (_strcmp(text_password, "") == 0) {
  } else if (_strcmp(text_password, "Yes") == 0) {

    build_screen_wait_off();

    lv_obj_add_flag(msgbox_power, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del_async(msgbox_power);
  } else if (_strcmp(text_password, "No") == 0) {
    lv_obj_add_flag(msgbox_power, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del_async(msgbox_power);
  }
}

static void build_msgbox_restart(void) {
  msgbox_restart = lv_msgbox_create(lv_scr_act(), "Restart", "Do you want to restart device ?", btns, true);
  lv_obj_set_size(msgbox_restart, 180 + 100, 120);
  lv_obj_add_event_cb(msgbox_restart, restart_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_center(msgbox_restart);
  lv_obj_clear_flag(msgbox_restart, LV_OBJ_FLAG_HIDDEN);
}

static void build_msgbox_power(void) {
  msgbox_power = lv_msgbox_create(lv_scr_act(), "Sleep", "Do you want to sleep device ?", btns, true);
  lv_obj_set_size(msgbox_power, 180 + 100, 120);
  lv_obj_add_event_cb(msgbox_power, power_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_center(msgbox_power);
  lv_obj_clear_flag(msgbox_power, LV_OBJ_FLAG_HIDDEN);
}

void main_set_show_fps_ram(uint8_t show) {
  show_fps_ram_active = show;
}

static void information_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *infor = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_CLICKED) {
    lv_obj_add_flag(infor, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del_async(infor);
  }
}

static void build_msgbox_information(void) {
  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  lv_obj_t *information = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(information, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(information, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(information, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(information, &style_background, 0);

  lv_obj_t *label_infor = lv_label_create(information);
  lv_label_set_text(label_infor, LV_SYMBOL_WARNING " Information");
  lv_obj_align(label_infor, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t *information_close_btn = lv_btn_create(information);
  lv_obj_set_size(information_close_btn, 30, 30);
  lv_obj_align(information_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(information_close_btn, information_btn_event_cb, LV_EVENT_ALL, information);
  lv_obj_t *btn_close_symbol = lv_label_create(information_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  lv_obj_t *label_text = lv_label_create(information);
  lv_obj_set_size(label_text, 290, 170);
  lv_obj_align_to(label_text, label_infor, LV_ALIGN_TOP_LEFT, 0, 30);

  lv_label_set_text(label_text, "Chip: Realtek RTL8720DN\n"
                                "+ Cortex-M33, 200MHz\n"
                                "+ Cortex-M23,  20MHz\n"
                                "Dual-band Wi-Fi 4, 2.4GHz + 5.8 GHz\n"
                                "LVGL: v8.3.11\n"
                                "Aduino IDE: v.2.3.4\n"
                                "TFT: 2.8 inch\n"
                                "Software: v1.1.0");
  /*Remove the style of scrollbar to have clean start*/
  lv_obj_remove_style(label_text, NULL, LV_PART_SCROLLBAR | LV_STATE_ANY);

  lv_obj_clear_flag(information, LV_OBJ_FLAG_HIDDEN);
}
