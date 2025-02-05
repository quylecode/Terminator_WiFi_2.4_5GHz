/* uncomment for XPT2046 */
#define TOUCH_XPT2046
#define TOUCH_XPT2046_CS 9
#define TOUCH_XPT2046_INT 2
#define TOUCH_XPT2046_ROTATION 1
#define TOUCH_XPT2046_SAMPLES 10

// Please fill below values from Arduino_GFX Example - TouchCalibration
bool touch_swap_xy = false;
int16_t touch_map_x1 = -1;
int16_t touch_map_x2 = -1;
int16_t touch_map_y1 = -1;
int16_t touch_map_y2 = -1;

int16_t touch_max_x = 0, touch_max_y = 0;
int16_t touch_raw_x = 0, touch_raw_y = 0;
int16_t touch_last_x = 0, touch_last_y = 0;

#if defined(TOUCH_XPT2046)
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
XPT2046_Touchscreen ts(TOUCH_XPT2046_CS, TOUCH_XPT2046_INT);

#elif defined(TOUCH_MODULE_ADDR)  // TouchLib
#include <Wire.h>
#include <TouchLib.h>
TouchLib touch(Wire, TOUCH_SDA, TOUCH_SCL, TOUCH_MODULE_ADDR);

#endif  // TouchLib

void touch_init(int16_t w, int16_t h, uint8_t r) {
  touch_max_x = w - 1;
  touch_max_y = h - 1;
  if (touch_map_x1 == -1) {
    switch (r) {
      case 3:
        touch_swap_xy = true;
        touch_map_x1 = touch_max_x;
        touch_map_x2 = 0;
        touch_map_y1 = 0;
        touch_map_y2 = touch_max_y;
        break;
      case 2:
        touch_swap_xy = false;
        touch_map_x1 = touch_max_x;
        touch_map_x2 = 0;
        touch_map_y1 = touch_max_y;
        touch_map_y2 = 0;
        break;
      case 1:
        touch_swap_xy = true;
        touch_swap_xy = false;
        touch_map_x1 = 4352;
        touch_map_x2 = touch_max_x;
        touch_map_y1 = touch_max_y;
        touch_map_y2 = 5623;
        break;
      default:  // case 0:
        touch_swap_xy = false;
        touch_map_x1 = 0;
        touch_map_x2 = touch_max_x;
        touch_map_y1 = 0;
        touch_map_y2 = touch_max_y;
        break;
    }
  }
  touch_map_x1 = 219;
  touch_map_x2 = 3769;
  touch_map_y1 = 283;
  touch_map_y2 = 3828;

#if defined(TOUCH_XPT2046)
  ts.begin();
  ts.setRotation(TOUCH_XPT2046_ROTATION);
#endif  // TouchLib
}

bool touch_has_signal() {
#if defined(TOUCH_XPT2046)
  return ts.tirqTouched();
#elif defined(TOUCH_MODULE_ADDR)  // TouchLib
  // TODO: implement TOUCH_INT
  return true;
#endif                            // TouchLib
  return false;
}

void translate_touch_raw() {
  if (touch_swap_xy) {
    touch_last_y = map(touch_raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_x = map(touch_raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
  } else {
    touch_last_x = map(touch_raw_x, touch_map_x1, touch_map_x2, 0, touch_max_x);
    touch_last_y = map(touch_raw_y, touch_map_y1, touch_map_y2, 0, touch_max_y);
  }
}

bool touch_touched() {
#if defined(TOUCH_XPT2046)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    touch_raw_x = p.x;
    touch_raw_y = p.y;
    int max_z = p.z;
    int count = 0;
    while ((ts.touched()) && (count < TOUCH_XPT2046_SAMPLES)) {
      count++;
      TS_Point p = ts.getPoint();
      if (p.z > max_z) {
        touch_raw_x = p.x;
        touch_raw_y = p.y;
        max_z = p.z;
      }
    }
    translate_touch_raw();
    return true;
  }
#elif defined(TOUCH_MODULE_ADDR)  // TouchLib
  if (touch.read()) {
    TP_Point t = touch.getPoint(0);
    touch_raw_x = t.x;
    touch_raw_y = t.y;

    touch_last_x = touch_raw_x;
    touch_last_y = touch_raw_y;

    translate_touch_raw();
    return true;
  }
#endif                            // TouchLib
  return false;
}

bool touch_released() {
#if defined(TOUCH_XPT2046)
  return true;
#elif defined(TOUCH_MODULE_ADDR)  // TouchLib
  return false;
#endif                            // TouchLib

  return false;
}
