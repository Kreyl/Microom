/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_KEYBRD_H_
#define USB_USB_KEYBRD_H_

#include "usb_common.h"
#include "HIDClassCommon.h"

class UsbKBrd_t : public UsbCommon_t {
private:
public:
    // Data
    void PressKey(uint8_t KeyCode);
    void DepressKey(uint8_t KeyCode);
    // Inner use
    uint8_t Protocol = 1;	// When initialized, all devices default to report protocol
    uint8_t IdleRate = 0;
    bool HasChanged = true;
    USB_KeyboardReport_Data_t Report;
    void ISendInReportI();	// Send report through IN endpoint
};

extern UsbKBrd_t UsbKBrd;

#endif /* USB_USB_KEYBRD_H_ */
