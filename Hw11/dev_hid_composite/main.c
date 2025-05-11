/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "hardware/gpio.h"

#include "usb_descriptors.h"

#define UP_BUTTON 15
#define DOWN_BUTTON 17
#define LEFT_BUTTON 14
#define RIGHT_BUTTON 16
#define MODE_BUTTON 13

#define MODE_LED 25

int x_delta;
int y_delta;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  // init buttons
  gpio_init(UP_BUTTON);
  gpio_init(DOWN_BUTTON);
  gpio_init(RIGHT_BUTTON);
  gpio_init(LEFT_BUTTON);
  gpio_init(MODE_BUTTON);

  gpio_set_dir(UP_BUTTON, GPIO_IN);
  gpio_set_dir(DOWN_BUTTON, GPIO_IN);
  gpio_set_dir(RIGHT_BUTTON, GPIO_IN);
  gpio_set_dir(LEFT_BUTTON, GPIO_IN);
  gpio_set_dir(MODE_BUTTON, GPIO_IN);

  gpio_pull_up(UP_BUTTON);
  gpio_pull_up(DOWN_BUTTON);
  gpio_pull_up(RIGHT_BUTTON);
  gpio_pull_up(LEFT_BUTTON);
  gpio_pull_up(MODE_BUTTON);

  // init mode LED  
  gpio_init(MODE_LED);
  gpio_set_dir(MODE_LED, GPIO_OUT);

  // ints to check if button was just pressed or pressed for a long time already
  int up_pressed = 0;
  int down_pressed = 0;
  int right_pressed = 0;
  int left_pressed = 0;

  static int up_time_since_pressed;
  static int down_time_since_pressed;
  static int right_time_since_pressed;
  static int left_time_since_pressed;

  static int circle_time;

  int mode = 0; // mode = 0 is manual control, mode = 1 is circle mode

  while (1)
  {
    tud_task(); // tinyusb device task
    if (gpio_get(MODE_BUTTON) == 0){
      while (gpio_get(MODE_BUTTON) ==0) {}
      sleep_ms(30); // Button debouncing
      if (gpio_get(MODE_BUTTON) == 1){ 
        if (mode == 0) {
          mode = 1;
          y_delta = 0;
          x_delta = 0;
          circle_time = to_ms_since_boot(get_absolute_time());
        }
        else if (mode == 1){
          mode = 0;
          y_delta = 0;
          x_delta = 0;
        }
      }
    }
    if (mode == 0){
      gpio_put(MODE_LED, false);
      // up button functions
      if (gpio_get(UP_BUTTON) == 0 && up_pressed == 0){ //button is first pressed
        y_delta -= 1;
        up_time_since_pressed = to_ms_since_boot(get_absolute_time());
        up_pressed = 1;
      }
      else if ((gpio_get(UP_BUTTON) == 0) && (up_pressed == 1) && ((to_ms_since_boot(get_absolute_time()) - up_time_since_pressed) > 500) && (y_delta > -10)) { // button is pressed for longer than half second 
        y_delta -=1;
        up_time_since_pressed = to_ms_since_boot(get_absolute_time());
      }
      else if ((gpio_get(UP_BUTTON) == 1) && (up_pressed == 1)){ // button is released
        y_delta = 0;
        up_pressed = 0;
      }
      
      // down button functions
      if (gpio_get(DOWN_BUTTON) == 0 && down_pressed == 0){ //button is first pressed
        y_delta += 1;
        down_time_since_pressed = to_ms_since_boot(get_absolute_time());
        down_pressed = 1;
      }
      else if ((gpio_get(DOWN_BUTTON) == 0) && (down_pressed == 1) && ((to_ms_since_boot(get_absolute_time()) - down_time_since_pressed) > 500) && (y_delta < 10)) { // button is pressed for longer than half second 
        y_delta +=1;
        down_time_since_pressed = to_ms_since_boot(get_absolute_time());
      }
      else if ((gpio_get(DOWN_BUTTON) == 1) && (down_pressed == 1)){ // button is released
        y_delta = 0;
        down_pressed = 0;
      }

      // left button functions
      if (gpio_get(LEFT_BUTTON) == 0 && left_pressed == 0){ //button is first pressed
        x_delta -= 1;
        left_time_since_pressed = to_ms_since_boot(get_absolute_time());
        left_pressed = 1;
      }
      else if ((gpio_get(LEFT_BUTTON) == 0) && (left_pressed == 1) && ((to_ms_since_boot(get_absolute_time()) - left_time_since_pressed) > 500) && (x_delta > -10)) { // button is pressed for longer than half second 
        x_delta -=1;
        left_time_since_pressed = to_ms_since_boot(get_absolute_time());
      }
      else if ((gpio_get(LEFT_BUTTON) == 1) && (left_pressed == 1)){ // button is released
        x_delta = 0;
        left_pressed = 0;
      }

      // right button functions
      if (gpio_get(RIGHT_BUTTON) == 0 && right_pressed == 0){ //button is first pressed
        x_delta += 1;
        right_time_since_pressed = to_ms_since_boot(get_absolute_time());
        right_pressed = 1;
      }
      else if ((gpio_get(RIGHT_BUTTON) == 0) && (right_pressed == 1) && ((to_ms_since_boot(get_absolute_time()) - right_time_since_pressed) > 500) && (x_delta < 10)) { // button is pressed for longer than half second 
        x_delta +=1;
        right_time_since_pressed = to_ms_since_boot(get_absolute_time());
      }
      else if ((gpio_get(RIGHT_BUTTON) == 1) && (right_pressed == 1)){ // button is released
        x_delta = 0;
        right_pressed = 0;
      }
    }
    else if (mode == 1){
      gpio_put(MODE_LED, true);
      x_delta = (int) (4 * sin((to_ms_since_boot(get_absolute_time()) - circle_time)/1000.0));
      y_delta = (int) (4 * cos((to_ms_since_boot(get_absolute_time()) - circle_time)/1000.0));
    }
    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      // Gets x_delta and y_delta from main and moves cursor
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00,x_delta, y_delta, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
