#include "app_display_lock.h"
#include "icon_screen_load.h"
#include <lvgl.h>

const char *message_[] PROGMEM = {
  "This is a product for research !",
  "Thank you for your appreciation !",
  "Contact me on Tiktok @quyle2304 :)",
  "Ready..."
};

/* Obj screen */
static lv_obj_t *screen_animation;
static lv_obj_t *screen_password;
static lv_obj_t *screen_wait;

static TaskHandle_t app_display_lock_animation_handler = NULL, app_display_animation_handler = NULL;

/* Funtion create screen lock */
static void app_display_lock_animation(void *pvParameters);
static void app_display_lock_main_on(void);
static void app_display_lock_main_hidden(void);
static void app_display_lock_create_screen_wait(void);
static void app_display_lock_create_screen_password(void);

/* Funtion private */
static lv_obj_t *create_title_screen(lv_obj_t *parent);
static lv_obj_t *create_handle_screen(lv_obj_t *parent);
static lv_obj_t *create_title_password(lv_obj_t *parent);

/* Variable private */
static lv_style_t style_scrollbar;
static lv_style_t style_btn;

static lv_obj_t *msgbox_password;
static char password_true[9];

/* Funtion handler callback event */
static void btn_password_event_handler(lv_event_t *e);
static void app_display_animation(void *pvParameters);

void app_display_lock_on(void) {
  xTaskCreate(app_display_lock_animation,
              "app_display_lock_animation",
              512,
              NULL,
              1,
              &app_display_lock_animation_handler);
  xTaskCreate(app_display_animation,
              "app_display_animation",
              512,
              NULL,
              2,
              &app_display_animation_handler);
}

static void set_value_loading(void *bar, int32_t temp) {
  lv_bar_set_value((lv_obj_t *)bar, temp, LV_ANIM_ON);
}

static void anim_x_cb(void *var, int32_t v) {
  lv_obj_set_x((lv_obj_t *)var, v);
}

static void app_display_animation(void *pvParameters) {
  vTaskDelay(200);

  lv_obj_t *label = lv_label_create(screen_animation);
  lv_label_set_text(label, "Terminator v1.1.0");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_CYAN), 0);

  label = lv_label_create(screen_animation);
  lv_label_set_text(label, " ");
  lv_obj_set_pos(label, -100, 25);
  lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_CYAN), 0);

  lv_anim_t orign_label;
  lv_anim_init(&orign_label);
  lv_anim_set_var(&orign_label, label);
  lv_anim_set_values(&orign_label, lv_obj_get_x(label), -lv_obj_get_width(label) - 20);
  lv_anim_set_time(&orign_label, 100);
  lv_anim_set_exec_cb(&orign_label, anim_x_cb);
  lv_anim_set_path_cb(&orign_label, lv_anim_path_ease_in);
  lv_anim_start(&orign_label);

  uint8_t i = 0, step = 0;
  while (1) {
    if (i < 4) {
      if (step == 0) {
        lv_label_set_text(label, message_[i]);
        lv_anim_t on_label;
        lv_anim_init(&on_label);
        lv_anim_set_var(&on_label, label);
        if (i == 2) {
          lv_anim_set_values(&on_label, lv_obj_get_x(label), 15);
        } else if (i == 3) {
          lv_anim_set_values(&on_label, lv_obj_get_x(label), 125);
          i++;
        } else {
          lv_anim_set_values(&on_label, lv_obj_get_x(label), 30);
        }
        lv_anim_set_time(&on_label, 1000);
        lv_anim_set_exec_cb(&on_label, anim_x_cb);
        lv_anim_set_path_cb(&on_label, lv_anim_path_overshoot);
        lv_anim_start(&on_label);
        step = 1;
        vTaskDelay(1500);
      } else {
        lv_anim_t off_label;
        lv_anim_init(&off_label);
        lv_anim_set_var(&off_label, label);
        lv_anim_set_values(&off_label, lv_obj_get_x(label), -lv_obj_get_width(label) - 20);
        lv_anim_set_time(&off_label, 1000);
        lv_anim_set_exec_cb(&off_label, anim_x_cb);
        lv_anim_set_path_cb(&off_label, lv_anim_path_ease_in);
        lv_anim_start(&off_label);
        step = 0;
        i++;
        vTaskDelay(1000);
      }
    } else {
      vTaskDelay(1000);
      vTaskDelete(app_display_animation_handler);
    }
  }
}

