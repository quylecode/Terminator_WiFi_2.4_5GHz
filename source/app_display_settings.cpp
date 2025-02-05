#include "app_display_settings.h"
#include <FlashStorage_RTL8720.h>

#define ADDRESS_SAVE_CONFIG_DEVICE 0x3FE000
#define SIZE_SAVE_CONFIG_DEVICE 0x2000

#define ADDRESS_SAVE_CONFIG 0x00000

struct config_device_t {
  /* WiFi Deauther */
  uint8_t deauth_reason;
  uint8_t frames_per_deauth;
  uint8_t num_wifi_deauth;
  /* WiFi Scan */
  uint8_t num_wifi_list;
  /* WiFi Spam */
  uint8_t spam_mode;
  uint8_t number_wifi_spam;
  uint8_t length_name_wifi_spam;
  /* password */
  char password[8];
  /* Display */
  uint8_t show_fps_and_ram;
};

static config_device_t config_device;

static uint8_t settings_main_init = 0;

static lv_obj_t *menu;
static lv_obj_t *settings_main;
static lv_obj_t *settings_main_close_btn;
static lv_obj_t *settings_main_btn_save;

static lv_obj_t *text_password;
static lv_obj_t *btn_password;

static lv_obj_t *msgbox_settings;
static lv_timer_t *timer_msgbox_settings;

static void settings_btn_event_cb(lv_event_t *e);
static void password_event_handler(lv_event_t *e);
static void build_settings_config_wifi_deauther(lv_obj_t *parent);
static void build_settings_config_password(lv_obj_t *parent);
static void build_settings_config_wifi_scan(lv_obj_t *parent);
static void build_settings_config_wifi_spam(lv_obj_t *parent);
static void build_settings_config_display(lv_obj_t *parent);

static void app_display_settings_set_deauther_wifi_config(void);
static void app_display_settings_set_scan_wifi_config(void);
static void app_display_settings_set_spam_wifi_config(void);
static void app_display_settings_set_password_config(void);
static void app_display_settings_set_main(void);

static void app_display_settings_print_all_config(void);

void app_display_settings_load_config(void) {
  /* request flash size 0x2000 from 0x00100000 */
  FlashStorage.get(ADDRESS_SAVE_CONFIG, config_device);
  app_display_settings_set_deauther_wifi_config();
  app_display_settings_set_scan_wifi_config();
  app_display_settings_set_spam_wifi_config();

  /* memcpy(config_device.password, "2304    ", 8); */
  if (config_device.password[0] == 255) {
    memcpy(config_device.password, "0000    ", 8);
    FlashStorage.put(ADDRESS_SAVE_CONFIG, config_device);
  }

  app_display_settings_set_password_config();
  app_display_settings_set_main();
  app_display_settings_print_all_config();
}

static void app_display_settings_print_all_config(void) {
  printf("----- Print all config -----\r\n");
  printf("deauth_reason: %d\r\n", config_device.deauth_reason);
  printf("frames_per_deauth: %d\r\n", config_device.frames_per_deauth);
  printf("num_wifi_deauth: %d\r\n", config_device.num_wifi_deauth);
  printf("num_wifi_list: %d\r\n", config_device.num_wifi_list);
  printf("spam_style: %d\r\n", config_device.spam_mode);
  printf("spam_number: %d\r\n", config_device.number_wifi_spam);
  printf("length_name_wifi: %d\r\n", config_device.length_name_wifi_spam);
  printf("password_true: %s\r\n", config_device.password);
  printf("show_fps_and_ram: %d\r\n", config_device.show_fps_and_ram);
}

static void timer_msgbox_settings_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_obj_add_flag(msgbox_settings, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clean(msgbox_settings);
  lv_timer_del(timer_msgbox_settings);
}

static void app_display_settings_show_message(const char *content) {
  msgbox_settings = lv_msgbox_create(lv_scr_act(), "Settings                           ", content, NULL, false);
  lv_obj_center(msgbox_settings);
  lv_obj_set_size(msgbox_settings, 210, 80);
  lv_obj_set_style_bg_color(msgbox_settings, lv_color_black(), 0);
  timer_msgbox_settings = lv_timer_create(timer_msgbox_settings_cb, 2000, NULL);
}

