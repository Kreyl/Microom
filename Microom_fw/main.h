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

#define APP_NAME        "Microom"
#define APP_VERSION     __DATE__ " " __TIME__

class App_t {
private:
    thread_t *PThread;
public:
    // Eternal methods
    void InitThread() { PThread = chThdGetSelfX(); }
    void SignalEvt(eventmask_t Evt) {
        chSysLock();
        chEvtSignalI(PThread, Evt);
        chSysUnlock();
    }
    void SignalEvtI(eventmask_t Evt) {
        chEvtSignalI(PThread, Evt);
        }
    void OnUartCmd(Uart_t *PUart);
    // Inner use
    void ITask();
};

extern App_t App;

#endif /* MAIN_H_ */
