/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "usb_keybrd.h"
#include "main.h"
#include "math.h"
#include "leds.h"
#include "filter.h"
#include "kl_adc.h"

App_t App;

int main(void) {
    // ==== Setup clock frequency ====
    uint8_t ClkResult = 1;
    Clk.SetupFlashLatency(64);  // Setup Flash Latency for clock in MHz
    Clk.EnablePrefetch();
    // 12 MHz/6 = 2; 2*192 = 384; 384/6 = 64 (preAHB divider); 384/8 = 48 (USB clock)
    Clk.SetupPLLDividers(6, 192, pllSysDiv6, 8);
    // 64/1 = 64 MHz core clock. APB1 & APB2 clock derive on AHB clock; APB1max = 42MHz, APB2max = 84MHz
    // Keep APB freq at 32 MHz to left peripheral settings untouched
    Clk.SetupBusDividers(ahbDiv1, apbDiv2, apbDiv2);
    if((ClkResult = Clk.SwitchToPLL()) == 0) Clk.HSIDisable();
    Clk.UpdateFreqValues();

    // Init OS
    halInit();
    chSysInit();

    // ==== Init hardware ====
    Uart.Init(115200, UART_GPIO, UART_TX_PIN, UART_GPIO, UART_RX_PIN);
    Uart.Printf("\r%S %S", APP_NAME, APP_VERSION);
    Clk.PrintFreqs();
    if(ClkResult != 0) Uart.Printf("\rXTAL failure");

    App.InitThread();
  	Led[0].Init();
  	Led[1].Init();

    Adc.Init();
    PinSetupAnalog(GPIOC, 0);
    PinSetupAnalog(GPIOC, 1);

    // Debug: init CS2 as output
//    PinSetupOut(GPIOC, 13, omPushPull, pudNone);

    // ==== USB ====
    UsbKBrd.Init();
    UsbKBrd.Connect();

    // Main cycle
    App.ITask();
}

__attribute__ ((__noreturn__))
void App_t::ITask() {
    TmrSampling.InitAndStart(PThread, MS2ST(SAMPLING_INTERVAL_MS), EVTMSK_SAMPLING, tvtPeriodic);

    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
#if 1 // ==== USB ====
        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("\rUsbReady");
            Led[0].SetHi();
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("\rUsbSuspend");
            Led[0].SetLo();
        }
#endif
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }

        // ==== ADC ====
        if(EvtMsk & EVTMSK_SAMPLING) {
        	Adc.StartMeasurement();
        }
        if(EvtMsk & EVTMSK_ADC_DONE) {
         	uint32_t Sns0 = Adc.GetResult(SNS_CHNL0);
         	uint32_t Sns1 = Adc.GetResult(SNS_CHNL1);
         	ProcessValues(Sns0, Sns1);
        }
    } // while true
}

#if 1 // ======================= Signal processing =============================
void App_t::ProcessValues(uint32_t Sns0, uint32_t Sns1) {
// 	Uart.Printf("\r%u; %u", Sns0, Sns1);
 	SnsState_t Current;
 	Current.St0 = (Sns0 > SNS_VALUE_THRESHOLD)? 1 : 0;
 	Current.St1 = (Sns1 > SNS_VALUE_THRESHOLD)? 1 : 0;
 	if(Current.St0 == 0 and Current.St1 == 0) return; // ignore "nothing" info
 	// Check if changed
 	if(Current != IBuf[0]) {
 		// Shift buffer
 		for(uint32_t i=(BUF_CNT-1); i>0; i--) IBuf[i] = IBuf[i-1];
		IBuf[0] = Current;	// Put new value into the buf
//		Uart.Printf("\rBuf:"); for(uint32_t i=0; i<GESTURE_LEN; i++) Uart.Printf(" %d %d;", IBuf[i].St0, IBuf[i].St1);
		// ==== Compare gesture sequence ====
		bool IsLike = true;
		for(uint32_t i=0; i<GESTURE_LEN; i++) {
			if(IBuf[i] != Gesture[GESTURE_LEN - 1 - i]) {
				IsLike = false;
				break;
			}
		} // for
		if(IsLike) {
			Uart.Printf("\rGest");
			UsbKBrd.PressAndRelease(HID_KEYBOARD_SC_A);
		}
 	} // if changed
}
#endif

#if 1 // ======================= Command processing ============================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("\r%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(OK);

    else PShell->Ack(CMD_UNKNOWN);
}

#endif
