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
#include "board.h"
#include "leds.h"
#include "filter.h"

App_t App;
PCM1865_t Pcm;

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
    // Leds
    for(uint8_t i=0; i<9; i++) {
        Led[i].Init();
//        Led[i].SetHi();
//        chThdSleepMilliseconds(270);
//        Led[i].SetLo();
    }

    // ==== USB ====
    UsbAu.Init();
    UsbAu.Connect();

    Pcm.Init();

    // Main cycle
    App.ITask();
}

__attribute__ ((__noreturn__))
void App_t::ITask() {
    // Filters init
    for(int i=0; i<CHNL_CNT; i++) LvlMtr[i].Reset();

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

        if(EvtMsk & EVTMSK_START_LISTEN) {
        }
        if(EvtMsk & EVTMSK_STOP_LISTEN) {

        }
#endif
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnUartCmd(&Uart);
            Uart.SignalCmdProcessed();
        }

    } // while true
}

#if 1 // ======================= Command processing ============================
void App_t::OnUartCmd(Uart_t *PUart) {
    UartCmd_t *PCmd = &PUart->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("\r%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) PUart->Ack(OK);

    else if(PCmd->NameIs("State"))  Pcm.PrintState();
    else if(PCmd->NameIs("PwrDwn")) Pcm.EnterPowerdownMode();
    else if(PCmd->NameIs("Run"))    Pcm.EnterRunMode();

    else if(PCmd->NameIs("Grp1")) Pcm.SelectMicGrp(mg1);
    else if(PCmd->NameIs("Grp2")) Pcm.SelectMicGrp(mg2);

    else if(PCmd->NameIs("SetGain")) {
        if(PCmd->GetNextNumber(&dw32) != OK) { PUart->Ack(CMD_ERROR); return; }
        Pcm.SetGain((int8_t)dw32);
    }

    else if(PCmd->NameIs("GetGain")) {
        int8_t g;
        g = Pcm.GetGain(1);
        PUart->Printf("\rGain1 = %d", g);
        g = Pcm.GetGain(2);
        PUart->Printf("\rGain2 = %d", g);
        g = Pcm.GetGain(3);
        PUart->Printf("\rGain3 = %d", g);
        g = Pcm.GetGain(4);
        PUart->Printf("\rGain4 = %d", g);
    }

    else PUart->Ack(CMD_UNKNOWN);
}


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

#if 1 // =========================== Filtering =================================
// Mic indx to Led indx
const int Mic2Led[4] = {1, 4, 7, 6};

void App_t::ProcessValues(int16_t *Values) {
    // Copy values to local array (and do not afraid that Values[] will be overwritten by DMA)
    IChnl[0] = Values[0];   // Mic1
    IChnl[1] = Values[1];   // Mic4
    IChnl[2] = Values[2];   // Mic7
    IChnl[3] = Values[3];   // Mic6
    // Calculate level
    int32_t Max = 0, Indx = 0;
    for(int i=0; i<CHNL_CNT; i++) {
        int32_t Lvl = LvlMtr[i].AddXAndCalculate(IChnl[i]);
        if(Max < Lvl) {
            Max = Lvl;
            Indx = i;
        }
    }
    // Show loudest
    Led[MaxLedIndx].SetLo();    // Switch off previous MaxLed
    MaxLedIndx = Mic2Led[Indx];
    Led[MaxLedIndx].SetHi();    // Switch on new MaxLed
    // Copy selected data to buffer-to-send and send to USB when filled up
    if(Buf2Send.Append(IChnl[Indx]) == addrSwitch) {
        uint8_t *p = (uint8_t*)Buf2Send.GetBufToRead();
        UsbAu.SendBufI(p, USB_PKT_SZ);
    }
}
#endif