static void app_display_lock_animation(void *pvParameters) {
  LV_IMG_DECLARE(screen_load_0);
  LV_IMG_DECLARE(screen_load_1);
  LV_IMG_DECLARE(screen_load_2);
  LV_IMG_DECLARE(screen_load_3);
  LV_IMG_DECLARE(screen_load_4);

  static lv_style_t style_indic;
  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_CYAN));
  lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_CYAN));
  lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

  screen_animation = lv_obj_create(lv_scr_act());
  lv_obj_center(screen_animation);
  lv_obj_set_size(screen_animation, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(screen_animation, lv_color_hex(0x000000), 0);
  lv_obj_set_scrollbar_mode(screen_animation, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *img = lv_img_create(screen_animation);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, -10);
  uint16_t delay = 100;
  uint32_t time_total = 0;

  lv_obj_t *bar = lv_bar_create(screen_animation);
  lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(bar, 250, 20);
  lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_bar_set_range(bar, -20, 40);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_pad_all(bar, 0, 0);

  lv_obj_t *loading_label = lv_label_create(screen_animation);
  lv_label_set_text(loading_label, "Loading...");
  lv_obj_set_style_text_color(loading_label, lv_palette_main(LV_PALETTE_CYAN), 0);
  lv_obj_align(loading_label, LV_ALIGN_BOTTOM_MID, 0, -40);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_exec_cb(&a, set_value_loading);
  lv_anim_set_time(&a, 9500);
  lv_anim_set_var(&a, bar);
  lv_anim_set_values(&a, -20, 40);
  lv_anim_set_repeat_count(&a, 0);
  lv_anim_start(&a);

  while (1) {
    lv_img_set_src(img, &screen_load_0);
    vTaskDelay(delay);
    lv_img_set_src(img, &screen_load_1);
    vTaskDelay(delay);
    lv_img_set_src(img, &screen_load_2);
    vTaskDelay(delay);
    lv_img_set_src(img, &screen_load_3);
    vTaskDelay(delay);
    lv_img_set_src(img, &screen_load_4);
    vTaskDelay(delay);
    time_total += 500;
    if (time_total >= 9500)
      break;
  }
  lv_obj_add_flag(screen_animation, LV_OBJ_FLAG_HIDDEN);
  app_display_lock_main_on();
  vTaskDelete(app_display_lock_animation_handler);
}

