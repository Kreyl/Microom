/*
 * main.cpp
 *
 *  Created on: 20 февр. 2014 г.
 *      Author: g.kruglov
 */

#include "main.h"
#include "math.h"
#include "usb_cdc.h"
#include "chprintf.h"

App_t App;

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
    chRegSetThreadName("blinker");
    while (true) {
        chThdSleepMilliseconds(702);
        if(SDU2.config->usbp->state == USB_ACTIVE) {
            chprintf((BaseSequentialStream*)&SDU2, "aga\r");
        }
    }
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

    // Init OS
    halInit();
    chSysInit();

    PinSetupOut(GPIOB, 10, omPushPull, pudNone);
    PinSetupOut(GPIOB, 8, omPushPull, pudNone);

    // ==== Init hardware ====
    Uart.Init(115200, GPIOB, 6);
    Uart.Printf("\r%S %S", APP_NAME, APP_VERSION);
    Clk.PrintFreqs();

    if(ClkResult != 0) Uart.Printf("\rXTAL failure");

    App.InitThread();

    // ==== USB ====
    // GPIO
    PinSetupAlterFunc(GPIOA, 11, omOpenDrain, pudNone, AF10);
    PinSetupAlterFunc(GPIOA, 12, omOpenDrain, pudNone, AF10);
    // Objects
    sduObjectInit(&SDU2);
    sduStart(&SDU2, &SerUsbCfg);
    // Connect
    usbDisconnectBus(SerUsbCfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(SerUsbCfg.usbp, &UsbCfg);
    usbConnectBus(SerUsbCfg.usbp);

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    // Main cycle
    App.ITask();
}

__attribute__ ((__noreturn__))
void App_t::ITask() {
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
//            // ==== Filter ====
//            y0 = PCurrentFilter->AddXAndCalculate(y0);

        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("\rUsbReady");
            PinSet(GPIOB, 10);
        }

//        if(EvtMsk & EVTMSK_USB_DATA_OUT) {
//            LedBlink(54);
//            while(UsbUart.ProcessOutData() == pdrNewCmd) OnUartCmd();
//        }
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
