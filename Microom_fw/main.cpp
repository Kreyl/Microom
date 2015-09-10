/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "math.h"
#include "usb_audio.h"
#include "chprintf.h"
#include "pcm1865.h"

App_t App;
PCM1865_t Pcm;

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
    chRegSetThreadName("blinker");
    while(true) {
        chThdSleepMilliseconds(450);

    } // while true
}


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
    // I2S Clk
    Clk.SetupI2SClk(192, 2);

    // Init OS
    halInit();
    chSysInit();

//    Clk.MCO1Enable(mco1PLL, mcoDiv4);
    Clk.MCO2Enable(mco2PLLI2S, mcoDiv5);
    PinSetupOut(GPIOB, 10, omPushPull, pudNone);
    PinSetupOut(GPIOB, 8, omPushPull, pudNone);

    // ==== Init hardware ====
    Uart.Init(115200, GPIOB, 6);
    Uart.Printf("\r%S %S", APP_NAME, APP_VERSION);
    Clk.PrintFreqs();

    if(ClkResult != 0) Uart.Printf("\rXTAL failure");

    App.InitThread();

    Pcm.Init();
    Pcm.SetGain(pacADC1L, 2);

    // ==== USB ====
//    UsbAu.Init();
//    UsbAu.Connect();

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    // Main cycle
    App.ITask();
}

__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
#if 1 // ==== USB ====
        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("\rUsbReady");
            PinSet(GPIOB, 10);
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("\rUsbSuspend");
            PinClear(GPIOB, 10);
        }

        if(EvtMsk & EVTMSK_START_LISTEN) {
            UsbAu.Put(0);           // First dummy sample
        }
        if(EvtMsk & EVTMSK_STOP_LISTEN) {

        }
#endif


    } // while true
}

#if 1 // ======================= Command processing ============================
//void App_t::OnUartCmd(Uart_t *PUart) {
//    UsbCmd_t *PCmd = UsbUart.PCmd;
//    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
//    Uart.Printf("\r%S", PCmd->Name);
    // Handle command
//    if(PCmd->NameIs("#Ping")) UsbUart.Ack(OK);

#if 0 // ==== Common ====
    else if(PCmd->NameIs("#SetSmplFreq")) {
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if(PCmd->TryConvertTokenToNumber(&dw32) != OK) { UsbUart.Ack(CMD_ERROR); return; }
        SamplingTmr.SetUpdateFrequency(dw32);
        UsbUart.Ack(OK);
    }

    else if(PCmd->NameIs("#SetResolution")) {
        if(PCmd->GetNextToken() != OK) { UsbUart.Ack(CMD_ERROR); return; }
        if((PCmd->TryConvertTokenToNumber(&dw32) != OK) or (dw32 < 1 or dw32 > 16)) { UsbUart.Ack(CMD_ERROR); return; }
        ResolutionMask = 0xFFFF << (16 - dw32);
        UsbUart.Ack(OK);
    }

    // Output analog filter
    else if(PCmd->NameIs("#OutFilterOn"))  { OutputFilterOn();  UsbUart.Ack(OK); }
    else if(PCmd->NameIs("#OutFilterOff")) { OutputFilterOff(); UsbUart.Ack(OK); }

    // Stat/Stop
    else if(PCmd->NameIs("#Start")) { PCurrentFilter->Start(); UsbUart.Ack(OK); }
    else if(PCmd->NameIs("#Stop"))  { PCurrentFilter->Stop();  UsbUart.Ack(OK); }
#endif

//    else UsbUart.Ack(CMD_UNKNOWN);  // reply only #-started stuff
//}
#endif

#if 1 // ============================ LEDs =====================================

#endif