static void app_display_lock_main_on(void) {
  app_display_lock_create_screen_password();
  app_display_lock_create_screen_wait();
  lv_obj_clear_flag(screen_wait, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(screen_password, LV_OBJ_FLAG_HIDDEN);
}

void app_display_lock_off(void) {
  lv_obj_del_async(screen_animation);
  lv_obj_del_async(screen_wait);
  lv_obj_del_async(screen_password);
}

static void app_display_lock_main_hidden(void) {
  lv_obj_add_flag(screen_wait, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(screen_password, LV_OBJ_FLAG_HIDDEN);
}

static void app_display_lock_create_screen_wait(void) {
  /*A transparent container in which the player section will be scrolled*/
  screen_wait = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(screen_wait, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(screen_wait, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(screen_wait, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_remove_style_all(screen_wait); /*Make it transparent*/
  lv_obj_set_size(screen_wait, lv_pct(100), lv_pct(100));
  lv_obj_set_scroll_snap_y(screen_wait, LV_SCROLL_SNAP_CENTER); /*Snap the children to the center*/
  lv_obj_center(screen_wait);

  /*Create a container for the player*/
  lv_obj_t *player = lv_obj_create(screen_wait);
  lv_obj_set_y(player, -35);
  lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES + 35 * 2);
  lv_obj_clear_flag(player, LV_OBJ_FLAG_SNAPABLE);
  lv_obj_set_style_bg_color(player, lv_color_hex(0x181818), 0);
  lv_obj_set_style_border_width(player, 0, 0);
  lv_obj_set_style_pad_all(player, 0, 0);
  lv_obj_set_scroll_dir(player, LV_DIR_VER);

  LV_IMG_DECLARE(background_lock);
  lv_obj_t *img;
  img = lv_img_create(player);
  lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_src(img, &background_lock);

  /* Transparent placeholders below the player container
  It is used only to snap it to center.*/
  lv_obj_t *place_holder_1 = lv_obj_create(screen_wait);
  lv_obj_remove_style_all(place_holder_1);
  lv_obj_clear_flag(place_holder_1, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_t *place_holder_2 = lv_obj_create(screen_wait);
  lv_obj_remove_style_all(place_holder_2);
  lv_obj_clear_flag(place_holder_2, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_set_size(place_holder_1, lv_pct(100), LV_VER_RES);
  lv_obj_set_y(place_holder_1, 0);

  lv_obj_set_size(place_holder_2, lv_pct(100), LV_VER_RES - 2 * 35);
  lv_obj_set_y(place_holder_2, LV_VER_RES + 35);

  lv_obj_update_layout(screen_wait);

  /*Arrange the content into a grid*/
  static const lv_coord_t grid_cols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
  static const lv_coord_t grid_rows[] = { 35,              /*Spacing*/
                                          LV_GRID_CONTENT, /*Title box*/
                                          LV_GRID_FR(1),   /*Spacer*/
                                          LV_GRID_CONTENT, /*Icon box*/
                                          LV_GRID_FR(1),   /*Spacer*/
                                          LV_GRID_CONTENT, /*Control box*/
                                          LV_GRID_FR(1),   /*Spacer*/
                                          LV_GRID_CONTENT, /*Control box*/
                                          LV_GRID_FR(1),   /*Spacer*/
                                          LV_GRID_CONTENT, /*Control box*/
                                          35,              /*Spacing*/
                                          LV_GRID_TEMPLATE_LAST };
  lv_obj_set_grid_dsc_array(player, grid_cols, grid_rows);
  lv_obj_set_style_grid_row_align(player, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
  lv_obj_t *title_screen = create_title_screen(player);
  lv_obj_set_grid_cell(title_screen, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_t *handle_screen = create_handle_screen(player);
  lv_obj_set_grid_cell(handle_screen, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 9, 1);
  lv_obj_t *password_screen = create_title_password(player);
  lv_obj_set_grid_cell(password_screen, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 10, 1);
}

static lv_obj_t *create_title_password(lv_obj_t *parent) {
  /*Create the titles*/
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_height(cont, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  /*A handle to scroll to the track list*/
  lv_obj_t *handle_label = lv_label_create(cont);
  lv_label_set_text(handle_label, "Please enter your password to login...");
  lv_obj_set_style_text_color(handle_label, lv_color_hex(0xffffff), 0);
  return cont;
}

static lv_obj_t *create_title_screen(lv_obj_t *parent) {
  /*Create the titles*/
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_height(cont, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  /*A handle to scroll to the track list*/
  lv_obj_t *handle_label = lv_label_create(cont);
  lv_label_set_text(handle_label, " ");
  lv_obj_set_style_text_color(handle_label, lv_color_hex(0xffffff), 0);
  return cont;
}

static lv_obj_t *create_handle_screen(lv_obj_t *parent) {
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_remove_style_all(cont);
  lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(cont, 8, 0);

  lv_obj_t *handle_rec = lv_obj_create(cont);
  lv_obj_set_size(handle_rec, 60, 6);
  lv_obj_set_style_bg_color(handle_rec, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_border_width(handle_rec, 0, 0);
  return cont;
}

static void app_display_lock_create_screen_password(void) {
  static const lv_coord_t grid_cols[] = { LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
  static const lv_coord_t grid_rows[] = { 12,
                                          LV_GRID_CONTENT, /* Text password */
                                          12,
                                          130,
                                          20,
                                          LV_GRID_TEMPLATE_LAST };
  lv_style_init(&style_scrollbar);
  lv_style_set_width(&style_scrollbar, 4);
  lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
  lv_style_set_bg_color(&style_scrollbar, lv_color_hex3(0xeee));
  lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
  lv_style_set_pad_right(&style_scrollbar, 4);

  lv_style_init(&style_btn);
  lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
  lv_style_set_grid_column_dsc_array(&style_btn, grid_cols);
  lv_style_set_grid_row_dsc_array(&style_btn, grid_rows);
  lv_style_set_grid_row_align(&style_btn, LV_GRID_ALIGN_CENTER);
  lv_style_set_layout(&style_btn, LV_LAYOUT_GRID);

  /*Create an empty transparent container*/
  screen_password = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(screen_password, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_style_all(screen_password);
  lv_obj_set_size(screen_password, LV_HOR_RES, LV_VER_RES - 35);
  lv_obj_set_y(screen_password, 35);
  lv_obj_add_style(screen_password, &style_scrollbar, LV_PART_SCROLLBAR);
  lv_obj_set_flex_flow(screen_password, LV_FLEX_FLOW_COLUMN);

  lv_obj_set_grid_dsc_array(screen_password, grid_cols, grid_rows);

  /*Create the password box*/
  lv_obj_t *text_password = lv_textarea_create(screen_password);
  lv_textarea_set_text(text_password, "");
  lv_textarea_set_password_mode(text_password, true);
  lv_textarea_set_max_length(text_password, 8);
  lv_textarea_set_one_line(text_password, true);
  lv_obj_set_width(text_password, lv_pct(40));
  lv_obj_align(text_password, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_add_state(text_password, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
  lv_textarea_set_placeholder_text(text_password, "Password");

  lv_obj_set_grid_cell(text_password, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

  static const char *btn_map[] = { "1", "2", "3", "\n",
                                   "4", "5", "6", "\n",
                                   "7", "8", "9", "\n",
                                   LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_OK, "" };

  lv_obj_t *btn_password = lv_btnmatrix_create(screen_password);
  lv_obj_set_size(btn_password, 130, 130);
  lv_obj_clear_flag(btn_password, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
  lv_obj_add_event_cb(btn_password, btn_password_event_handler, LV_EVENT_VALUE_CHANGED, text_password);
  lv_btnmatrix_set_map(btn_password, btn_map);
  lv_obj_set_grid_cell(btn_password, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
}

static void event_msgbox_cb_password(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  char *text_password = (char *)lv_msgbox_get_active_btn_text(obj);
  if (_strcmp(text_password, "") == 0) {
  } else if (_strcmp(text_password, "OK") == 0) {
    lv_obj_add_flag(msgbox_password, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clean(msgbox_password);
  }
}

static void app_display_lock_build_msgbox_password(void) {
  static const char *btns[] = { " ", "OK", " ", "" };
  msgbox_password = lv_msgbox_create(lv_scr_act(), " ", "Incorrect password!", btns, false);
  lv_obj_set_size(msgbox_password, 180, 110);
  lv_obj_add_event_cb(msgbox_password, event_msgbox_cb_password, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_center(msgbox_password);
  lv_obj_clear_flag(msgbox_password, LV_OBJ_FLAG_HIDDEN);
}

/* Event handle callback btn password */
static void btn_password_event_handler(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *textarea_password = (lv_obj_t *)lv_event_get_user_data(e);
  const char *char_number = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

  if (_strcmp(char_number, LV_SYMBOL_BACKSPACE) == 0) {
    lv_textarea_del_char(textarea_password);
  } else if (_strcmp(char_number, LV_SYMBOL_OK) == 0) {
    vTaskDelay(500);
    char *text_password = (char *)lv_textarea_get_text(textarea_password);
    char password_login[9];
    uint8_t len = strlen(text_password);
    memcpy_P(password_login, text_password, len);
    for (uint8_t i = len; i < 8; i++) {
      password_login[i] = ' ';
    }
    password_login[8] = '\0';

    if (_strcmp(password_login, password_true) == 0) {
      /* Passwword Correct */
      app_display_lock_main_hidden();
      app_display_main_build();
      app_display_main_on();
      app_display_lock_off();
      app_display_main_home();
    } else {
      lv_textarea_set_text(textarea_password, "");
      /* Passwword Incorrect */
      app_display_lock_build_msgbox_password();
    }
  } else {
    lv_textarea_add_text(textarea_password, char_number);
  }
}

void lock_set_password(char *password) {
  memcpy_P(password_true, password, 8);
  password_true[8] = '\0';
}
