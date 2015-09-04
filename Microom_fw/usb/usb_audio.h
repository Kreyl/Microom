/*
 * usb_audio.h
 *
 *  Created on: 05 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef USB_USB_AUDIO_H_
#define USB_USB_AUDIO_H_

class UsbAudio_t {
private:

public:
    void Init();
    void Connect();
    void Disconnect();
    bool IsActive() { return false; }
    // Inner use
};

extern UsbAudio_t UsbAu;

#endif /* USB_USB_AUDIO_H_ */
