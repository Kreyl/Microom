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
    PinSetupAnalog(GPIOC, 4);

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
    TmrReset.Init(PThread, MS2ST(RESET_INTERVAL), EVTMSK_RESET, tvtOneShot);
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

        if(EvtMsk & EVTMSK_RESET) {
            ResetCounters();
        }
    } // while true
}

#if 1 // ======================= Signal processing =============================
void App_t::ProcessValues(uint32_t Sns0, uint32_t Sns1) {
// 	Uart.Printf("%u; %u", Sns0, Sns1);
    // Normalize
    HiLo_t Norm0, Norm1;
    Norm0 = Normalize(Sns0, Prev0);
    Norm1 = Normalize(Sns1, Prev1);
//    Uart.Printf("    %u; %u\r", Norm0, Norm1);
    // Detect edge
    RiseFall_t Edge0 = DetectEdge(Norm0, Prev0);
    RiseFall_t Edge1 = DetectEdge(Norm1, Prev1);
    // Detect gesture
    if((Edge0 == rfRising and Norm1 == hlLow) or (Edge0 == rfFalling and Norm1 == hlHigh)) CntD++;
    else if((Edge1== rfRising and Norm0 == hlLow) or (Edge1 == rfFalling and Norm0 == hlHigh)) CntU++;
    // Save current values as previous
    Prev0 = Norm0;
    Prev1 = Norm1;
    // Start timer
    if(CntD == 1 or CntU == 1) TmrReset.StartIfNotRunning();
    // Send event if gesture recognized
    if(CntD >= 2) {
        ResetCounters();
        Uart.Printf("Down\r");
        UsbKBrd.PressAndRelease(HID_KEYBOARD_SC_A);
    }
    else if(CntU >= 2) {
        ResetCounters();
        Uart.Printf("Up\r");
//        UsbKBrd.PressAndRelease(HID_KEYBOARD_SC_A);
    }
}

HiLo_t App_t::Normalize(uint32_t X, HiLo_t PrevX) {
    if(PrevX == hlLow) return (X > SNS_HIGH_THRESHOLD)? hlHigh : hlLow;
    else return (X < SNS_LOW_THRESHOLD)? hlLow : hlHigh;
}

RiseFall_t App_t::DetectEdge(HiLo_t X, HiLo_t PrevX) {
    if(X > PrevX) return rfRising;
    else if(X < PrevX) return rfFalling;
    else return rfNone;
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
