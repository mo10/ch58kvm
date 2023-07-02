#include "main.h"

#include "CH58x_common.h"
#include "hid.h"

static void debug_uart_init() {
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
}

static uint8_t pressed = 0;

int main() {
    SetSysClock(CLK_SOURCE_PLL_60MHz);

#if defined(DEBUG)
    debug_uart_init(); /* Enable debug uart */
#endif

    /* Status LED init */
    GPIOB_ResetBits(STATUS_LED_PIN);
    GPIOB_ModeCfg(STATUS_LED_PIN, GPIO_ModeOut_PP_5mA);
//    TMR0_TimerInit(FREQ_SYS / 1); // 设置定时时间 100ms
//    TMR0_ITCfg(ENABLE, TMR0_3_IT_CYC_END); // 开启中断
//    PFIC_EnableIRQ(TMR0_IRQn);
    /* Key */
    GPIOB_ResetBits(KEY_PIN);
    GPIOB_ModeCfg(KEY_PIN, GPIO_ModeIN_PU);

    // USB1 init
//    printf("\r\nUSB1 Init\r\n");
//    hid_init(0);
//    PFIC_EnableIRQ(USB_IRQn);
    // USB2 init
    printf("USB2 Init\r\n");
    hid_init(1);
    PFIC_EnableIRQ(USB2_IRQn);

    while (1) {
        if (!GPIOB_ReadPortPin(KEY_PIN) && !pressed) {
            // Test keyboard
//            hid_kbd_report report = {0};
//            report.modifier.left_ctrl = 1;
//            report.modifier.left_alt = 1;
//            report.key[0] = HID_KBD_USAGE_DELFWD;
//            hid_send_keyboard_report(1, &report);
            // Test Mouse
            hid_mouse_report  report = {0};
//            report.buttons.middle =1;
            report.pointer.x=-5;
            report.pointer.y=5;
            report.h_wheel = -1;
            report.v_wheel = -1;
            hid_send_mouse_report(1, &report);

            pressed = 1;
            continue;
        }
        if (GPIOB_ReadPortPin(KEY_PIN) && pressed) {
            // Release keyboard
//            hid_kbd_report report = {0};
//            hid_send_keyboard_report(1, &report);
            // Release mouse
            hid_mouse_report report = {0};
            hid_send_mouse_report(1, &report);
            pressed = 0;
            continue;
        }

        // Keyboard caps lock
        if (hid_get_keyboard_led_status(1).caps_lock) {
            GPIOB_ResetBits(STATUS_LED_PIN);
        } else {
            GPIOB_SetBits(STATUS_LED_PIN);
        }
    }
}

__attribute__((used))
__attribute__((interrupt("WCH-Interrupt-fast")))
__attribute__((section(".highcode")))
void TMR0_IRQHandler(void) {
    if (TMR0_GetITFlag(TMR0_3_IT_CYC_END)) {
        TMR0_ClearITFlag(TMR0_3_IT_CYC_END);
//        GPIOB_InverseBits(STATUS_LED_PIN);
    }
}
