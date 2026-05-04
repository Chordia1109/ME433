#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#define CFG_TUSB_RHPORT0_MODE      OPT_MODE_DEVICE
#define CFG_TUSB_OS                OPT_OS_PICO

#define CFG_TUD_ENABLED            1
#define CFG_TUD_ENDPOINT0_SIZE     64

#define CFG_TUD_HID                1
#define CFG_TUD_HID_EP_BUFSIZE     16

#ifdef __cplusplus
}
#endif

#endif