/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_AUDIO_H_
#define USB_USB_AUDIO_H_

#include "kl_buf.h"

#define USB_AUDIO_BUF_CNT   16

union Sample_t {
    int16_t Word;
    uint8_t b[2];
} __attribute__((packed));

class UsbAudio_t {
private:
    uint16_t w[2];
    CircBufNumber_t<int16_t, USB_AUDIO_BUF_CNT> IBuf;
    Sample_t ISample;
public:
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return false; }
    void Put(uint16_t);
    // Inner use
    void IOnDataTransmitted(USBDriver *usbp, usbep_t ep);
};

extern UsbAudio_t UsbAu;

#endif /* USB_USB_AUDIO_H_ */
