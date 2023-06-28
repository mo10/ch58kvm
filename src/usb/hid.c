#include "usbd_core.h"
#include "usbd_hid.h"
#include "hid.h"

// pid.codes Test PID
#define USBD_VID           0x1209
#define USBD_PID           0001

#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

// HID Keyboard endpoint in
#define HID_KBD_EPIN USB_ENDPOINT_IN(1)
#define HID_KBD_EPIN_SIZE 8
#define HID_KBD_EPIN_INTERVAL 1
// HID Keyboard endpoint out
#define HID_KBD_EPOUT USB_ENDPOINT_OUT(1)
#define HID_KBD_EPOUT_SIZE 8
#define HID_KBD_EPOUT_INTERVAL 1

#define HID_KBD_REPORT_DESC_SIZE 63

#define USB_HID_CONFIG_DESC_SIZ (USB_SIZEOF_CONFIG_DESC     \
                                + USB_SIZEOF_INTERFACE_DESC \
                                + 9 /* HID_DESC */          \
                                + USB_SIZEOF_ENDPOINT_DESC  \
                                + USB_SIZEOF_ENDPOINT_DESC)

#define HID_KEYBOARD_REPORT_DESC_SIZE 63

static const uint8_t hid_descriptor[] = {
        USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01),
        USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
        USB_INTERFACE_DESCRIPTOR_INIT(0x00, 0x00, 0x02, 0x03, HID_SUBCLASS_BOOTIF, HID_PROTOCOL_KEYBOARD, 0x00),
        /******************** Descriptor of HID keyboard ********************/
        0x09,                          /* bLength: HID Descriptor size */
        HID_DESCRIPTOR_TYPE_HID,       /* bDescriptorType: HID */
        0x11,                          /* bcdHID: HID Class Spec release number */
        0x01,
        0x00,                          /* bCountryCode: Hardware target country */
        0x01,                          /* bNumDescriptors: Number of HID class descriptors to follow */
        0x22,                          /* bDescriptorType */
        HID_KEYBOARD_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
        0x00,
        /******************** Descriptor of endpoint in ********************/
        USB_ENDPOINT_DESCRIPTOR_INIT(HID_KBD_EPIN, USB_ENDPOINT_TYPE_INTERRUPT, HID_KBD_EPIN_SIZE,
                                     HID_KBD_EPIN_INTERVAL),
        USB_ENDPOINT_DESCRIPTOR_INIT(HID_KBD_EPOUT, USB_ENDPOINT_TYPE_INTERRUPT, HID_KBD_EPOUT_SIZE,
                                     HID_KBD_EPOUT_INTERVAL),
        ///////////////////////////////////////
        /// string0 descriptor
        ///////////////////////////////////////
        USB_LANGID_INIT(USBD_LANGID_STRING),
        ///////////////////////////////////////
        /// string1 descriptor
        ///////////////////////////////////////
        0x14,                       /* bLength */
        USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
        'C', 0x00,                  /* wcChar0 */
        'h', 0x00,                  /* wcChar1 */
        'e', 0x00,                  /* wcChar2 */
        'r', 0x00,                  /* wcChar3 */
        'r', 0x00,                  /* wcChar4 */
        'y', 0x00,                  /* wcChar5 */
        'U', 0x00,                  /* wcChar6 */
        'S', 0x00,                  /* wcChar7 */
        'B', 0x00,                  /* wcChar8 */
        ///////////////////////////////////////
        /// string2 descriptor
        ///////////////////////////////////////
        0x26,                       /* bLength */
        USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
        'C', 0x00,                  /* wcChar0 */
        'h', 0x00,                  /* wcChar1 */
        'e', 0x00,                  /* wcChar2 */
        'r', 0x00,                  /* wcChar3 */
        'r', 0x00,                  /* wcChar4 */
        'y', 0x00,                  /* wcChar5 */
        'U', 0x00,                  /* wcChar6 */
        'S', 0x00,                  /* wcChar7 */
        'B', 0x00,                  /* wcChar8 */
        ' ', 0x00,                  /* wcChar9 */
        'H', 0x00,                  /* wcChar10 */
        'I', 0x00,                  /* wcChar11 */
        'D', 0x00,                  /* wcChar12 */
        ' ', 0x00,                  /* wcChar13 */
        'D', 0x00,                  /* wcChar14 */
        'E', 0x00,                  /* wcChar15 */
        'M', 0x00,                  /* wcChar16 */
        'O', 0x00,                  /* wcChar17 */
        ///////////////////////////////////////
        /// string3 descriptor
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