static void back_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *menu = (lv_obj_t *)lv_event_get_user_data(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    if (lv_menu_get_cur_main_page(menu) == lv_obj_get_child(menu, 0)) {
      if (!lv_obj_has_flag(btn_password, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(btn_password, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
}

void app_display_settings_build(void) {
  if (settings_main_init == 1)
    return;
  settings_main_init = 1;

  /* Check ram usage */
  if (!app_display_scan_wifi_get_active()) {
    app_display_scan_wifi_off();
  }
  if (!app_display_deauther_wifi_get_active()) {
    app_display_deauther_wifi_off();
  }
  if (!app_display_spam_wifi_get_active()) {
    app_display_spam_wifi_off();
  }

  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  /* Settings screen main */
  settings_main = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(settings_main, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(settings_main, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(settings_main, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(settings_main, &style_background, 0);

  lv_obj_t *settings_label = lv_label_create(settings_main);
  lv_label_set_text(settings_label, LV_SYMBOL_SETTINGS " Settings");
  lv_obj_align(settings_label, LV_ALIGN_TOP_LEFT, 0, 0);

  settings_main_close_btn = lv_btn_create(settings_main);
  lv_obj_set_size(settings_main_close_btn, 30, 30);
  lv_obj_align(settings_main_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(settings_main_close_btn, settings_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol = lv_label_create(settings_main_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  settings_main_btn_save = lv_btn_create(settings_main);
  lv_obj_set_size(settings_main_btn_save, 70, 30);
  lv_obj_align(settings_main_btn_save, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(settings_main_btn_save, settings_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_save_symbol = lv_label_create(settings_main_btn_save);
  lv_label_set_text_fmt(btn_save_symbol, LV_SYMBOL_SAVE " Save");
  lv_obj_center(btn_save_symbol);

  static lv_style_t style_menu;
  lv_style_init(&style_menu);
  /*Set a background color and a radius*/
  lv_style_set_radius(&style_menu, 10);
  /*Create a menu object*/
  menu = lv_menu_create(settings_main);
  lv_obj_set_size(menu, lv_pct(100), lv_pct(72));
  lv_obj_set_pos(menu, lv_pct(0), lv_pct(12));
  lv_obj_add_style(menu, &style_menu, 0);

  /*Modify the header*/
  lv_obj_t *back_btn = lv_menu_get_main_header_back_btn(menu);
  lv_obj_t *back_btn_label = lv_label_create(back_btn);
  lv_label_set_text(back_btn_label, "Back");

  lv_obj_t *cont;
  lv_obj_t *label;

  /*Create sub pages*/
  lv_obj_t *sub_1_page = lv_menu_page_create(menu, "WiFi Scan");
  build_settings_config_wifi_scan(sub_1_page);

  lv_obj_t *sub_2_page = lv_menu_page_create(menu, " WiFi Deauther");
  build_settings_config_wifi_deauther(sub_2_page);

  lv_obj_t *sub_3_page = lv_menu_page_create(menu, "WiFi Spam");
  build_settings_config_wifi_spam(sub_3_page);

  lv_obj_t *sub_4_page = lv_menu_page_create(menu, "Change password");
  build_settings_config_password(sub_4_page);

  lv_obj_t *sub_5_page = lv_menu_page_create(menu, "Display");
  build_settings_config_display(sub_5_page);

  /*Create a main page*/
  lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "WiFi Scan");
  lv_menu_set_load_page_event(menu, cont, sub_1_page);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "WiFi Deauther");
  lv_menu_set_load_page_event(menu, cont, sub_2_page);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "WiFi Spam");
  lv_menu_set_load_page_event(menu, cont, sub_3_page);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "Change password");
  lv_menu_set_load_page_event(menu, cont, sub_4_page);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "Display");
  lv_menu_set_load_page_event(menu, cont, sub_5_page);

  lv_menu_set_page(menu, main_page);

  static const char *btn_map[] = { "1", "2", "3", "\n",
                                   "4", "5", "6", "\n",
                                   "7", "8", "9", "\n",
                                   LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, "" };

  btn_password = lv_btnmatrix_create(settings_main);
  lv_obj_set_size(btn_password, 110, 110);
  lv_obj_clear_flag(btn_password, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
  lv_btnmatrix_set_map(btn_password, btn_map);
  lv_obj_add_event_cb(btn_password, password_event_handler, LV_EVENT_VALUE_CHANGED, text_password);
  lv_obj_set_pos(btn_password, 155, 60);
  lv_obj_add_flag(btn_password, LV_OBJ_FLAG_HIDDEN);

  lv_obj_add_event_cb(menu, back_event_handler, LV_EVENT_VALUE_CHANGED, menu);
}

void app_display_settings_on(void) {
  lv_obj_clear_flag(settings_main, LV_OBJ_FLAG_HIDDEN);
}

void app_display_settings_off(void) {
  if (settings_main_init) {
    lv_obj_del_async(settings_main);
    settings_main_init = 0;
  }
}

uint8_t app_display_settings_get_active(void) {
  if (settings_main_init) {
    return lv_obj_has_flag(settings_main, LV_OBJ_FLAG_HIDDEN) ? 0 : 1;
  } else {
    return 1;
  }
}

static void settings_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    if (btn == settings_main_close_btn) {
      lv_obj_add_flag(settings_main, LV_OBJ_FLAG_HIDDEN);
    }
    if (btn == settings_main_btn_save) {
      app_display_settings_set_deauther_wifi_config();
      app_display_settings_set_scan_wifi_config();
      app_display_settings_set_spam_wifi_config();
      app_display_settings_set_main();
      /* write save flash memory */
      FlashStorage.put(ADDRESS_SAVE_CONFIG, config_device);
    }
  }
}

static void lv_spinbox_increment_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    lv_spinbox_increment(spinbox);
    config_device.frames_per_deauth = lv_spinbox_get_value(spinbox);
  }
}

static void lv_spinbox_decrement_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    lv_spinbox_decrement(spinbox);
    config_device.frames_per_deauth = lv_spinbox_get_value(spinbox);
  }
}

static void deauth_reason_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    config_device.deauth_reason = atoi(String(buf).c_str());
  }
}

