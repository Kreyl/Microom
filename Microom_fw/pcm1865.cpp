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


/* 8000 Hz sampling freq, 2 bytes per sample. One frame per millisecond
 * means 8 samples per frame, but every second IN packet is lost. Therefore,
 * 16 samples per frame required.
 */

// ST I2S formats
#define I2SSTD_I2S      0x00
#define I2SSTD_MSB      SPI_I2SCFGR_I2SSTD_0
#define I2SSTD_LSB      SPI_I2SCFGR_I2SSTD_1
#define I2SSTD_PCM      (SPI_I2SCFGR_I2SSTD_0 | SPI_I2SCFGR_I2SSTD_1)

PinOutputPWM_t<7, invNotInverted, omPushPull> TmrClk {GPIOC, 6, TIM3, 1};

// Wrapper for DMA TX IRQ
extern "C" {
void PcmRxIrq(void *p, uint32_t flags) { Pcm.IRQDmaRxHandler(); }
}

// ==== TX DMA IRQ ====
void PCM1865_t::IRQDmaRxHandler() {
    dmaStreamDisable(PCM_DMA_STREAM);
    // Switch buffers
    IndxCh += 2; // Two channels at a time
    if(IndxCh >= PCM_CH_CNT) IndxCh = 0;

    // Switch mic pair
//    switch(IndxCh) {
//        case 2:
//            WriteReg(0x06, 0x02);   // ADC1L = VinL2(SE)
//            WriteReg(0x07, 0x02);   // ADC1R = VinR2(SE)
//            break;
//        case 4:
//            WriteReg(0x06, 0x04);   // ADC1L = VinL3(SE)
//            WriteReg(0x07, 0x04);   // ADC1R = VinR3(SE)
//            break;
//        case 6:
//            WriteReg(0x06, 0x08);   // ADC1L = VinL4(SE)
//            WriteReg(0x07, 0x08);   // ADC1R = VinR4(SE)
//            break;
//        default: // 0
//            WriteReg(0x06, 0x01);   // ADC1L = VinL1(SE)
//            WriteReg(0x07, 0x01);   // ADC1R = VinR1(SE)
//            break;
//    } // switch

    // == Send data to USB ==
    if(IndxCh == 0) { // New cycle just have started
        // Copy data to buffer-to-send
        BufToSend[IndxToSend++] = IChannels[2];
        if(IndxToSend >= PCM_USB_BUF_CNT) {
            IndxToSend = 0;
            UsbAu.SendBufI((uint8_t*)BufToSend, (PCM_USB_BUF_CNT * SAMPLE_SZ));
        }
    } // if new cycle

    // Start DMA
    dmaStreamSetMemory0   (PCM_DMA_STREAM, &IChannels[IndxCh]);
    dmaStreamSetTransactionSize(PCM_DMA_STREAM, 2);
    dmaStreamSetMode      (PCM_DMA_STREAM, PCM_DMA_RX_MODE);
    dmaStreamEnable       (PCM_DMA_STREAM);
}

void PCM1865_t::Init() {
    // Variables
    // ==== Control SPI ====
    CS.Init();
    CS.SetHi();
    // SPI pins
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_SCK,  omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MISO, omPushPull, pudNone, PCM_SPI_AF);
    PinSetupAlterFunc(PCM_SPI_GPIO, PCM_MOSI, omPushPull, pudNone, PCM_SPI_AF);
    // ==== Control SPI ==== MSB first, master, ClkLowIdle, FirstEdge, Baudrate=32/4=8MHz
    ISpi.Setup(PCM_SPI, boMSB, cpolIdleLow, cphaSecondEdge, sbFdiv8);
    ISpi.Enable();

    EnterPowerdownMode();

    // ==== I2S ====
    // GPIO
//    PinSetupAlterFunc(GPIOC, 6,  omPushPull, pudNone, AF5);  // I2S2 MCK
//    PinSetupAlterFunc(GPIOB, 12, omPushPull, pudNone, AF5);  // I2S2_WS LRClk1
//    PinSetupAlterFunc(GPIOB, 13, omPushPull, pudNone, AF5);  // I2S2_CK BitClk1
//    PinSetupAlterFunc(GPIOB, 15, omPushPull, pudNone, AF5);  // I2S2_SD DataOut1

    PinSetupIn(GPIOB, 15, pudPullDown);

//    PCM_I2S_RccEnable();
    // I2S Clk
//    RCC->CFGR &= ~RCC_CFGR_I2SSRC;  // Disable external clock
//    Clk.SetupI2SClk(256, 5);        // I2S PLL Divider
    // I2S
//    PCM_I2S->CR1 = 0;
//    PCM_I2S->CR2 = SPI_CR2_RXDMAEN;
//    PCM_I2S->I2SCFGR = 0;  // Disable I2S
    // Mode=I2S, Master Receive, PcmSync not needed, I2SSTD=MSB, CkPol=Low, DatLen=16bit, ChLen=16bit
//    PCM_I2S->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG | I2SSTD_I2S;
    // Mode=I2S, Master Receive, PcmSync=short, I2SSTD=PCM, CkPol=Low, DatLen=16bit, ChLen=16bit
//    PCM_I2S->I2SCFGR = SPI_I2SCFGR_I2SMOD | SPI_I2SCFGR_I2SCFG | I2SSTD_PCM | (0b10 << 1) | 1;
//    PCM_I2S->I2SPR = SPI_I2SPR_MCKOE | (1 << 8) | (uint16_t)12;
//    PCM_I2S->I2SCFGR |= SPI_I2SCFGR_I2SE;

    TmrClk.Init();
    TmrClk.Set(3);

    chThdSleepMilliseconds(9);  // Let clocks to stabilize

    // ==== Setup regs ====
    ResetRegs();
    SelectPage(0);

    // Clocks
    WriteReg(0x28, 0x00);   // PLL disabled

    //WriteReg(0x20, 0b01011110); // SCK, Master, All connected to PLL, Clk detector disabled
