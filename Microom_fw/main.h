/*
 * main.h
 *
 *  Created on: 15 сент. 2014 г.
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
#include "board.h"

#define APP_NAME            "Microom"
#define APP_VERSION         __DATE__ " " __TIME__

#define CHNL_CNT            4 // 4 mics simultaneously

/* 16000 Hz sampling freq, 2 bytes per sample. One frame per millisecond
 * means 16 samples per frame, but every second IN packet is lost. Therefore,
 * 32 samples per frame required. */

// 16 samples per ms => 32 samples per two ms;
#define USB_SAMPLES_PER_MS  16  // for 16kHz sampling freq

#define SENDING_PERIOD      2   // Usb receives one pkt every two ms (usb driver works this way)
#define PCM_USB_BUF_CNT     (USB_SAMPLES_PER_MS * SENDING_PERIOD)
#define USB_PKT_SZ          (PCM_USB_BUF_CNT * SAMPLE_SZ)

#define LVLMTR_CNT          4096 // Size of Moving Average for Level metering

// Automatic Gain Control
#define AGC_ENABLED         FALSE
#define AGC_MAX_GAIN        PCM_MAX_GAIN_DB
#define AGC_MIN_GAIN        PCM_MIN_GAIN_DB
#define AGC_HI_VOLUME       1080
#define AGC_LO_VOLUME       1008
#define AGC_PERIOD_TICKS    1000

class App_t {
private:
    thread_t *PThread;
    int16_t IChnl[CHNL_CNT];
    LvlMtr_t<LVLMTR_CNT> LvlMtr[CHNL_CNT];
    uint8_t Mic2LedLevel(int32_t MicLevel);
#if AGC_ENABLED
    uint32_t AgcCounter = 0;
    int8_t Gain = 0;
#endif
    uint8_t ChToSend=0;
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
    void OnCmd(Shell_t *PShell);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* MAIN_H_ */
