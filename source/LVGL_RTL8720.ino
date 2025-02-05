#include <lvgl.h>
#include <GTimer.h>
#include <stdio.h>
#include "app_display_lvgl.h"
#include "app_display_lock.h"
#include "app_display_settings.h"

void setup() {
  app_display_lvgl_init();
  app_display_lock_on();
  app_display_settings_load_config();
}

void loop() {
  app_display_lvgl_handler();
}

// https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek_amebad_index.json

// https://ezgif.com/resize


/*

Máy tính AI NVIDIA Jetson Nano Developer Kit B01 + SD64GB + ADAPTER 5V 5A: 3tr
Waveshare 17742 IMX219-83 Stereo Camera: 800k
Màn hình 5 inch HDMI: 250k

*/