static void number_wifi_deauth_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    config_device.num_wifi_deauth = atoi(String(buf).c_str());
  }
}

static void app_display_settings_set_deauther_wifi_config(void) {
  deauther_wifi_set_deauth_reason(config_device.deauth_reason);
  deauther_wifi_set_frames_per_deauth(config_device.frames_per_deauth);
  deauther_wifi_set_num_wifi_deauth(config_device.num_wifi_deauth);
  deauther_wifi_set_num_wifi_list(config_device.num_wifi_list);
}

static void build_settings_config_wifi_deauther(lv_obj_t *parent) {
  lv_obj_t *cont;
  lv_obj_t *label;

  /* Create config deauth reason */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Deauther reason: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  const char *opts = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24";
  lv_obj_t *dd;
  dd = lv_dropdown_create(cont);
  lv_dropdown_set_options_static(dd, opts);
  lv_obj_set_width(dd, lv_pct(25));
  lv_obj_align(dd, LV_ALIGN_TOP_RIGHT, 0, 10);
  lv_obj_add_event_cb(dd, deauth_reason_event_cb, LV_EVENT_ALL, NULL);
  lv_dropdown_set_selected(dd, config_device.deauth_reason % 25);

  /* Create config frames per deauth */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Frames / deauth: ");

  lv_obj_t *spinbox = lv_spinbox_create(cont);
  lv_spinbox_set_range(spinbox, 0, 100);
  lv_spinbox_set_digit_format(spinbox, 3, 0);
  lv_spinbox_step_next(spinbox);
  lv_spinbox_set_value(spinbox, config_device.frames_per_deauth % 101);
  lv_obj_set_width(spinbox, 45);
  lv_obj_center(spinbox);

  lv_coord_t h = lv_obj_get_height(spinbox) - 5;

  lv_obj_t *btn = lv_btn_create(cont);
  lv_obj_set_size(btn, h, h);
  lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
  lv_obj_add_event_cb(btn, lv_spinbox_increment_event_cb, LV_EVENT_ALL, spinbox);

  btn = lv_btn_create(cont);
  lv_obj_set_size(btn, h, h);
  lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
  lv_obj_add_event_cb(btn, lv_spinbox_decrement_event_cb, LV_EVENT_ALL, spinbox);

  /* Create config number wifi deauth */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Num WiFi Deauth: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  const char *opts_num_wifi_deauth = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25";
  dd = lv_dropdown_create(cont);
  lv_dropdown_set_options_static(dd, opts_num_wifi_deauth);
  lv_obj_set_width(dd, lv_pct(25));
  lv_obj_align(dd, LV_ALIGN_TOP_RIGHT, 0, 10);
  lv_obj_add_event_cb(dd, number_wifi_deauth_event_cb, LV_EVENT_ALL, NULL);
  lv_dropdown_set_selected(dd, config_device.num_wifi_deauth % 26);
}

static void app_display_settings_set_password_config(void) {
  lock_set_password(config_device.password);
}
static void text_password_settings_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_obj_clear_flag(btn_password, LV_OBJ_FLAG_HIDDEN);
  }
  if (code == LV_EVENT_DEFOCUSED) {
    lv_obj_add_flag(btn_password, LV_OBJ_FLAG_HIDDEN);
  }
}

