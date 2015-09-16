/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_AUDIO_H_
#define USB_USB_AUDIO_H_

#include "kl_buf.h"

#define USB_AUDIO_BUF_SZ    108

union Sample_t {
    int16_t Word;
    uint8_t b[2];
} __attribute__((packed));

class UsbAudio_t {
private:
    uint8_t IBuf[USB_AUDIO_BUF_SZ];
public:
    output_queue_t oqueue;
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return false; }
    void Put(uint16_t);
    // Inner use
    void IOnDataTransmitted(USBDriver *usbp, usbep_t ep);
    void IOnConfigured();
};

extern UsbAudio_t UsbAu;

#endif /* USB_USB_AUDIO_H_ */
