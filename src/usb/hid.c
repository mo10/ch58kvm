#include "usbd_core.h"
#include "hid.h"

// pid.codes Test PID
#define USBD_VID           0x1209
#define USBD_PID           0001
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 0x0409

// HID Keyboard endpoint in
#define HID_KBD_EPIN USB_ENDPOINT_IN(1)
#define HID_KBD_EPIN_SIZE 8
#define HID_KBD_EPIN_INTERVAL 0x01
// HID Keyboard endpoint out (keyboard led)
#define HID_KBD_EPOUT USB_ENDPOINT_OUT(1)
#define HID_KBD_EPOUT_SIZE 1
#define HID_KBD_EPOUT_INTERVAL 0x01
// HID mouse endpoint in
#define HID_MOUSE_EPIN USB_ENDPOINT_IN(2)
#define HID_MOUSE_EPIN_SIZE 3
#define HID_MOUSE_EPIN_INTERVAL 0x01

#define USB_HID_CONFIG_DESC_SIZ ( USB_SIZEOF_CONFIG_DESC    \
                                /* Keyboard interface  */   \
                                + USB_SIZEOF_INTERFACE_DESC \
                                + USB_SIZEOF_HID_DESC       \
                                + USB_SIZEOF_ENDPOINT_DESC  \
                                + USB_SIZEOF_ENDPOINT_DESC  \
                                /* Mouse interface  */      \
                                + USB_SIZEOF_INTERFACE_DESC \
                                + USB_SIZEOF_HID_DESC       \
                                + USB_SIZEOF_ENDPOINT_DESC  \
                              )


// https://www.usb.org/sites/default/files/hid1_11.pdf
// Appendix E: Example USB Descriptors for HID Class Devices
//   E.6 Report Descriptor (Keyboard)
static const uint8_t hid_kbd_report_desc[] = {
        0x05, 0x01,         // Usage Page (Generic Desktop)
        0x09, 0x06,         // Usage (Keyboard)
        0xa1, 0x01,         // Collection (Application)
        0x05, 0x07,         //   Usage Page (Key Codes)
        0x19, 0xe0,         //   Usage Minimum (224)
        0x29, 0xe7,         //   Usage Maximum (231)
        0x15, 0x00,         //   Logical Minimum (0)
        0x25, 0x01,         //   Logical Maximum (1)
        0x75, 0x01,         //   Report Size (1)
        0x95, 0x08,         //   Report Count (8)
        0x81, 0x02,         //   Input (Data, Variable, Absolute) ;Modifier byte
        0x95, 0x01,         //   Report Count (1)
        0x75, 0x08,         //   Report Size (8)
        0x81, 0x01,         //   Input (Constant) ;Reserved byte
        0x95, 0x05,         //   Report Count (5)
        0x75, 0x01,         //   Report Size (1)
        0x05, 0x08,         //   Usage Page (LEDs)
        0x19, 0x01,         //   Usage Minimum (1)
        0x29, 0x05,         //   Usage Maximum (5)
        0x91, 0x02,         //   Output (Data, Variable, Absolute) ;LED report
        0x95, 0x01,         //   Report Count (1)
        0x75, 0x03,         //   Report Size (3)
        0x91, 0x01,         //   Output (Constant) ;LED report padding
        0x95, 0x06,         //   Report Count (6)
        0x75, 0x08,         //   Report Size (8)
        0x15, 0x00,         //   Logical Minimum (0)
        0x25, 0x65,         //   Logical Maximum (101)
        0x05, 0x07,         //   Usage Page (Key Codes)
        0x19, 0x00,         //   Usage Minimum (0)
        0x29, 0x65,         //   Usage Maximum (101)
        0x81, 0x00,         //   Input (Data, Array) ;Key arrays (6 bytes)
        0xc0                // End Collection
};

