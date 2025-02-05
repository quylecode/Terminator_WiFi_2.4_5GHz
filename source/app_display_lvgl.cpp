#include "app_display_lvgl.h"
#include "touch.h"
#include "WiFi.h"
#include "wifi_conf.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "WDT.h"

#define WIFI_CHANNEL 48
#define WIFI_SSID "Hidden WiFi"
#define WIFI_PASS "verysecurepassword"

WDT wdt_loop_main;

/*******************************************************************************
 * Start of Arduino_GFX setting
 *
 * Arduino_GFX try to find the settings depends on selected board in Arduino IDE
 * Or you can define the display dev kit not in the board list
 * Defalult pin list for non display dev kit:
 * Arduino Nano, Micro and more: CS:  9, DC:  8, RST:  7, BL:  6, SCK: 13, MOSI: 11, MISO: 12
 * ESP32 various dev board     : CS:  5, DC: 27, RST: 33, BL: 22, SCK: 18, MOSI: 23, MISO: nil
 * ESP32-C3 various dev board  : CS:  7, DC:  2, RST:  1, BL:  3, SCK:  4, MOSI:  6, MISO: nil
 * ESP32-S2 various dev board  : CS: 34, DC: 38, RST: 33, BL: 21, SCK: 36, MOSI: 35, MISO: nil
 * ESP32-S3 various dev board  : CS: 40, DC: 41, RST: 42, BL: 48, SCK: 36, MOSI: 35, MISO: nil
 * ESP8266 various dev board   : CS: 15, DC:  4, RST:  2, BL:  5, SCK: 14, MOSI: 13, MISO: 12
 * Raspberry Pi Pico dev board : CS: 17, DC: 27, RST: 26, BL: 28, SCK: 18, MOSI: 19, MISO: 16
 * RTL8720 BW16 old patch core : CS: 18, DC: 17, RST:  2, BL: 23, SCK: 19, MOSI: 21, MISO: 20
 * RTL8720_BW16 Official core  : CS:  9, DC:  8, RST:  6, BL:  3, SCK: 10, MOSI: 12, MISO: 11
 * RTL8722 dev board           : CS: 18, DC: 17, RST: 22, BL: 23, SCK: 13, MOSI: 11, MISO: 12
 * RTL8722_mini dev board      : CS: 12, DC: 14, RST: 15, BL: 13, SCK: 11, MOSI:  9, MISO: 10
 * Seeeduino XIAO dev board    : CS:  3, DC:  2, RST:  1, BL:  0, SCK:  8, MOSI: 10, MISO:  9
 * Teensy 4.1 dev board        : CS: 39, DC: 41, RST: 40, BL: 22, SCK: 13, MOSI: 11, MISO: 12
 ******************************************************************************/
#include <Arduino_GFX_Library.h>
// https://wamingo.net/rgbbgr/
#define GFX_BL GFX_NOT_DEFINED  // default backlight pin, you may replace DF_GFX_BL to actual backlight pin
/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = create_default_Arduino_DataBus();
/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
Arduino_GFX *gfx = new Arduino_ILI9341(bus, GFX_NOT_DEFINED, 1 /* rotation */, false /* IPS */);

static TaskHandle_t inctick_lvgl_task_handler = NULL, test_task_handler = NULL;

static uint32_t buffer_size;
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;

/* Display flushing */
void app_display_lvgl_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
  lv_disp_flush_ready(disp_drv);
}

/*Read the touchpad*/
void app_display_lvgl_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  /*
  if (touch_has_signal()) {
    if (touch_touched()) {
      data->state = LV_INDEV_STATE_PR;
      // Set the coordinates
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_REL;
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
  */
  if (touch_touched()) {
    if (touch_touched()) {
      data->state = LV_INDEV_STATE_PR;
      // Set the coordinates
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_REL;
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void inctick_lvgl_handler(void *pvParameters) {
  while (1) {
    lv_tick_inc(1);
    vTaskDelay(1);
  }
}

void app_display_lvgl_init(void) {
  Serial.begin(115200);
  Serial.println("Arduino_GFX LVGL_Arduino_v8 example ");
  String LVGL_Arduino = String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  Serial.print("Dynamic memory size: ");
  Serial.print(os_get_free_heap_size_arduino());
  Serial.print(" bytes");
  Serial.println();

  wifi_on(RTW_MODE_AP);  // An access point is needed for frame injection to work properly, but we can just hide its ssid
  wifi_start_ap_with_hidden_ssid(WIFI_SSID, RTW_SECURITY_WPA2_AES_PSK, WIFI_PASS, 11, 18, WIFI_CHANNEL);

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  } else {
    Serial.println("gfx->begin() success!");
  }
  gfx->setRotation(1);
  // Init touch device
  touch_init(gfx->width(), gfx->height(), gfx->getRotation());

  lv_init();

  xTaskCreate(inctick_lvgl_handler,
              "inctick_lvgl_handler",
              128,
              NULL,
              2,
              &inctick_lvgl_task_handler);

  buffer_size = gfx->width() * 40;
  Serial.println("LVGL disp_draw_buf heap_caps_malloc");
  disp_draw_buf = (lv_color_t *)malloc(buffer_size * 2);

  if (!disp_draw_buf) {
    Serial.println("LVGL disp_draw_buf allocate failed!");
  } else {
    Serial.println("LVGL disp_draw_buf allocate successfully!");
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, buffer_size);

    /* Initialize the display */
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = gfx->width();
    disp_drv.ver_res = gfx->height();
    disp_drv.flush_cb = app_display_lvgl_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.rotated = 2;
    lv_disp_drv_register(&disp_drv);

    /* Initialize the (dummy) input device driver */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = app_display_lvgl_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    wdt_loop_main.InitWatchdog(2000);  // setup 5s watchdog
    wdt_loop_main.StartWatchdog();
  }
}

void app_display_lvgl_handler(void) {
  lv_timer_handler(); /* let the GUI do its work */
#ifdef DIRECT_MODE
#if defined(CANVAS) || defined(RGB_PANEL)
  gfx->flush();
#else  // !(defined(CANVAS) || defined(RGB_PANEL))
#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, gfx->width(), gfx->height());
#else
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)disp_draw_buf, gfx->width(), gfx->height());
#endif
#endif  // !(defined(CANVAS) || defined(RGB_PANEL))
#else   // !DIRECT_MODE
#ifdef CANVAS
  gfx->flush();
#endif
#endif  // !DIRECT_MODE
  vTaskDelay(5);
  wdt_loop_main.RefreshWatchdog();
}
