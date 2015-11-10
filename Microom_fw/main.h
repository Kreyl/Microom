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

#define APP_NAME            "Gesture Sensor"
#define APP_VERSION         __DATE__ " " __TIME__

#define SNS_LOW_THRESHOLD		1530
#define SNS_HIGH_THRESHOLD      1710
#define SAMPLING_INTERVAL_MS	11

enum HiLo_t { hlLow=0, hlHigh=1 };

class App_t {
private:
    HiLo_t Prev0 = hlLow, Prev1 = hlLow;
    HiLo_t Normalize(uint32_t X, HiLo_t PrevX);
    RiseFall_t DetectEdge(HiLo_t X, HiLo_t PrevX);
    uint32_t CntU=0, CntD=0;
    void ResetCounters() { CntU=0; CntD=0; }


    thread_t *PThread;
    void ProcessValues(uint32_t Sns0, uint32_t Sns1);
public:
    TmrVirtual_t TmrSampling;
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