static const uint8_t hid_keyboard_report_desc[HID_KEYBOARD_REPORT_DESC_SIZE] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x06, // USAGE (Keyboard)
        0xa1, 0x01, // COLLECTION (Application)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
        0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0x01, // LOGICAL_MAXIMUM (1)
        0x75, 0x01, // REPORT_SIZE (1)
        0x95, 0x08, // REPORT_COUNT (8)
        0x81, 0x02, // INPUT (Data,Var,Abs)
        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x08, // REPORT_SIZE (8)
        0x81, 0x03, // INPUT (Cnst,Var,Abs)
        0x95, 0x05, // REPORT_COUNT (5)
        0x75, 0x01, // REPORT_SIZE (1)
        0x05, 0x08, // USAGE_PAGE (LEDs)
        0x19, 0x01, // USAGE_MINIMUM (Num Lock)
        0x29, 0x05, // USAGE_MAXIMUM (Kana)
        0x91, 0x02, // OUTPUT (Data,Var,Abs)
        0x95, 0x01, // REPORT_COUNT (1)
        0x75, 0x03, // REPORT_SIZE (3)
        0x91, 0x03, // OUTPUT (Cnst,Var,Abs)
        0x95, 0x06, // REPORT_COUNT (6)
        0x75, 0x08, // REPORT_SIZE (8)
        0x15, 0x00, // LOGICAL_MINIMUM (0)
        0x25, 0xFF, // LOGICAL_MAXIMUM (255)
        0x05, 0x07, // USAGE_PAGE (Keyboard)
        0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
        0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
        0x81, 0x00, // INPUT (Data,Ary,Abs)
        0xc0        // END_COLLECTION
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

static void usbd_hid_kbd_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
    USB_LOG_RAW("actual in len:%lu\r\n", nbytes);
    hid_state[busid] = HID_STATE_IDLE;
}

static void usbd_hid_kbd_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes) {
    USB_LOG_RAW("actual out len:%lu\r\n", nbytes);
    usbd_ep_start_read(busid, HID_KBD_EPOUT, read_buffer[busid], 64);
}

static struct usbd_endpoint hid_kbd_epin = {
        .ep_cb = usbd_hid_kbd_in_callback,
        .ep_addr = HID_KBD_EPIN
};

static struct usbd_endpoint hid_kbd_epout = {
        .ep_cb = usbd_hid_kbd_out_callback,
        .ep_addr = HID_KBD_EPOUT
};

struct usbd_interface intf0[CONFIG_USBDEV_MAX_BUS];


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
    usbd_add_interface(busid,
                       usbd_hid_init_intf(&intf0[busid],
                                          hid_keyboard_report_desc,
                                          HID_KEYBOARD_REPORT_DESC_SIZE));
    usbd_add_endpoint(busid, &hid_kbd_epin);
    usbd_add_endpoint(busid, &hid_kbd_epout);

    usbd_initialize(busid);
}

void hid_keyboard_test(uint8_t busid) {
    const uint8_t sendbuffer[8] = {0x00, 0x00, HID_KBD_USAGE_A, 0x00, 0x00, 0x00, 0x00, 0x00};

    memcpy(write_buffer[busid], sendbuffer, 8);
    int ret = usbd_ep_start_write(busid, HID_KBD_EPIN, write_buffer[busid], 8);
    if (ret < 0) {
        return;
    }
    hid_state[busid] = HID_STATE_BUSY;
    while (hid_state[busid] == HID_STATE_BUSY) {
    }
}
