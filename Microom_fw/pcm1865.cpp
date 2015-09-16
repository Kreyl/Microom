/*
 * pcm1865.cpp
 *
 *  Created on: 08 сент. 2015 г.
 *      Author: Kreyl
 */

#include "pcm1865.h"
#include "pcm1865_regs.h"
#include "uart.h"
#include "clocking.h"
#include "usb_audio.h"

/*
 * Fs = 8kHz
 * SCKI = Fs*256 (fixed) == 2048kHz (stm output)
 *
 */

// ST I2S formats
#define I2SSTD_I2S      0x00
#define I2SSTD_MSB      SPI_I2SCFGR_I2SSTD_0
#define I2SSTD_LSB      SPI_I2SCFGR_I2SSTD_1
#define I2SSTD_PCM      (SPI_I2SCFGR_I2SSTD_0 | SPI_I2SCFGR_I2SSTD_1)

// Wrapper for DMA TX IRQ
extern "C" {
void PcmRxIrq(void *p, uint32_t flags) { Pcm.IRQDmaRxHandler(); }
}

// ==== TX DMA IRQ ====
void PCM1865_t::IRQDmaRxHandler() {
    dmaStreamDisable(PCM_DMA_STREAM);
    // Switch buffers
//    if(PWrite == &IRxBuf[0][0]) {
//        PWrite = &IRxBuf[1][0];
//        PRead = &IRxBuf[0][0];
//    }
//    else {
//        PWrite = &IRxBuf[0][0];
//        PRead = &IRxBuf[1][0];
//    }
    dmaStreamSetMemory0   (PCM_DMA_STREAM, PWrite);
    dmaStreamSetTransactionSize(PCM_DMA_STREAM, PCM_BUF_CNT);
    dmaStreamSetMode      (PCM_DMA_STREAM, PCM_DMA_RX_MODE);
    dmaStreamEnable       (PCM_DMA_STREAM);
//    Uart.PrintfI("\r%d", w / PCM_BUF_CNT);

    oqWriteTimeout(&UsbAu.oqueue, PRead, (PCM_BUF_CNT * 2),
}



void PCM1865_t::Init() {
    // Variables
    PWrite = &IRxBuf[0][0];
    PRead  = &IRxBuf[1][0];
    // ==== Control SPI ====
    CS.Init();
    CS.SetHi();
    // SPI pins
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_SCK,  omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MISO, omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MOSI, omPushPull, pudNone, PCM_SPI_AF);
    // ==== Control SPI ==== MSB first, master, ClkLowIdle, FirstEdge, Baudrate=32/4=8MHz
    ISpi.Setup(PCM_SPI, boMSB, cpolIdleLow, cphaSecondEdge, sbFdiv4);
    ISpi.Enable();

    EnterPowerdownMode();

    // ==== I2S ====
    // GPIO
    PinSetupAlterFunc(GPIOC, 6,  omPushPull, pudNone, AF5);  // I2S2 MCK
    PinSetupAlterFunc(GPIOB, 12, omPushPull, pudNone, AF5);  // I2S2_WS LRClk1
    PinSetupAlterFunc(GPIOB, 13, omPushPull, pudNone, AF5);  // I2S2_CK BitClk1
    PinSetupAlterFunc(GPIOB, 15, omPushPull, pudNone, AF5);  // I2S2_SD DataOut1

    PCM_I2S_RccEnable();
    // I2S Clk
    RCC->CFGR &= ~RCC_CFGR_I2SSRC;  // Disable external clock
    Clk.SetupI2SClk(128, 5);        // I2S PLL Divider
    // I2S
    PCM_I2S->CR1 = 0;
    PCM_I2S->CR2 = SPI_CR2_RXDMAEN;
    PCM_I2S->I2SCFGR = 0;  // Disable I2S
    // Mode=I2S, Master Receive, PcmSync not needed, I2SSTD=MSB, CkPol=Low, DatLen=16bit, ChLen=16bit
    PCM_I2S->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG | I2SSTD_I2S;
    PCM_I2S->I2SPR = SPI_I2SPR_MCKOE | (1 << 8) | (uint16_t)12;    // 8000
    PCM_I2S->I2SCFGR |= SPI_I2SCFGR_I2SE;
    // ==== DMA ====
    dmaStreamAllocate     (PCM_DMA_STREAM, IRQ_PRIO_LOW, PcmRxIrq, NULL);
    dmaStreamSetPeripheral(PCM_DMA_STREAM, &PCM_I2S->DR);
    dmaStreamSetMemory0   (PCM_DMA_STREAM, PWrite);
    dmaStreamSetTransactionSize(PCM_DMA_STREAM, PCM_BUF_CNT);
    dmaStreamSetMode      (PCM_DMA_STREAM, PCM_DMA_RX_MODE);
    dmaStreamEnable       (PCM_DMA_STREAM);

    chThdSleepMilliseconds(9);  // Let clocks to stabilize

    // ==== Setup regs ====
    ResetRegs();
    SelectPage(0);

    WriteReg(0x05, 0b00000110); // No Smooth, no Link, no ClippDet, def attenuation, no AGC
    // Main ADC Channel selection (everywhere signal is not inverted)
    WriteReg(0x06, 0x01);   // ADC1L = VinL1(SE)
    WriteReg(0x07, 0x01);   // ADC1R = VinR1(SE)
    WriteReg(0x08, 0x02);   // ADC2L = VinL2(SE)
    WriteReg(0x09, 0x02);   // ADC2R = VinR2(SE)
    WriteReg(0x0A, 0x00);   // Secondary ADC not connected
    // Formats
    WriteReg(0x0B, 0b11001101); // RX_WLEN=16bit (not used), LRCK duty=50%, TX_WLEN=16bit, FMT=Left Justified (==MSB justified in ST terms)
//    WriteReg(0x0C, 0x01);   // TDM mode: 4 ch
    WriteReg(0x0C, 0x00);   // TDM mode: 2 ch

    EnterRunMode();

}