// Support 3 button + 12bits X 12bits Y + Wheel + Horizontal Wheel
static const uint8_t hid_mouse_report_desc[] = {
        0x05, 0x01,         // Usage Page (Generic Desktop)
        0x09, 0x02,         // Usage (Mouse)
        0xa1, 0x01,         // Collection (Application)
        0x09, 0x01,         //   Usage (Pointer)
        0xa1, 0x00,         //   Collection (Physical)
        // 1byte: 3 Buttons
        0x05, 0x09,         //     Usage Page (Buttons)
        0x19, 0x01,         //     Usage Minimum (01)
        0x29, 0x03,         //     Usage Maximum (03)
        0x15, 0x00,         //     Logical Minimum (0)
        0x25, 0x01,         //     Logical Maximum (1)
        0x95, 0x03,         //     Report Count (3)
        0x75, 0x01,         //     Report Size (1)
        0x81, 0x02,         //     Input (Data, Variable, Absolute) ;3 button bits
        0x95, 0x01,         //     Report Count (1)
        0x75, 0x05,         //     Report Size (5)
        0x81, 0x01,         //     Input (Constant) ;5 bit padding
        // 12bits X + 12bits Y
        0x05, 0x01,         //     Usage Page (Generic Desktop)
        0x16, 0x01, 0xf8,   //     Logical Minimum (-2047)
        0x26, 0xff, 0x07,   //     Logical Maximum (2047)
        0x75, 0x0c,         //     Report Size (12)
        0x95, 0x02,         //     Report Count (2)
        0x09, 0x30,         //     Usage (X)
        0x09, 0x31,         //     Usage (Y)
        0x81, 0x06,         //     Input (Data, Variable, Relative) ;3 position bytes (12bit X & 12bit Y)
        // 1byte Vertical Wheel
        0x15, 0x81,         //     Logical Minimum (-127)
        0x25, 0x7f,         //     Logical Maximum (127)
        0x75, 0x08,         //     Report Size (8)
        0x95, 0x01,         //     Report Count (1)
        0x09, 0x38,         //     Usage (Wheel)
        0x81, 0x06,         //     Input (Data, Variable, Relative) ;1 position byte Wheel
        // 1byte Horizontal Wheel
        0x05, 0x0c,         //     Usage Page (Consumer)
        0x0a, 0x38, 0x02,    //     Usage (AC Pan)
        0x95, 0x01,         //     Report Count (1)
        0x81, 0x06,         //     Input (Data, Variable, Relative) ;1 position byte AC Pan
        0xc0,               //   End Collection
        0xc0                // End Collection
};

static const uint8_t hid_descriptor[] = {
        USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01),
        USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x02, 0x01,
                                   USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
        /* Keyboard interface */
        USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0x03, HID_SUBCLASS_BOOTIF,
                                      HID_PROTOCOL_KEYBOARD, 0x00),
        USB_HID_DESCRIPTOR_INIT(0x0111, 0x00, 0x01, 0x22, sizeof(hid_kbd_report_desc)),
        USB_ENDPOINT_DESCRIPTOR_INIT(HID_KBD_EPIN, USB_ENDPOINT_TYPE_INTERRUPT, HID_KBD_EPIN_SIZE,
                                     HID_KBD_EPIN_INTERVAL),
        USB_ENDPOINT_DESCRIPTOR_INIT(HID_KBD_EPOUT, USB_ENDPOINT_TYPE_INTERRUPT, HID_KBD_EPOUT_SIZE,
                                     HID_KBD_EPOUT_INTERVAL),
        /* Mouse interface */
        USB_INTERFACE_DESCRIPTOR_INIT(0x01, 0x00, 0x01, 0x03, HID_SUBCLASS_BOOTIF,
                                      HID_PROTOCOL_MOUSE, 0x01),
        USB_HID_DESCRIPTOR_INIT(0x0111, 0x00, 0x01, 0x22, sizeof(hid_mouse_report_desc)),
        USB_ENDPOINT_DESCRIPTOR_INIT(HID_MOUSE_EPIN, USB_ENDPOINT_TYPE_INTERRUPT, HID_MOUSE_EPIN_SIZE,
                                     HID_MOUSE_EPIN_INTERVAL),

        ///////////////////////////////////////
        /// string0 language ID
        ///////////////////////////////////////
        USB_LANGID_INIT(USBD_LANGID_STRING),
        ///////////////////////////////////////
        /// string1 manufacturer
        ///////////////////////////////////////
        0x1E,                       /* bLength */
        USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
        'A', 0x00,                  /* wcChar0 */
        'r', 0x00,                  /* wcChar1 */
        'u', 0x00,                  /* wcChar2 */
        'b', 0x00,                  /* wcChar3 */
        'a', 0x00,                  /* wcChar4 */
        ' ', 0x00,                  /* wcChar5 */
        'N', 0x00,                  /* wcChar6 */
        'e', 0x00,                  /* wcChar7 */
        't', 0x00,                  /* wcChar8 */
        'w', 0x00,                  /* wcChar9 */
        'o', 0x00,                  /* wcChar10 */
        'r', 0x00,                  /* wcChar11 */
        'k', 0x00,                  /* wcChar12 */
        's', 0x00,                  /* wcChar13 */
        ///////////////////////////////////////
        /// string2 product
        ///////////////////////////////////////
        0x24,                       /* bLength */
        USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
        'R', 0x00,                  /* wcChar0 */
        'e', 0x00,                  /* wcChar1 */
        'm', 0x00,                  /* wcChar2 */
        'o', 0x00,                  /* wcChar3 */
        't', 0x00,                  /* wcChar4 */
        'e', 0x00,                  /* wcChar5 */
        ' ', 0x00,                  /* wcChar6 */
        'U', 0x00,                  /* wcChar7 */
        'S', 0x00,                  /* wcChar8 */
        'B', 0x00,                  /* wcChar9 */
        ' ', 0x00,                  /* wcChar10 */
        'D', 0x00,                  /* wcChar11 */
        'o', 0x00,                  /* wcChar12 */
        'n', 0x00,                  /* wcChar13 */
        'g', 0x00,                  /* wcChar14 */
        'l', 0x00,                  /* wcChar15 */
        'e', 0x00,                  /* wcChar16 */
        ///////////////////////////////////////
        /// string3 serial
        ///////////////////////////////////////
        0x16,                       /* bLength */
        USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
        '2', 0x00,                  /* wcChar0 */
        '0', 0x00,                  /* wcChar1 */
        '2', 0x00,                  /* wcChar2 */
        '2', 0x00,                  /* wcChar3 */
        '1', 0x00,                  /* wcChar4 */
        '2', 0x00,                  /* wcChar5 */
        '3', 0x00,                  /* wcChar6 */
        '4', 0x00,                  /* wcChar7 */
        '5', 0x00,                  /* wcChar8 */
        '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
        ///////////////////////////////////////
        /// device qualifier descriptor
        ///////////////////////////////////////
        0x0a,
        USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
        0x00,
        0x02,
        0x00,
        0x00,
        0x00,
        0x40,
        0x01,
        0x00,
