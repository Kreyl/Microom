/*
 * pcm1865.h
 *
 *  Created on: 08 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef PCM1865_H_
#define PCM1865_H_

#include "kl_lib.h"

#define PCM_CS_GPIO     GPIOA
#define PCM_CS_PIN      4

// SPI
#define PCM_SPI_GPIO    GPIOA
#define PCM_SCK         5
#define PCM_MISO        6
#define PCM_MOSI        7
#define PCM_SPI         SPI1
#define PCM_SPI_AF      AF5

enum PcmAdcChnls_t {pacADC1L = 0x01, pacADC1R = 0x02, pacADC2L = 0x03, pacADC2R = 0x04};

class PCM1865_t {
private:
    Spi_t ISpi;
    PinOutput_t CS{PCM_CS_GPIO, PCM_CS_PIN, omPushPull};
    void WriteReg(uint8_t Addr, uint8_t Value);
    uint8_t ReadReg(uint8_t Addr);
    // Commands
    void ResetRegs() { WriteReg(0x00, 0xFF); }
    void SelectPage(uint8_t Page) { WriteReg(0x00, Page); }
public:
    void Init();
    void PrintState();
    void EnterRunMode() { WriteReg(0x70, 0); }
    void SetGain(PcmAdcChnls_t Chnl, int8_t Gain_dB);
};


#endif /* PCM1865_H_ */
