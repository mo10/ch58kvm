#ifndef HID_H
#define HID_H

#include <stdint.h>


void hid_init(uint8_t busid);
void hid_keyboard_test(uint8_t busid);

#endif //HID_H
