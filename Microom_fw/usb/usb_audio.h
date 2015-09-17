/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_AUDIO_H_
#define USB_USB_AUDIO_H_

#include "kl_buf.h"

union Sample_t {
    int16_t Word;
    uint8_t b[2];
} __attribute__((packed));

class UsbAudio_t {
private:
public:
    bool IsListening = false;
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return false; }
    // Data
    void SendBufI(uint8_t *Ptr, uint32_t Len);

    void Put(uint16_t);
    // Inner use
};

extern UsbAudio_t UsbAu;

#endif /* USB_USB_AUDIO_H_ */
