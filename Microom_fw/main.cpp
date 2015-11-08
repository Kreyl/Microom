/*
 * main.cpp
 *
 *  Created on: 20 ����. 2014 �.
 *      Author: g.kruglov
 */

#include "usb_keybrd.h"
#include "main.h"
#include "math.h"
#include "leds.h"
#include "filter.h"

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
    Uart.Printf("\r%S %S\r", APP_NAME, APP_VERSION);
    Clk.PrintFreqs();
    if(ClkResult != 0) Uart.Printf("XTAL failure\r");

    App.InitThread();
  	Led[0].Init();
  	Led[1].Init();

  	Apds.Init();
  	if(Apds.EnableGestureSns() != OK) Uart.Printf("Gesture Sns Failure\r");

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
    while(true) {
        uint32_t EvtMsk = chEvtWaitAny(ALL_EVENTS);
#if 1 // ==== USB ====
        if(EvtMsk & EVTMSK_USB_READY) {
            Uart.Printf("UsbReady\r");
            Led[0].SetHi();
        }
        if(EvtMsk & EVTMSK_USB_SUSPEND) {
            Uart.Printf("UsbSuspend\r");
            Led[0].SetLo();
        }
#endif
        if(EvtMsk & EVTMSK_UART_NEW_CMD) {
            OnCmd((Shell_t*)&Uart);
            Uart.SignalCmdProcessed();
        }

        // ==== Sensor ====
        if(EvtMsk & EVTMSK_GESTURE) {
            ProcessValues(Apds.GetGesture());
//            Uart.Printf("\rp");
        }
    } // while true
}

#if 1 // ======================= Signal processing =============================
void App_t::ProcessValues(Gesture_t Gesture) {
    switch(Gesture) {
        case gstUp:
            Uart.Printf("Gst Up\r");
            UsbKBrd.PressAndRelease(HID_KEYBOARD_SC_A);
            break;
        case gstDown:
            Uart.Printf("Gst Down\r");
            break;
        case gstLeft:
            Uart.Printf("Gst Left\r");
            break;
        case gstRight:
            Uart.Printf("Gst Right\r");
            break;
        case gstNone:
            Uart.Printf("Gst None\r");
            break;
    }
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
