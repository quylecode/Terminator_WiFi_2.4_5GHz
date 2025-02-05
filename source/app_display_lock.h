#ifndef APP_DISPLAY_LOCK_H
#define APP_DISPLAY_LOCK_H

#include <Arduino.h>
#include "app_display_main.h"

void app_display_lock_on(void);
void app_display_lock_off(void);
void lock_set_password(char *password);

#endif
