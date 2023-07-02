#ifndef HID_H
#define HID_H

#include <stdint.h>
#include "usbd_hid.h"

#define USB_SIZEOF_HID_DESC 0x09

#define USB_HID_DESCRIPTOR_INIT(bcdHID, bCountryCode, bNumDescriptors, bDescriptorType, wItemLength) \
    0x09,                          /* bLength */                                                     \
    HID_DESCRIPTOR_TYPE_HID,       /* bDescriptorType */                                             \
    WBVAL(bcdHID),                 /* bcdHID */                                                      \
    bCountryCode,                  /* bCountryCode */                                                \
    bNumDescriptors,               /* bNumDescriptors */                                             \
    bDescriptorType,               /* bDescriptorType */                                             \
    WBVAL(wItemLength)             /* wItemLength */                                                 \

typedef struct __attribute__((packed)) {
    struct __attribute__((packed)) {
        uint8_t left_ctrl: 1;
        uint8_t left_shift: 1;
        uint8_t left_alt: 1;
        uint8_t left_gui: 1;
        uint8_t right_ctrl: 1;
        uint8_t right_shift: 1;
        uint8_t right_alt: 1;
        uint8_t right_gui: 1;
    } modifier;
    uint8_t reserved;
    uint8_t key[6];
} hid_kbd_report;

typedef struct __attribute__((packed)) {
    uint8_t num_lock: 1;
    uint8_t caps_lock: 1;
    uint8_t scroll_lock: 1;
    uint8_t compose: 1;
    uint8_t kana: 1;
    uint8_t reserved: 3;
} hid_kbd_led_report;

typedef struct __attribute__((packed)) {
    struct __attribute__((packed)) {
        uint8_t left: 1;
        uint8_t right: 1;
        uint8_t middle: 1;
        uint8_t reserved: 5;
    } buttons;
    struct __attribute__((packed)) {
        int16_t x: 12;
        int16_t y: 12;
    } pointer;
    int8_t v_wheel;
    int8_t h_wheel;
} hid_mouse_report;

void hid_init(uint8_t busid);

hid_kbd_led_report hid_get_keyboard_led_status(uint8_t busid);

void hid_send_keyboard_report(uint8_t busid, const hid_kbd_report *report);
void hid_send_mouse_report(uint8_t busid, const hid_mouse_report *report);

#endif //HID_H