/* Event handle callback btn password */
static void password_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *textarea_password = (lv_obj_t *)lv_event_get_user_data(e);
  const char *char_number = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));
  if (_strcmp(char_number, LV_SYMBOL_BACKSPACE) == 0) {
    lv_textarea_del_char(textarea_password);
  } else if (_strcmp(char_number, LV_SYMBOL_OK) == 0) {
    lv_obj_add_flag(btn_password, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_textarea_add_text(textarea_password, char_number);
  }
}

/* Event handle callback btn set change */
static void button_set_change_password_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (strlen(lv_textarea_get_text(text_password)) != 0) {
      const char *txt = lv_textarea_get_text(text_password);
      uint8_t len = strlen(txt);
      memcpy_P(config_device.password, txt, len);
      for (uint8_t i = len; i < 8; i++) {
        config_device.password[i] = ' ';
      }
      FlashStorage.put(ADDRESS_SAVE_CONFIG, config_device);
      app_display_settings_show_message("Password change ok !");
    } else {
      app_display_settings_show_message("Password is empty !");
    }
  }
}

/* Event handle callback btn eye */
static void button_eye_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_CLICKED) {
    if (lv_textarea_get_password_mode(text_password)) {
      lv_textarea_set_password_mode(text_password, false);
      lv_label_set_text_fmt(label, LV_SYMBOL_EYE_OPEN);
    } else {
      lv_textarea_set_password_mode(text_password, true);
      lv_label_set_text_fmt(label, LV_SYMBOL_EYE_CLOSE);
    }
  }
}

static void build_settings_config_password(lv_obj_t *parent) {
  lv_obj_t *cont;
  lv_obj_t *label;

  cont = lv_menu_cont_create(parent);
  label = lv_label_create(cont);
  lv_label_set_text(label, "New password:");

  cont = lv_menu_cont_create(parent);
  /*Create the password box*/
  text_password = lv_textarea_create(cont);
  lv_textarea_set_text(text_password, "");
  lv_textarea_set_max_length(text_password, 8);
  lv_textarea_set_password_mode(text_password, true);
  lv_textarea_set_one_line(text_password, true);
  lv_obj_set_width(text_password, lv_pct(40));
  lv_obj_align(text_password, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_add_state(text_password, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
  lv_textarea_set_placeholder_text(text_password, "Password");
  lv_obj_add_event_cb(text_password, text_password_settings_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *button_eye = lv_btn_create(cont);
  lv_obj_set_size(button_eye, 25, 25);
  label = lv_label_create(button_eye);
  lv_label_set_text_fmt(label, LV_SYMBOL_EYE_CLOSE);
  lv_obj_center(label);
  lv_obj_add_event_cb(button_eye, button_eye_event_cb, LV_EVENT_CLICKED, label);

  lv_obj_t *button_set_change_password = lv_btn_create(cont);
  lv_obj_set_size(button_set_change_password, 90, 30);
  lv_obj_align(button_set_change_password, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(button_set_change_password, button_set_change_password_event_cb, LV_EVENT_CLICKED, NULL);
  label = lv_label_create(button_set_change_password);
  lv_label_set_text_fmt(label, LV_SYMBOL_EDIT " Change");
  lv_obj_center(label);
}

static void app_display_settings_set_scan_wifi_config(void) {
  wifi_scan_set_number_wifi_load(config_device.num_wifi_list);
}

static void wifi_scan_settings_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    config_device.num_wifi_list = atoi(String(buf).c_str());
  }
}

static void build_settings_config_wifi_scan(lv_obj_t *parent) {
  lv_obj_t *cont;
  lv_obj_t *label;

  /* Create config deauth reason */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Number WiFi Scan: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  const char *opts = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30";
  lv_obj_t *dd;
  dd = lv_dropdown_create(cont);
  lv_dropdown_set_options_static(dd, opts);
  lv_obj_set_width(dd, lv_pct(25));
  lv_obj_align(dd, LV_ALIGN_TOP_RIGHT, 0, 10);
  lv_obj_add_event_cb(dd, wifi_scan_settings_event_cb, LV_EVENT_ALL, NULL);
  lv_dropdown_set_selected(dd, config_device.num_wifi_list % 31);
}

static void app_display_settings_set_spam_wifi_config(void) {
  wifi_spam_set_spam_style(config_device.spam_mode);
  wifi_spam_set_spam_number(config_device.number_wifi_spam);
  wifi_spam_set_length_name_wifi(config_device.length_name_wifi_spam);
}

static void wifi_spam_settings_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    if (_strcmp(buf, "Default") == 0) {
      config_device.spam_mode = 0;
    } else if (_strcmp(buf, "Number") == 0) {
      config_device.spam_mode = 1;
    } else if (_strcmp(buf, "Character") == 0) {
      config_device.spam_mode = 2;
    } else if (_strcmp(buf, "Letter") == 0) {
      config_device.spam_mode = 3;
    } else {
      config_device.length_name_wifi_spam = atoi(String(buf).c_str()) % 33;
    }
  }
}

