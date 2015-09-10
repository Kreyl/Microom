/*
 * pcm1865.cpp
 *
 *  Created on: 08 сент. 2015 г.
 *      Author: Kreyl
 */

#include "pcm1865.h"
#include "pcm1865_regs.h"
#include "uart.h"

#define PcmDelay()  { for(volatile uint32_t i=0; i<27; i++) __NOP(); }

void PCM1865_t::Init() {
    CS.Init();
    CS.SetHi();
    // SPI pins
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_SCK,  omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MISO, omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MOSI, omPushPull, pudNone, PCM_SPI_AF);
    // ==== SPI ==== MSB first, master, ClkLowIdle, FirstEdge, Baudrate=...
    ISpi.Setup(PCM_SPI, boMSB, cpolIdleLow, cphaSecondEdge, sbFdiv64);
    ISpi.Enable();
    SelectPage(0);

    // SCKI
    PinSetupAlterFunc(GPIOC, 6, omPushPull, pudNone, AF5);  // I2S2 MCK
    rccEnableSPI2(FALSE);
    SPI2->CR1 = 0;
    SPI2->CR2 = 0;
    SPI2->I2SCFGR = 0;  // Disable I2S
    // I2S, Master Receive, CkPol=Low, DatLen=16bit, ChLen=16bit, Frame = Philips I2S
    SPI2->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG;
    SPI2->I2SPR = SPI_I2SPR_MCKOE | SPI_I2SPR_I2SDIV | (uint16_t)187;    // 8000
    SPI2->I2SCFGR |= SPI_I2SCFGR_I2SE;
    // ==== I2S ====
    //SPI2->I2SPR = SPI_I2SPR_MCKOE |
}

void PCM1865_t::WriteReg(uint8_t Addr, uint8_t Value) {
    PcmDelay();
    CS.SetLo();
    PcmDelay();
    Addr = (Addr << 1) | 0x00;  // Write operation
    ISpi.ReadWriteByte(Addr);
    ISpi.ReadWriteByte(Value);
    PcmDelay();
    CS.SetHi();
    PcmDelay();
}

uint8_t PCM1865_t::ReadReg(uint8_t Addr) {
    PcmDelay();
    CS.SetLo();
    PcmDelay();
    Addr = (Addr << 1) | 0x01;  // Read operation
    ISpi.ReadWriteByte(Addr);
    uint8_t r = ISpi.ReadWriteByte(0);
    PcmDelay();
    CS.SetHi();
    PcmDelay();
    return r;
}

#if 1 // ============================== Commands ===============================
void PCM1865_t::SetGain(PcmAdcChnls_t Chnl, int8_t Gain_dB) {
    if(Gain_dB < -12 or Gain_dB > 40) return;

    uint8_t r = ReadReg(1);
    Uart.Printf("\rBefore: %X", r);

//    Gain_dB *= 2;
//    WriteReg((uint8_t)Chnl, (uint8_t)Gain_dB);
    WriteReg(1, 5);
    r = ReadReg(1);
    Uart.Printf("\rAfter: %X", r);
}
#endif