//    WriteReg(0x20, 0b01010010); // SCK, Master, All connected to SCK, Clk detector disabled
//    WriteReg(0x20, 0b00010001); // Master, Clk detector enabled
//    WriteReg(0x20, 0b00110000); // Master, use PLL, All use SCK, Clk detector disabled
    WriteReg(0x20, 0b00111110); // Master, use PLL, All use PLL, Clk detector disabled
//    WriteReg(0x20, 0b00110001); // Master, use PLL for BCK/LRCK, All use SCK, Clk detector enabled
//    WriteReg(0x20, 0b00000001); // Slave, Clk detector enabled

    // PLL coeffs
    WriteReg(0x29, 0);      // PLL P = 1
    WriteReg(0x2A, 0);      // PLL R = 1
    WriteReg(0x2B, 8);      // PLL J-part }
    WriteReg(0x2D, 0x07);   // PLL D-MSB  } K = 8.192
    WriteReg(0x2C, 0x80);   // PLL D-LSB  }

    // DSP1, DSP2, ADC dividers
    WriteReg(0x21, 7);    // DSP1 Clock Divider Value
    WriteReg(0x22, 15);   // DSP2 Clock Divider Value
    WriteReg(0x23, 31);   // ADC Clock Divider Value

    // BCK & LRCK dividers
    WriteReg(0x25, 7);   // PLL SCK Clock Divider value
    WriteReg(0x26, 1);   // Master Clock (SCK) Divider value
    WriteReg(0x27, 255);  // Master SCK Clock Divider value

    WriteReg(0x28, 0x01);   // PLL enabled, src is SCK

    // == Main part ====
    WriteReg(0x05, 0b00000110); // No Smooth, no Link, no ClippDet, def attenuation, no AGC
    // Main ADC Channel selection (everywhere signal is not inverted)
    WriteReg(0x06, 0x01);   // ADC1L = VinL1(SE)
    WriteReg(0x07, 0x01);   // ADC1R = VinR1(SE)
    WriteReg(0x08, 0x02);   // ADC2L = VinL2(SE)
    WriteReg(0x09, 0x02);   // ADC2R = VinR2(SE)
//    WriteReg(0x08, 0x00);   // ADC2L = No Select
//    WriteReg(0x09, 0x00);   // ADC2R = No Select
    WriteReg(0x0A, 0x00);   // Secondary ADC not connected

    // == Formats ==
    // RX_WLEN=16bit (not used), LRCK duty=50%, TX_WLEN=16bit, FMT=Left Justified (==MSB justified in ST terms)
//    WriteReg(0x0B, 0b11001101);
    // RX_WLEN=16bit (not used), LRCK duty=50%, TX_WLEN=16bit, FMT=I2S
//    WriteReg(0x0B, 0b11001100);
    // RX_WLEN=16bit (not used), LRCK duty=50%, TX_WLEN=16bit, FMT=TDM
    WriteReg(0x0B, 0b11001111);
    WriteReg(0x0C, 0x01);   // TDM mode: 4 ch
//    WriteReg(0x0C, 0x00);   // TDM mode: 2 ch
//    WriteReg(0x0D, 0x00);   // TX TDM Offset: 0

    EnterRunMode();
    chThdSleepMilliseconds(450);
    PrintState();

    uint8_t b;
    b = ReadReg(0x20);
    Uart.Printf("\r 0x20: %X", b);
    b = ReadReg(0x21);
    Uart.Printf("\r 0x21: %u", b);
    b = ReadReg(0x22);
    Uart.Printf("\r 0x22: %u", b);
    b = ReadReg(0x23);
    Uart.Printf("\r 0x23: %u", b);
    b = ReadReg(0x25);
    Uart.Printf("\r 0x25: %u", b);
    b = ReadReg(0x26);
    Uart.Printf("\r 0x26: %u", b);
    b = ReadReg(0x27);
    Uart.Printf("\r 0x27: %u", b);
    b = ReadReg(0x28);
    Uart.Printf("\r 0x28: %X", b);
    b = ReadReg(0x29);
    Uart.Printf("\r 0x29: %u", b);
    b = ReadReg(0x2A);
    Uart.Printf("\r 0x2A: %u", b);
    b = ReadReg(0x2B);
    Uart.Printf("\r 0x2B: %u", b);
    b = ReadReg(0x2C);
    Uart.Printf("\r 0x2C: %u", b);
    b = ReadReg(0x2D);
    Uart.Printf("\r 0x2D: %u", b);

    // ==== DMA ====
    dmaStreamAllocate     (PCM_DMA_STREAM, IRQ_PRIO_LOW, PcmRxIrq, NULL);
    dmaStreamSetPeripheral(PCM_DMA_STREAM, &PCM_I2S->DR);
    dmaStreamSetMemory0   (PCM_DMA_STREAM, IChannels);
    dmaStreamSetTransactionSize(PCM_DMA_STREAM, 2);
    dmaStreamSetMode      (PCM_DMA_STREAM, PCM_DMA_RX_MODE);
    dmaStreamEnable       (PCM_DMA_STREAM);
}

void PCM1865_t::WriteReg(uint8_t Addr, uint8_t Value) {
//    Uart.Printf("\rW: %02X %02X", Addr, Value);
    CS.SetLo();
    Addr = (Addr << 1) | 0x00;  // Write operation
    ISpi.ReadWriteByte(Addr);
    ISpi.ReadWriteByte(Value);
    CS.SetHi();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
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
