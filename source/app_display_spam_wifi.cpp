#include "app_display_spam_wifi.h"
#include "WDT.h"

extern WDT wdt_loop_main;

#define WIFI_SPAM_DEFAULT 0
#define WIFI_SPAM_NUMBER 1
#define WIFI_SPAM_CHARACTER 2
#define WIFI_SPAM_LETTER 3

static TaskHandle_t wifi_spam_attack_task_handler = NULL;

static lv_obj_t *wifi_spam;
static lv_obj_t *wifi_spam_close_btn;

static lv_obj_t *wifi_spam_random_btn;
static lv_obj_t *wifi_spam_list_create_btn;

static lv_obj_t *wifi_spam_random_ui;

static lv_obj_t *wifi_spam_list_create_ui;
static lv_obj_t *wifi_spam_list_create_ui_close_btn;
static lv_obj_t *list_wifi_spam;
static lv_obj_t *keyboard_list_wifi_spam;
static const char *txt_list_wifi_spam;
static uint16_t txt_list_length;

static lv_obj_t *wifi_spam_attack_start_btn;
static lv_obj_t *wifi_spam_attack_stop_btn;

static uint8_t wifi_spam_init = 0;
static uint8_t wifi_spam_list_create_init = 0;
static uint8_t wifi_spam_random_status = 0;
static uint8_t wifi_spam_list_status = 0;

static lv_obj_t *wifi_spam_attack_noti;
static lv_timer_t *wifi_spam_timer_noti;

static lv_obj_t *wifi_spam_random_spinner_load;
static lv_obj_t *wifi_spam_list_spinner_load;

void app_display_spam_wifi_list_create_build(void);

static void wifi_spam_btn_event_cb(lv_event_t *e);
static void wifi_spam_attack_task_init(void);

// ===== Settings ===== //
const uint8_t channels[] = { 1, 6, 11 };  // used Wi-Fi channels (available: 1-14)
const bool wpa2 = true;                   // WPA2 networks
const bool appendSpaces = true;           // makes all SSIDs 32 characters long to improve performance

volatile uint8_t wifi_spam_style = 0;
volatile uint8_t wifi_spam_number = 0;
volatile uint8_t wifi_spam_length_name = 0;

static char *name_wifi_create = NULL;

/*
  SSIDs:
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
const char ssids[] PROGMEM = {
  "Mom Use This One\n"
  "Abraham Linksys\n"
  "Benjamin FrankLAN\n"
  "Martin Router King\n"
  "John Wilkes Bluetooth\n"
  "Pretty Fly for a Wi-Fi\n"
  "Bill Wi the Science Fi\n"
  "I Believe Wi Can Fi\n"
  "Tell My Wi-Fi Love Her\n"
  "No More Mister Wi-Fi\n"
  "LAN Solo\n"
  "The LAN Before Time\n"
  "Silence of the LANs\n"
  "House LANister\n"
  "Winternet Is Coming\n"
  "Ping’s Landing\n"
  "The Ping in the North\n"
  "This LAN Is My LAN\n"
  "Get Off My LAN\n"
  "The Promised LAN\n"
  "The LAN Down Under\n"
  "FBI Surveillance Van 4\n"
  "Area 51 Test Site\n"
  "Drive-By Wi-Fi\n"
  "Planet Express\n"
  "Wu Tang LAN\n"
  "Darude LANstorm\n"
  "Never Gonna Give You Up\n"
  "Hide Yo Kids, Hide Yo Wi-Fi\n"
  "Loading…\n"
  "Searching…\n"
  "VIRUS.EXE\n"
  "Virus-Infected Wi-Fi\n"
  "Starbucks Wi-Fi\n"
  "Text ###-#### for Password\n"
  "Yell ____ for Password\n"
  "The Password Is 1234\n"
  "Free Public Wi-Fi\n"
  "No Free Wi-Fi Here\n"
  "Get Your Own Damn Wi-Fi\n"
  "It Hurts When IP\n"
  "Dora the Internet Explorer\n"
  "404 Wi-Fi Unavailable\n"
  "Porque-Fi\n"
  "Titanic Syncing\n"
  "Test Wi-Fi Please Ignore\n"
  "Drop It Like It’s Hotspot\n"
  "Life in the Fast LAN\n"
  "The Creep Next Door\n"
  "Ye Olde Internet\n"
};

char *wifi_spam_create_name_wifi(uint8_t mode, uint8_t length_name, uint8_t number);

// run-time variables
static char emptySSID[32];
static uint8_t channelIndex = 0;
static uint8_t macAddr[6];
static uint8_t wifi_channel = 1;
static uint32_t currentTime = 0;
static uint32_t packetSize = 0;
static uint32_t packetCounter = 0;
static uint32_t attackTime = 0;
static uint32_t packetRateTime = 0;

// beacon frame definition
static uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00,              // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  // Source

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00,                                      // Fragment & sequence number (will be done by the SDK)
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00,  // Timestamp
  /* 32 - 33 */ 0xe8, 0x03,                                      // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00,                                      // capabilities Tnformation

  // Tagged parameters

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20,  // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,  // SSID

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08,  // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82,             // 1(B)
  /* 73 */ 0x84,             // 2(B)
  /* 74 */ 0x8b,             // 5.5(B)
  /* 75 */ 0x96,             // 11(B)
  /* 76 */ 0x24,             // 18
  /* 77 */ 0x30,             // 24
  /* 78 */ 0x48,             // 36
  /* 79 */ 0x6c,             // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01,  // Channel set, length
  /* 82 */ 0x01,             // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, /*Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices*/
  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};

