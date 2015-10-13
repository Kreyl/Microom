/*
 * main.h
 *
 *  Created on: 15 ����. 2014 �.
 *      Author: g.kruglov
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "ch.h"
#include "kl_lib.h"
#include "uart.h"
#include "evt_mask.h"
#include "filter.h"
#include "kl_buf.h"

#define APP_NAME            "Microom"
#define APP_VERSION         __DATE__ " " __TIME__

#define CHNL_CNT            4 // 4 mics simultaneously

// 8 samples per ms => 16 samples per two ms;
#define USB_SAMPLES_PER_MS  8   // for 8kHz sampling freq
#define SENDING_PERIOD      2   // Usb receives one pkt every two ms (usb driver works this way)
#define PCM_USB_BUF_CNT     (USB_SAMPLES_PER_MS * SENDING_PERIOD)
#define USB_PKT_SZ          (PCM_USB_BUF_CNT * SAMPLE_SZ)

class App_t {
private:
    thread_t *PThread;
    int16_t IChnl[CHNL_CNT];
    int MaxLedIndx = 1;
    LvlMtr_t LvlMtr[CHNL_CNT];

    DoubleBuf_t<int16_t, PCM_USB_BUF_CNT> Buf2Send;
public:
    void ProcessValues(int16_t *Values);
    // Eternal methods
    void InitThread() { PThread = chThdGetSelfX(); }
    void SignalEvt(eventmask_t Evt) {
        chSysLock();
        chEvtSignalI(PThread, Evt);
        chSysUnlock();
    }
    void SignalEvtI(eventmask_t Evt) { chEvtSignalI(PThread, Evt); }
    void OnUartCmd(Uart_t *PUart);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* MAIN_H_ */