void PCM1865_t::WriteReg(uint8_t Addr, uint8_t Value) {
//    Uart.Printf("\rW: %02X %02X", Addr, Value);
    CS.SetLo();
    Addr = (Addr << 1) | 0x00;  // Write operation
    ISpi.ReadWriteByte(Addr);
    ISpi.ReadWriteByte(Value);
    CS.SetHi();
}

uint8_t PCM1865_t::ReadReg(uint8_t Addr) {
    CS.SetLo();
    Addr = (Addr << 1) | 0x01;  // Read operation
    ISpi.ReadWriteByte(Addr);
    uint8_t r = ISpi.ReadWriteByte(0);
    CS.SetHi();
    return r;
}

#if 1 // ============================== Commands ===============================
void PCM1865_t::PrintState() {
    Uart.Printf("\rPCM State");
    uint8_t b = ReadReg(0x72);
    switch(b & 0x0F) {
        case 0x00: Uart.Printf("\r  PwrDown"); break;
        case 0x01: Uart.Printf("\r  Wait Clk Stable"); break;
        case 0x02: Uart.Printf("\r  Release reset"); break;
        case 0x03: Uart.Printf("\r  Stand-by"); break;
        case 0x04: Uart.Printf("\r  Fade IN"); break;
        case 0x05: Uart.Printf("\r  Fade OUT"); break;
        case 0x09: Uart.Printf("\r  Sleep"); break;
        case 0x0F: Uart.Printf("\r  Run"); break;
        default: Uart.Printf("\r  ! Reserved"); break;
    }
    // Ratio
    b = ReadReg(0x73);
    switch(b & 0x07) {
        case 0x00: Uart.Printf("\r  Fs: Out of range (Low) or LRCK Halt"); break;
        case 0x01: Uart.Printf("\r  Fs: 8kHz"); break;
        case 0x02: Uart.Printf("\r  Fs: 16kHz"); break;
        case 0x03: Uart.Printf("\r  Fs: 32-48kHz"); break;
        case 0x04: Uart.Printf("\r  Fs: 88.2-96kHz"); break;
        case 0x05: Uart.Printf("\r  Fs: 176.4-192kHz"); break;
        case 0x06: Uart.Printf("\r  Fs: Out of range (High)"); break;
        case 0x07: Uart.Printf("\r  Invalid Fs"); break;
        default: break;
    }
    b = ReadReg(0x74);
    switch(b & 0x70) {
        case 0x00: Uart.Printf("\r  BCK Ratio: Out of range (L) or BCK Halt"); break;
        case 0x10: Uart.Printf("\r  BCK Ratio: 32"); break;
        case 0x20: Uart.Printf("\r  BCK Ratio: 48"); break;
        case 0x30: Uart.Printf("\r  BCK Ratio: 64"); break;
        case 0x40: Uart.Printf("\r  BCK Ratio: 256"); break;
        case 0x60: Uart.Printf("\r  BCK Ratio: Out of range (H)"); break;
        case 0x70: Uart.Printf("\r  Invalid BCK ratio or LRCK Halt"); break;
        default: break;
    }
    switch(b & 0x07) {
        case 0x00: Uart.Printf("\r  SCK Ratio: Out of range (L) or SCK Halt"); break;
        case 0x01: Uart.Printf("\r  SCK Ratio: 128"); break;
        case 0x02: Uart.Printf("\r  SCK Ratio: 256"); break;
        case 0x03: Uart.Printf("\r  SCK Ratio: 384"); break;
        case 0x04: Uart.Printf("\r  SCK Ratio: 512"); break;
        case 0x05: Uart.Printf("\r  SCK Ratio: 768"); break;
        case 0x06: Uart.Printf("\r  SCK Ratio: Out of range (H)"); break;
        case 0x07: Uart.Printf("\r  Invalid SCK ratio or LRCK Halt"); break;
        default: break;
    }
    // Clock
    b = ReadReg(0x75);
    if(b & 0x40) Uart.Printf("\r  LRCK Halt");
    else if(b & 0x04) Uart.Printf("\r  LRCK Error");
    else Uart.Printf("\r  LRCK Ok");
    if(b & 0x20) Uart.Printf("\r  BCK  Halt");
    else if(b & 0x02) Uart.Printf("\r  BCK  Error");
    else Uart.Printf("\r  BCK  Ok");
    if(b & 0x10) Uart.Printf("\r  SCK  Halt");
    else if(b & 0x01) Uart.Printf("\r  SCK  Error");
    else Uart.Printf("\r  SCK  Ok");
    // Power
    b = ReadReg(0x78);
    if(b & 0x04) Uart.Printf("\r  DVDD Ok");
    else Uart.Printf("\r  DVDD Bad/Missing");
    if(b & 0x02) Uart.Printf("\r  AVDD Ok");
    else Uart.Printf("\r  AVDD Bad/Missing");
    if(b & 0x01) Uart.Printf("\r  LDO  Ok");
    else Uart.Printf("\r  LDO  Failure");
}


#define Addr    0x02
void PCM1865_t::SetGain(PcmAdcChnls_t Chnl, int8_t Gain_dB) {
    if(Gain_dB < -12 or Gain_dB > 40) return;

    uint8_t b = ReadReg(Addr);
    Uart.Printf("\rBefore: %X", b);

//    Gain_dB *= 2;
//    WriteReg((uint8_t)Chnl, (uint8_t)Gain_dB);
    WriteReg(Addr, 5);
    b = ReadReg(Addr);
    Uart.Printf("\rAfter: %X", b);
}
#endif