#endif
        0x00
};

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CONFIG_USBDEV_MAX_BUS][64];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[CONFIG_USBDEV_MAX_BUS][64];

#define HID_STATE_IDLE 0
#define HID_STATE_BUSY 1

/*!< hid state ! Data can be sent only when state is idle  */
static volatile uint8_t hid_state[CONFIG_USBDEV_MAX_BUS] = {0};

void usbd_event_handler(uint8_t busid, uint8_t event) {
    switch (event) {
        case USBD_EVENT_RESET:
            USB_LOG_WRN("USBD_EVENT_RESET");
            hid_state[busid] = HID_STATE_IDLE;
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            /* setup first out ep read transfer */

            usbd_ep_start_read(busid, HID_KBD_EPOUT, read_buffer[busid], 64);
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

static void usbd_hid_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
    hid_state[busid] = HID_STATE_IDLE;
}

static volatile hid_kbd_led_report led_status[CONFIG_USBDEV_MAX_BUS] = {0};

static void usbd_hid_kbd_led_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
    usbd_ep_start_read(busid, HID_KBD_EPOUT, read_buffer[busid], 64);
    led_status[busid] = *(hid_kbd_led_report *) read_buffer[busid];
}

static struct usbd_endpoint hid_kbd_epin = {
        .ep_cb = usbd_hid_in_callback,
        .ep_addr = HID_KBD_EPIN
};

static struct usbd_endpoint hid_kbd_epout = {
        .ep_cb = usbd_hid_kbd_led_callback,
        .ep_addr = HID_KBD_EPOUT
};

static struct usbd_endpoint hid_mouse_epin = {
        .ep_cb = usbd_hid_in_callback,
        .ep_addr = HID_MOUSE_EPIN
};

struct usbd_interface intf0[CONFIG_USBDEV_MAX_BUS];
struct usbd_interface intf1[CONFIG_USBDEV_MAX_BUS];

static struct usbd_bus ch58x_bus[CONFIG_USBDEV_MAX_BUS] = {
        {
                .busid= 0,
                .reg_base = 0x40008000,
        },
        {
                .busid= 1,
                .reg_base = 0x40008400,
        },
};
extern struct usbd_udc_driver ch58xfs_udc_driver;

void hid_init(uint8_t busid) {
    usbd_bus_add_udc(busid, ch58x_bus[busid].reg_base, &ch58xfs_udc_driver, NULL);
    usbd_desc_register(busid, hid_descriptor);
    usbd_add_interface(
            busid,
            usbd_hid_init_intf(
                    &intf0[busid],
                    hid_kbd_report_desc,
                    sizeof(hid_kbd_report_desc))
    );

    usbd_add_interface(
            busid,
            usbd_hid_init_intf(
                    &intf1[busid],
                    hid_mouse_report_desc,
                    sizeof(hid_mouse_report_desc))
    );

    usbd_add_endpoint(busid, &hid_kbd_epin);
    usbd_add_endpoint(busid, &hid_kbd_epout);
    usbd_add_endpoint(busid, &hid_mouse_epin);

    usbd_initialize(busid);
}

hid_kbd_led_report hid_get_keyboard_led_status(uint8_t busid) {
    return led_status[busid];
}

void hid_send_keyboard_report(uint8_t busid, const hid_kbd_report *report) {
    memcpy(write_buffer[busid], report, sizeof(hid_kbd_report));
    int ret = usbd_ep_start_write(busid, HID_KBD_EPIN, write_buffer[busid], sizeof(hid_kbd_report));
    if (ret < 0) {
        return;
    }

    hid_state[busid] = HID_STATE_BUSY;
    while (hid_state[busid] == HID_STATE_BUSY);
}

void hid_send_mouse_report(uint8_t busid, const hid_mouse_report *report) {
    memcpy(write_buffer[busid], report, sizeof(hid_mouse_report));
    int ret = usbd_ep_start_write(busid, HID_MOUSE_EPIN, write_buffer[busid], sizeof(hid_mouse_report));
    if (ret < 0) {
        return;
    }

    hid_state[busid] = HID_STATE_BUSY;
    while (hid_state[busid] == HID_STATE_BUSY);
}