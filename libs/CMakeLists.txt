cmake_minimum_required(VERSION 3.10)

set(CHERRY_USB_ROOT ${CMAKE_CURRENT_LIST_DIR}/CherryUSB)

set(CHERRY_USB_SRCS
        ${CHERRY_USB_ROOT}/core/usbd_core.c
        ${CHERRY_USB_ROOT}/core/usbd_udc.c
        ${CHERRY_USB_ROOT}/port/ch32/usb_dc_ch58x_usbfs.c)

# USB Class
file(GLOB CHERRY_USB_CLASS_SRCS
        ${CHERRY_USB_ROOT}/class/hid/usbd_hid.c
        )

# RVMSIS
set(TARGET_C_SOURCES
        ${CHERRY_USB_SRCS}
        ${CHERRY_USB_CLASS_SRCS})

set(TARGET_C_INCLUDES
        ${CHERRY_USB_ROOT}/common
        ${CHERRY_USB_ROOT}/core
        ${CHERRY_USB_ROOT}/port/ch32
        ${CHERRY_USB_ROOT}/class/hid
        )

add_library(CherryUSB STATIC ${TARGET_C_SOURCES})

# Shared sources, includes and definitions
target_include_directories(CherryUSB
        PUBLIC ${TARGET_C_INCLUDES}
        INTERFACE ${TARGET_C_INCLUDES}
        )

