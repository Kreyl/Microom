/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
 *      Author: g.kruglov
 */

#include "main.h"
#include "math.h"
#include "usb_audio.h"
#include "chprintf.h"
#include "pcm1865.h"
#include "leds.h"
#include "filter.h"

App_t App;
PCM1865_t Pcm;
PinOutputPWM_t<LED_TOP_VALUE, invNotInverted, omPushPull> Led[4] = {
        {LED5_GPIO, LED5_PIN, LED5_TIM, LED5_TCHNL},
        {LED2_GPIO, LED2_PIN, LED2_TIM, LED2_TCHNL},
        {LED7_GPIO, LED7_PIN, LED7_TIM, LED7_TCHNL},
        {LED8_GPIO, LED8_PIN, LED8_TIM, LED8_TCHNL},
};

PinOutput_t LedAux = {LEDAUX_GPIO, LEDAUX_PIN, omPushPull};

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
    LedAux.Init();
    for(uint8_t i=0; i<4; i++) {
        Led[i].Init();
//        for(uint8_t j=0; j<250; j++) {
//            Led[i].Set(j);
//            chThdSleepMilliseconds(4);
//        }
//        Led[i].Set(0);
    }

    // Debug: init CS2 as output
    PinSetupOut(GPIOC, 13, omPushPull, pudNone);

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
            LedAux.SetHi();
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("\rUsbSuspend");
            LedAux.SetLo();
        }

        if(EvtMsk & EVTMSK_START_LISTEN) {
        }
        if(EvtMsk & EVTMSK_STOP_LISTEN) {

        }
#endif
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }

    } // while true
}

#if 1 // ======================= Command processing ============================
void App_t::OnCmd(Shell_t *PShell) {
	Cmd_t *PCmd = &PShell->Cmd;
    __attribute__((unused)) int32_t dw32 = 0;  // May be unused in some configurations
    Uart.Printf("\r%S\r", PCmd->Name);
    // Handle command
    if(PCmd->NameIs("Ping")) PShell->Ack(OK);

    else if(PCmd->NameIs("State"))  Pcm.PrintState();
    else if(PCmd->NameIs("PwrDwn")) Pcm.EnterPowerdownMode();
    else if(PCmd->NameIs("Run"))    Pcm.EnterRunMode();

    else if(PCmd->NameIs("Grp1")) Pcm.SelectMicGrp(mg1);
    else if(PCmd->NameIs("Grp2")) Pcm.SelectMicGrp(mg2);

    else if(PCmd->NameIs("SetGain")) {
        if(PCmd->GetNextNumber(&dw32) != OK) { PShell->Ack(CMD_ERROR); return; }
        Pcm.SetGain((int8_t)dw32);
    }

    else if(PCmd->NameIs("GetGain")) {
        int8_t g;
        g = Pcm.GetGain(1);
        PShell->Printf("\rGain1 = %d", g);
        g = Pcm.GetGain(2);
        PShell->Printf("\rGain2 = %d", g);
        g = Pcm.GetGain(3);
        PShell->Printf("\rGain3 = %d", g);
        g = Pcm.GetGain(4);
        PShell->Printf("\rGain4 = %d", g);
    }

    else if(PCmd->NameIs("SelCh")) {
        if(PCmd->GetNextNumber(&dw32) != OK) { PShell->Ack(CMD_ERROR); return; }
        ChToSend = (uint8_t)dw32;
    }

    else PShell->Ack(CMD_UNKNOWN);
}

#endif

#if 1 // ======================= Sample processing =============================
// Mic indx to Led indx
const int Mic2Led[4] = {1, 4, 7, 6};

uint8_t App_t::Mic2LedLevel(int32_t MicLevel) {
    if(MicLevel <= 0) return 0;
    int32_t r = MicLevel / 8192;
    if(r > LED_TOP_VALUE) r = LED_TOP_VALUE;
    return (uint8_t)r;
}


int32_t PrevIndx=-1;
void App_t::ProcessValues(int16_t *Values) {
    // Copy values to local array (and do not afraid that Values[] will be overwritten by DMA)
    IChnl[0] = Values[0];   // Mic1
    IChnl[1] = Values[1];   // Mic4
    IChnl[2] = Values[2];   // Mic7
    IChnl[3] = Values[3];   // Mic6
    // Calculate and show level
    int32_t Max = 0;
    __attribute__((unused)) int32_t Indx = 0;
    for(int i=0; i<CHNL_CNT; i++) {
        int32_t Lvl = LvlMtr[i].AddXAndCalculate(IChnl[i]);
        uint8_t Brt = Mic2LedLevel(Lvl);
        Led[i].Set(Brt);
        if(Max < Lvl) {
            Max = Lvl;
            Indx = i;
        }
    }

#if AGC_ENABLED
    if(AgcCounter++ >= AGC_PERIOD_TICKS) {
        AgcCounter = 0;
        Max /= LVLMTR_CNT;
        // Adjust gain
        if(Max > AGC_HI_VOLUME and Gain > AGC_MIN_GAIN) {
            Gain--;
            Pcm.SetGain(Gain);
        }
        else if(Max < AGC_LO_VOLUME and Gain < AGC_MAX_GAIN) {
            Gain++;
            Pcm.SetGain(Gain);
        }
//        Uart.PrintfI("\rMax = %d; Gain = %d", Max, Gain);
    }
#endif

    // Copy selected data to buffer-to-send and send to USB when filled up
    if(ChToSend <= 3) Indx = ChToSend;
    if(Indx != PrevIndx) {
        PrevIndx = Indx;
        Uart.PrintfI("%u\r", Indx);
    }

    if(Buf2Send.Append(IChnl[Indx]) == addrSwitch) {
        uint8_t *p = (uint8_t*)Buf2Send.GetBufToRead();
        UsbAu.SendBufI(p, USB_PKT_SZ);
    }
}
#endif