static void lv_spinbox_wifi_spam_increment_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    lv_spinbox_increment(spinbox);
    config_device.number_wifi_spam = lv_spinbox_get_value(spinbox);
  }
}

static void lv_spinbox_wifi_spam_decrement_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *spinbox = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
    lv_spinbox_decrement(spinbox);
    config_device.number_wifi_spam = lv_spinbox_get_value(spinbox);
  }
}

static void build_settings_config_wifi_spam(lv_obj_t *parent) {
  lv_obj_t *cont;
  lv_obj_t *label;

  /* Create stype spam wifi */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "WiFi Random Style: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  const char *opts = "Default\nNumber\nCharacter\nLetter";
  lv_obj_t *dd;
  dd = lv_dropdown_create(cont);
  lv_dropdown_set_options_static(dd, opts);
  lv_obj_set_width(dd, lv_pct(40));
  lv_obj_align(dd, LV_ALIGN_TOP_RIGHT, 0, 10);
  lv_obj_add_event_cb(dd, wifi_spam_settings_event_cb, LV_EVENT_ALL, NULL);
  lv_dropdown_set_selected(dd, config_device.spam_mode % 4);

  /* Create number wifi spam */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Num WiFi Spam: ");

  lv_obj_t *spinbox = lv_spinbox_create(cont);
  lv_spinbox_set_range(spinbox, 0, 100);
  lv_spinbox_set_digit_format(spinbox, 3, 0);
  lv_spinbox_step_next(spinbox);
  lv_spinbox_set_value(spinbox, config_device.number_wifi_spam % 101);
  lv_obj_set_width(spinbox, 45);
  lv_obj_center(spinbox);

  lv_coord_t h = lv_obj_get_height(spinbox) - 5;

  lv_obj_t *btn = lv_btn_create(cont);
  lv_obj_set_size(btn, h, h);
  lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
  lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_PLUS, 0);
  lv_obj_add_event_cb(btn, lv_spinbox_wifi_spam_increment_event_cb, LV_EVENT_ALL, spinbox);

  btn = lv_btn_create(cont);
  lv_obj_set_size(btn, h, h);
  lv_obj_align_to(btn, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_MINUS, 0);
  lv_obj_add_event_cb(btn, lv_spinbox_wifi_spam_decrement_event_cb, LV_EVENT_ALL, spinbox);

  /* Create length name wifi spam */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Name WiFi Length: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  const char *opts_length_name_wifi = "2\n4\n6\n8\n10\n12\n14\n16\n18\n20\n22\n24\n26\n28\n30\n32";
  dd = lv_dropdown_create(cont);
  lv_dropdown_set_options_static(dd, opts_length_name_wifi);
  lv_obj_set_width(dd, lv_pct(25));
  lv_obj_align(dd, LV_ALIGN_TOP_RIGHT, 0, 10);
  lv_obj_add_event_cb(dd, wifi_spam_settings_event_cb, LV_EVENT_ALL, NULL);
  lv_dropdown_set_selected(dd, (config_device.length_name_wifi_spam % 33) / 2 - 1);
}

static void app_display_settings_set_main(void) {
  main_set_show_fps_ram(config_device.show_fps_and_ram);
}

static void display_switch_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *switch_box = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    if (lv_obj_has_state(switch_box, LV_STATE_CHECKED)) {
      config_device.show_fps_and_ram = 1;
    } else {
      config_device.show_fps_and_ram = 0;
    }
  }
}

static void build_settings_config_display(lv_obj_t *parent) {
  lv_obj_t *cont;
  lv_obj_t *label;

  /* Create stype spam wifi */
  cont = lv_menu_cont_create(parent);

  label = lv_label_create(cont);
  lv_label_set_text(label, "Show FPS and RAM: ");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);

  label = lv_switch_create(cont);
  if (config_device.show_fps_and_ram) {
    lv_obj_add_state(label, LV_STATE_CHECKED);
  } else {
    lv_obj_clear_state(label, LV_STATE_CHECKED);
  }
  lv_obj_add_event_cb(label, display_switch_btn_event_cb, LV_EVENT_VALUE_CHANGED, label);
  lv_obj_set_size(label, 35, 20);
}
