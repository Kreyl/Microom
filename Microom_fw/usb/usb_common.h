/*
 * usb_common.h
 *
 *  Created on: 29 ���. 2015 �.
 *      Author: Kreyl
 */

#ifndef USB_USB_COMMON_H_
#define USB_USB_COMMON_H_

#include "board.h"
#include "kl_lib.h"

struct SetupPkt_t {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
};

extern const USBConfig UsbCfg;

class UsbCommon_t {
public:
    void Init() {
        PinSetupAlterFunc(USB_GPIO, USB_DM_PIN, omOpenDrain, pudNone, AF10);
        PinSetupAlterFunc(USB_GPIO, USB_DP_PIN, omOpenDrain, pudNone, AF10);
    }
    void Connect() {
        usbDisconnectBus(&USBDrv);
        chThdSleepMilliseconds(1500);
        usbStart(&USBDrv, &UsbCfg);
        usbConnectBus(&USBDrv);
    }
    void Disconnect() { usbDisconnectBus(&USBDrv); }
};

#endif /* USB_USB_COMMON_H_ */
