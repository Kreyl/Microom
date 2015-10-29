/*
 * usb_cdc.cpp
 *
 *  Created on: 03 сент. 2015 г.
 *      Author: Kreyl
 */

#include "descriptors_keybrd.h"
#include "usb_keybrd.h"
#include "ch.h"
#include "hal.h"
#include "usb.h"
#include "usb_lld.h"
#include "main.h"

UsbKBrd_t UsbKBrd;

bool OnSetupPkt(USBDriver *usbp);
static void usb_event(USBDriver *usbp, usbevent_t event);

#if 1 // ========================== Endpoints ==================================
// ==== EP1 ====
void OnDataTransmitted(USBDriver *usbp, usbep_t ep) {  }

static USBInEndpointState ep1instate;

// EP1 initialization structure (both IN and OUT).
static const USBEndpointConfig ep1config = {
    USB_EP_MODE_TYPE_INTR,
    NULL,                   // setup_cb
    OnDataTransmitted,      // in_cb
    NULL,                   // out_cb
    64,                     // in_maxsize
    64,                     // out_maxsize
    &ep1instate,            // in_state
    NULL,                   // out_state
    2,                      // in_multiplier: Determines the space allocated for the TXFIFO as multiples of the packet size
    NULL                    // setup_buf: Pointer to a buffer for setup packets. Set this field to NULL for non-control endpoints
};
#endif

#if 1 // ======================== Events & Config ==============================
// ==== USB driver configuration ====
const USBConfig UsbCfg = {
    usb_event,          // This callback is invoked when an USB driver event is registered
    GetDescriptor,      // Device GET_DESCRIPTOR request callback
    OnSetupPkt,         // This hook allows to be notified of standard requests or to handle non standard requests
    NULL                // Start Of Frame callback
};

static void usb_event(USBDriver *usbp, usbevent_t event) {
	Uart.PrintfI("\rUSB evt=%X", event);
    switch (event) {
        case USB_EVENT_RESET:
            return;
        case USB_EVENT_ADDRESS:
            return;
        case USB_EVENT_CONFIGURED:
            chSysLockFromISR();
            /* Enable the endpoints specified in the configuration.
            Note, this callback is invoked from an ISR so I-Class functions must be used.*/
            usbInitEndpointI(usbp, EP_DATA_IN_ID, &ep1config);
            App.SignalEvtI(EVTMSK_USB_READY);
            chSysUnlockFromISR();
            return;
        case USB_EVENT_SUSPEND:
            chSysLockFromISR();
            App.SignalEvtI(EVTMSK_USB_READY);
            chSysUnlockFromISR();
            return;
        case USB_EVENT_WAKEUP:
            return;
        case USB_EVENT_STALLED:
            return;
    } // switch
}

#endif

/* ==== Setup Packet handler ====
 * true         Message handled internally.
 * false        Message not handled. */
#define REQDIR_DEVICETOHOST        (1 << 7)
#define REQTYPE_CLASS              (1 << 5)
#define REQREC_INTERFACE           (1 << 0)

bool OnSetupPkt(USBDriver *usbp) {
    SetupPkt_t *Setup = (SetupPkt_t*)usbp->setup;
    Uart.PrintfI("\rSetup: %A", usbp->setup, 8, ' ');
    if(Setup->wIndex == 0) {	// 0 is Keyboard interface number
    	switch(Setup->bRequest) {
    		case HID_REQ_GetReport:
    			if(Setup->bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
    				uint16_t ReportSize = 0;
    				uint8_t  ReportID   = (Setup->wValue & 0xFF);
    				uint8_t  ReportType = (Setup->wValue >> 8) - 1;
    				uint8_t  ReportData[HIDInterfaceInfo->Config.PrevReportINBufferSize];

    				memset(ReportData, 0, sizeof(ReportData));

    				CALLBACK_HID_Device_CreateHIDReport(HIDInterfaceInfo, &ReportID, ReportType, ReportData, &ReportSize);

    				if (HIDInterfaceInfo->Config.PrevReportINBuffer != NULL)
    				{
    					memcpy(HIDInterfaceInfo->Config.PrevReportINBuffer, ReportData,
    					       HIDInterfaceInfo->Config.PrevReportINBufferSize);
    				}

    				Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);

    				Endpoint_ClearSETUP();

    				if (ReportID)
    				  Endpoint_Write_8(ReportID);

    				Endpoint_Write_Control_Stream_LE(ReportData, ReportSize);
    				Endpoint_ClearOUT();
    			}

    			break;

    	}
    }


    if(Setup->bmRequestType == 0x01) { // Host2Device, standard, recipient=interface
        if(Setup->bRequest == USB_REQ_SET_INTERFACE) {
            if(Setup->wIndex == 1) {
                usbSetupTransfer(usbp, NULL, 0, NULL);
                // wValue contains alternate setting
                chSysLockFromISR();
                if(Setup->wValue == 1) {    // Transmission on
//                    App.SignalEvtI(EVTMSK_START_LISTEN);
                }
                else {
//                    App.SignalEvtI(EVTMSK_STOP_LISTEN);
                }
                chSysUnlockFromISR();
                return true;
            }
        }
    }
    return false;
}

//void UsbAudio_t::SendBufI(uint8_t *Ptr, uint32_t Len) {
//    if(!IsListening) return;
//    // If the USB driver is not in the appropriate state then transactions must not be started
//    if(usbGetDriverStateI(&USBDrv) != USB_ACTIVE) return;
//    // If there is not an ongoing transaction and Len != 0
//    if(!usbGetTransmitStatusI(&USBDrv, EP_DATA_IN_ID)) {
//        if(Len > 0) {
//            usbPrepareTransmit(&USBD1, EP_DATA_IN_ID, Ptr, Len);
//            chSysLockFromISR();
//            usbStartTransmitI(&USBD1, EP_DATA_IN_ID);
//            chSysUnlockFromISR();
//        }
//    }
//}
