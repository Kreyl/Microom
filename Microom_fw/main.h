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
#include "board.h"
#include "apds9960.h"

#define APP_NAME            "Gesture Sensor"
#define APP_VERSION         __DATE__ " " __TIME__

#define SAMPLING_INTERVAL_MS	99

class App_t {
private:
    thread_t *PThread;
    void ProcessValues(Gesture_t Gesture);
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