// goes to next channel
void nextChannel(void) {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex > sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      wifi_set_channel(wifi_channel);
    }
  }
}

// generates random MAC
void randomMac(void) {
  for (int i = 0; i < 6; i++)
    macAddr[i] = random(256);
}

void wifi_spam_attack_init(void) {
  // create empty SSID
  for (int i = 0; i < 32; i++)
    emptySSID[i] = ' ';
  // for random generator
  randomSeed(1);

  // set packetSize
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31;
  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }

  // generate random mac address
  randomMac();

  //change WiFi mode
  wifi_off();
  wifi_on(RTW_MODE_AP);

  // set channel
  wifi_set_channel(channels[0]);
}

void wifi_spam_attack_start(const char *wifi_list, uint16_t txt_list_length) {
  // put your main code here, to run repeatedly:
  currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = txt_list_length;
    bool sent = false;

    // go to next channel
    nextChannel();

    while (i < ssidsLen) {
      // read out next SSID
      j = 0;
      do {
        // tmp = pgm_read_byte(ssids + i + j);
        tmp = *(wifi_list + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // reset SSID
      memcpy(&beaconPacket[38], emptySSID, 32);

      // write new SSID into beacon frame
      memcpy_P(&beaconPacket[38], wifi_list + i, ssidLen);

      // set channel for beacon frame
      beaconPacket[82] = wifi_channel;

      // send packet
      if (appendSpaces) {
        for (int k = 0; k < 3; k++) {
          wifi_tx_raw_frame(beaconPacket, packetSize);
          packetCounter++;
          vTaskDelay(1);
        }
      }

      i += j;
    }
  }

  // show packet-rate each second
  if (currentTime - packetRateTime > 1000) {
    packetRateTime = currentTime;
    Serial.print("Packets/s: ");
    Serial.println(packetCounter);
    packetCounter = 0;
  }
}

void app_display_spam_wifi_build(void) {
  if (wifi_spam_init == 1)
    return;
  wifi_spam_init = 1;

  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  /* WiFi Spam Screen Main */
  wifi_spam = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(wifi_spam, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(wifi_spam, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(wifi_spam, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(wifi_spam, &style_background, 0);

  lv_obj_t *wifi_spam_label = lv_label_create(wifi_spam);
  lv_label_set_text(wifi_spam_label, LV_SYMBOL_WIFI " WiFi Spam");
  lv_obj_align(wifi_spam_label, LV_ALIGN_TOP_LEFT, 0, 0);

  wifi_spam_close_btn = lv_btn_create(wifi_spam);
  lv_obj_set_size(wifi_spam_close_btn, 30, 30);
  lv_obj_align(wifi_spam_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(wifi_spam_close_btn, wifi_spam_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol = lv_label_create(wifi_spam_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  wifi_spam_random_btn = lv_btn_create(wifi_spam);
  lv_obj_set_size(wifi_spam_random_btn, 160, 40);
  lv_obj_align(wifi_spam_random_btn, LV_ALIGN_CENTER, 0, -30);
  lv_obj_add_event_cb(wifi_spam_random_btn, wifi_spam_btn_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *wifi_spam_random_btn_label = lv_label_create(wifi_spam_random_btn);
  lv_label_set_text(wifi_spam_random_btn_label, "WiFi Spam Random");
  lv_obj_center(wifi_spam_random_btn_label);
  lv_obj_set_style_text_color(wifi_spam_random_btn_label, lv_color_black(), 0);

  wifi_spam_list_create_btn = lv_btn_create(wifi_spam);
  lv_obj_set_size(wifi_spam_list_create_btn, 160, 40);
  lv_obj_align(wifi_spam_list_create_btn, LV_ALIGN_CENTER, 0, 30);
  lv_obj_add_event_cb(wifi_spam_list_create_btn, wifi_spam_btn_event_cb, LV_EVENT_ALL, NULL);

  lv_obj_t *wifi_spam_list_create_btn_label = lv_label_create(wifi_spam_list_create_btn);
  lv_label_set_text(wifi_spam_list_create_btn_label, "WiFi Spam List");
  lv_obj_center(wifi_spam_list_create_btn_label);
  lv_obj_set_style_text_color(wifi_spam_list_create_btn_label, lv_color_black(), 0);

  wifi_spam_random_spinner_load = lv_spinner_create(wifi_spam, 1000, 60);
  lv_obj_set_size(wifi_spam_random_spinner_load, 45, 45);
  lv_obj_align(wifi_spam_random_spinner_load, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(wifi_spam_random_spinner_load, LV_OBJ_FLAG_HIDDEN);
}

void app_display_spam_wifi_on(void) {
  lv_obj_clear_flag(wifi_spam, LV_OBJ_FLAG_HIDDEN);
}

void app_display_spam_wifi_off(void) {
  if (wifi_spam_init == 1) {
    lv_obj_del_async(wifi_spam);
    wifi_spam_init = 0;
  }
  if (wifi_spam_list_create_init == 1) {
    lv_obj_del_async(wifi_spam_list_create_ui);
    wifi_spam_list_create_init = 0;
  }
}

uint8_t app_display_spam_wifi_get_active(void) {
  if (wifi_spam_init == 1) {
    return lv_obj_has_flag(wifi_spam, LV_OBJ_FLAG_HIDDEN) ? 0 : 1;
  } else {
    return 1;
  }
}

static void timer_cb(lv_timer_t *timer) {
  LV_UNUSED(timer);
  lv_obj_clear_state(wifi_spam_close_btn, LV_STATE_DISABLED);
  lv_obj_add_flag(wifi_spam_attack_noti, LV_OBJ_FLAG_HIDDEN);
  lv_obj_del_async(wifi_spam_attack_noti);
  lv_timer_del(wifi_spam_timer_noti);
}

static void wifi_spam_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_spam_close_btn) {
      lv_obj_add_flag(wifi_spam, LV_OBJ_FLAG_HIDDEN);
    }
    if (btn == wifi_spam_list_create_btn) {
      if (wifi_spam_random_status == 1) {
        wifi_spam_attack_noti = lv_msgbox_create(lv_scr_act(), "Warning", "WiFi Spam Random is running. Please turn off WiFi Spam Random to turn on WiFi Spam List !", NULL, false);
        lv_obj_center(wifi_spam_attack_noti);
        lv_obj_set_size(wifi_spam_attack_noti, 270, 120);
        lv_obj_add_state(wifi_spam_close_btn, LV_STATE_DISABLED);
        wifi_spam_timer_noti = lv_timer_create(timer_cb, 2000, NULL);
      } else {
        app_display_spam_wifi_list_create_build();
      }
    }
    if (btn == wifi_spam_random_btn) {
      if (wifi_spam_list_status == 1) {
        wifi_spam_attack_noti = lv_msgbox_create(lv_scr_act(), "Warning", "WiFi Spam List is running. Please turn off WiFi Spam List to turn on WiFi Spam Random !", NULL, false);
        lv_obj_center(wifi_spam_attack_noti);
        lv_obj_set_size(wifi_spam_attack_noti, 270, 120);
        lv_obj_add_state(wifi_spam_close_btn, LV_STATE_DISABLED);
        wifi_spam_timer_noti = lv_timer_create(timer_cb, 2000, NULL);
        return;
      }
      if (wifi_spam_random_status == 0) {
        lv_obj_set_style_bg_color(wifi_spam_random_btn, lv_palette_main(LV_PALETTE_RED), 0);
        wifi_spam_random_status = 1;
        wifi_spam_attack_task_init();
        /* Notification attack */  // 35
        wifi_spam_attack_noti = lv_msgbox_create(lv_scr_act(), "Message                            ", "WiFi Spam Random On !", NULL, false);
        lv_obj_center(wifi_spam_attack_noti);
        lv_obj_set_size(wifi_spam_attack_noti, 210, 80);
        lv_obj_add_state(wifi_spam_close_btn, LV_STATE_DISABLED);
        wifi_spam_timer_noti = lv_timer_create(timer_cb, 2000, NULL);

        lv_obj_clear_flag(wifi_spam_random_spinner_load, LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_set_style_bg_color(wifi_spam_random_btn, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
        wifi_spam_random_status = 0;
        if (wifi_spam_attack_task_handler != NULL) {
          vTaskDelete(wifi_spam_attack_task_handler);
          wifi_spam_attack_task_handler = NULL;
        }
        /* Notification attack */
        wifi_spam_attack_noti = lv_msgbox_create(lv_scr_act(), "Message                            ", "WiFi Spam Random Off !", NULL, false);
        lv_obj_center(wifi_spam_attack_noti);
        lv_obj_set_size(wifi_spam_attack_noti, 210, 80);
        lv_obj_add_state(wifi_spam_close_btn, LV_STATE_DISABLED);
        wifi_spam_timer_noti = lv_timer_create(timer_cb, 2000, NULL);

        lv_obj_add_flag(wifi_spam_random_spinner_load, LV_OBJ_FLAG_HIDDEN);

        /* Free buf */
        if (name_wifi_create != NULL) {
          free(name_wifi_create);
        }
      }
    }
  }
}

static void wifi_spam_list_create_ui_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);

  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_spam_list_create_ui_close_btn) {
      lv_obj_add_flag(wifi_spam_list_create_ui, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

static void text_wifi_spam_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    if (!wifi_spam_list_status) {
      lv_keyboard_set_textarea(kb, ta);
      lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
  }
  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  }
}

static void keyboard_text_wifi_spam_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *kb = lv_event_get_target(e);
  const char *txt_kb = lv_keyboard_get_btn_text(kb, lv_keyboard_get_selected_btn(kb));
  if (_strcmp(txt_kb, LV_SYMBOL_OK) == 0) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_event_send(list_wifi_spam, LV_EVENT_DEFOCUSED, NULL);
    lv_event_send(wifi_spam_list_create_ui, LV_EVENT_CLICKED, NULL);
  }
}

static void wifi_spam_attack_btn_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *btn = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    if (btn == wifi_spam_attack_start_btn) {
      lv_obj_add_state(wifi_spam_attack_start_btn, LV_STATE_DISABLED);
      lv_obj_clear_state(wifi_spam_attack_stop_btn, LV_STATE_DISABLED);
      txt_list_wifi_spam = lv_textarea_get_text(list_wifi_spam);
      for (uint16_t i = 0; i < 65535; i++) {
        if (*(txt_list_wifi_spam + i) != NULL) {
          Serial.print(*(txt_list_wifi_spam + i));
        } else {
          Serial.println(i);
          txt_list_length = i;
          break;
        }
      }
      if (wifi_spam_attack_task_handler == NULL) {
        wifi_spam_attack_task_init();
        wifi_spam_list_status = 1;
      }
      lv_obj_clear_flag(wifi_spam_list_spinner_load, LV_OBJ_FLAG_HIDDEN);
    }
    if (btn == wifi_spam_attack_stop_btn) {
      lv_obj_clear_state(wifi_spam_attack_start_btn, LV_STATE_DISABLED);
      lv_obj_add_state(wifi_spam_attack_stop_btn, LV_STATE_DISABLED);
      if (wifi_spam_attack_task_handler != NULL) {
        vTaskDelete(wifi_spam_attack_task_handler);
        wifi_spam_attack_task_handler = NULL;
      }
      wifi_spam_list_status = 0;
      lv_obj_add_flag(wifi_spam_list_spinner_load, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void app_display_spam_wifi_list_create_build(void) {
  if (wifi_spam_list_create_init == 1) {
    lv_obj_clear_flag(wifi_spam_list_create_ui, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  wifi_spam_list_create_init = 1;

  static lv_style_t style_background;
  lv_style_init(&style_background);
  lv_style_set_bg_color(&style_background, lv_color_hex(0x000000));

  /* Create main wifi spam list */
  wifi_spam_list_create_ui = lv_obj_create(lv_scr_act());
  lv_obj_add_flag(wifi_spam_list_create_ui, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_size(wifi_spam_list_create_ui, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(wifi_spam_list_create_ui, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(wifi_spam_list_create_ui, &style_background, 0);

  lv_obj_t *wifi_spam_list_create_ui_label = lv_label_create(wifi_spam_list_create_ui);
  lv_label_set_text(wifi_spam_list_create_ui_label, LV_SYMBOL_WIFI " WiFi Spam List");
  lv_obj_align(wifi_spam_list_create_ui_label, LV_ALIGN_TOP_LEFT, 0, 0);

  wifi_spam_list_create_ui_close_btn = lv_btn_create(wifi_spam_list_create_ui);
  lv_obj_set_size(wifi_spam_list_create_ui_close_btn, 30, 30);
  lv_obj_align(wifi_spam_list_create_ui_close_btn, LV_ALIGN_TOP_RIGHT, 0, -10);
  lv_obj_add_event_cb(wifi_spam_list_create_ui_close_btn, wifi_spam_list_create_ui_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_close_symbol = lv_label_create(wifi_spam_list_create_ui_close_btn);
  lv_label_set_text(btn_close_symbol, LV_SYMBOL_CLOSE);
  lv_obj_center(btn_close_symbol);

  // Spam WiFi Attack Button
  wifi_spam_attack_start_btn = lv_btn_create(wifi_spam_list_create_ui);
  lv_obj_set_size(wifi_spam_attack_start_btn, 70, 30);
  lv_obj_align(wifi_spam_attack_start_btn, LV_ALIGN_CENTER, -50, 40);
  lv_obj_add_event_cb(wifi_spam_attack_start_btn, wifi_spam_attack_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_attack_start = lv_label_create(wifi_spam_attack_start_btn);
  lv_label_set_text(btn_attack_start, "START");
  lv_obj_center(btn_attack_start);
  lv_obj_set_style_text_color(btn_attack_start, lv_color_black(), 0);

  wifi_spam_attack_stop_btn = lv_btn_create(wifi_spam_list_create_ui);
  lv_obj_set_size(wifi_spam_attack_stop_btn, 70, 30);
  lv_obj_align(wifi_spam_attack_stop_btn, LV_ALIGN_CENTER, 50, 40);
  lv_obj_add_event_cb(wifi_spam_attack_stop_btn, wifi_spam_attack_btn_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_t *btn_attack_stop = lv_label_create(wifi_spam_attack_stop_btn);
  lv_label_set_text(btn_attack_stop, "STOP");
  lv_obj_center(btn_attack_stop);
  lv_obj_add_state(wifi_spam_attack_stop_btn, LV_STATE_DISABLED);
  lv_obj_set_style_text_color(btn_attack_stop, lv_color_black(), 0);

  /*Create a keyboard to use it with an of the text areas*/
  keyboard_list_wifi_spam = lv_keyboard_create(wifi_spam_list_create_ui);
  lv_obj_set_width(keyboard_list_wifi_spam, lv_pct(100));
  lv_obj_add_flag(keyboard_list_wifi_spam, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(keyboard_list_wifi_spam, keyboard_text_wifi_spam_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_align(keyboard_list_wifi_spam, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_scrollbar_mode(keyboard_list_wifi_spam, LV_SCROLLBAR_MODE_OFF);

  /*Create a text area. The keyboard will write here*/
  list_wifi_spam = lv_textarea_create(wifi_spam_list_create_ui);
  lv_obj_align(list_wifi_spam, LV_ALIGN_TOP_MID, 0, 24);
  lv_obj_add_event_cb(list_wifi_spam, text_wifi_spam_event_cb, LV_EVENT_ALL, keyboard_list_wifi_spam);
  lv_obj_set_size(list_wifi_spam, 250, 80);
  lv_textarea_set_placeholder_text(list_wifi_spam, "Please input name wifi spam... ");

  wifi_spam_list_spinner_load = lv_spinner_create(wifi_spam_list_create_ui, 1000, 60);
  lv_obj_set_size(wifi_spam_list_spinner_load, 45, 45);
  lv_obj_align(wifi_spam_list_spinner_load, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(wifi_spam_list_spinner_load, LV_OBJ_FLAG_HIDDEN);

  lv_obj_clear_flag(wifi_spam_list_create_ui, LV_OBJ_FLAG_HIDDEN);
}

static void wifi_spam_attack_task(void *pvParameters) {
  wifi_spam_attack_init();

  if (wifi_spam_style != WIFI_SPAM_DEFAULT) {
    name_wifi_create = wifi_spam_create_name_wifi(wifi_spam_style, wifi_spam_length_name, wifi_spam_number);
  }

  while (1) {
    if (wifi_spam_random_status == 1) {
      if (wifi_spam_style == WIFI_SPAM_DEFAULT) {
        wifi_spam_attack_start(ssids, strlen_P(ssids));
      } else {
        wifi_spam_attack_start(name_wifi_create, (wifi_spam_length_name + 1) * wifi_spam_number);
      }
    } else {
      wifi_spam_attack_start(txt_list_wifi_spam, txt_list_length);
    }
    vTaskDelay(50);
  }
}

static void wifi_spam_attack_task_init(void) {
  xTaskCreate(wifi_spam_attack_task, "wifi_spam_attack_task", 1024 + 3200, NULL, 5, &wifi_spam_attack_task_handler);
}

void wifi_spam_set_spam_style(uint8_t style) {
  wifi_spam_style = style;
}

void wifi_spam_set_spam_number(uint8_t number) {
  wifi_spam_number = number;
}

void wifi_spam_set_length_name_wifi(uint8_t length) {
  wifi_spam_length_name = length;
}

char *wifi_spam_create_name_wifi(uint8_t mode, uint8_t length_name, uint8_t number) {
  char *name_wifi = (char *)malloc((length_name + 1) * number);
  char *temp = name_wifi;
  for (uint8_t i = 0; i < number; i++) {
    for (uint8_t j = 0; j < length_name; j++) {
      if (mode == WIFI_SPAM_CHARACTER) {
        *name_wifi = (rand() % 16) + 32;
        name_wifi++;
      } else if (mode == WIFI_SPAM_NUMBER) {
        *name_wifi = (rand() % 10) + 48;
        name_wifi++;
      } else if (mode == WIFI_SPAM_LETTER) {
        *name_wifi = (rand() % 63) + 64;
        name_wifi++;
      }
    }
    *name_wifi = '\n';
    name_wifi++;
  }
  return temp;
}
