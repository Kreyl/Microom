/*
 * pcm1865.h
 *
 *  Created on: 08 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef PCM1865_H_
#define PCM1865_H_

#include "kl_lib.h"
#include "board.h"
#include "main.h"

#define PCM_DMA_RX_MODE     STM32_DMA_CR_CHSEL(PCM_DMA_CHNL) | \
                            DMA_PRIORITY_MEDIUM | \
                            STM32_DMA_CR_MSIZE_HWORD | \
                            STM32_DMA_CR_PSIZE_HWORD | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                            STM32_DMA_CR_TCIE    |    /* Enable Transmission Complete IRQ */ \
                            STM32_DMA_CR_CIRC         /* Circular buffer enable */

// 8 samples per ms => 16 samples per two ms;
#define USB_SAMPLES_PER_MS  8   // for 8kHz sampling freq
#define PCM_SAMPLES_PER_MS  16  // for 16kHz sampling freq
#define SENDING_PERIOD      2   // Usb receives one pkt every two ms (usb driver works this way)
#define SAMPLE_SZ           2   // 16bit == 2bytes
#define PCM_USB_BUF_CNT     (USB_SAMPLES_PER_MS * SENDING_PERIOD)
#define PCM_CH_CNT          2


enum PcmAdcChnls_t {pacADC1L = 0x01, pacADC1R = 0x02, pacADC2L = 0x03, pacADC2R = 0x04};

class PCM1865_t {
private:
    Spi_t ISpi;
    PinOutput_t CS{PCM_CS_GPIO, PCM_CS_PIN, omPushPull};
    int16_t IChannels[PCM_CH_CNT], IndxCh = 0;
    int16_t BufToSend[PCM_USB_BUF_CNT], IndxToSend=0;
    void WriteReg(uint8_t Addr, uint8_t Value);
    uint8_t ReadReg(uint8_t Addr);
    // Commands
    void ResetRegs() { WriteReg(0x00, 0xFF); }
    void SelectPage(uint8_t Page) { WriteReg(0x00, Page); }
public:
    void Init();
    void PrintState();
    void PrintClkRegs();
    void EnterRunMode() { WriteReg(0x70, 0x70); }
    void EnterPowerdownMode() { WriteReg(0x70, 0x74); }
    void SetGain(PcmAdcChnls_t Chnl, int8_t Gain_dB);
    // Inner use
    void IRQDmaRxHandler();
};

extern PCM1865_t Pcm;

#endif /* PCM1865_H_ */
