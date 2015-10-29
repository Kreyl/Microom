/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_KEYBRD_H_
#define USB_USB_KEYBRD_H_

#include "usb_common.h"

class UsbKBrd_t : public UsbCommon_t {
private:
public:
    bool IsActive() { return false; }
    // Data
    void SendKeyCode(uint8_t KeyCode);

    // Inner use
};

extern UsbKBrd_t UsbKBrd;

#endif /* USB_USB_KEYBRD_H_ */
