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

#define SNS_VALUE_THRESHOLD		1530
#define SAMPLING_INTERVAL_MS	999

// Sns state
struct SnsState_t {
	int8_t St0, St1;
	bool operator == (const SnsState_t &ASt) { return ((St0 == ASt.St0) and (St1 == ASt.St1)); }
	bool operator != (const SnsState_t &ASt) { return ((St0 != ASt.St0) or  (St1 != ASt.St1)); }
	SnsState_t& operator = (const SnsState_t &ASt) {
		St0 = ASt.St0;
		St1 = ASt.St1;
		return *this;
	}
	void Reset() { St0 = -1; St1 = -1; }
};

// Gesture
const SnsState_t Gesture[] = {
		{1, 0},
		{1, 1},
		{0, 1},
//		{1, 1},
//		{1, 0},
};
#define GESTURE_LEN	(countof(Gesture))

#define BUF_CNT		GESTURE_LEN

class App_t {
private:
    thread_t *PThread;
    void ProcessValues(uint32_t Sns0, uint32_t Sns1);
    SnsState_t IBuf[BUF_CNT];
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
