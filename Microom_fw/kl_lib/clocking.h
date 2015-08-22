/*
 * clocking_f0.h
 *
 *  Created on: 20.01.2013
 *      Author: kreyl
 */

#ifndef CLOCKING_H_
#define CLOCKING_H_

#include "mcuconf.h"
#include "ch.h"

#if defined STM32L1XX_MD

#include "stm32l1xx.h"
#include "chconf.h"
/*
 * Right after reset, CPU works on internal MSI source.
 * To switch to external src (HSE) without dividing (i.e. SysClk == CrystalFreq),
 * call SwitchToHSE(), and then optionally HSIDisable().
 * To switch from HSE to HSI, call SwitchToHSI() then optionally HSEDisable().
 * To switch to PLL, disable it first with PLLDisable(), then setup dividers
 * with SetupPLLDividers(), then call SwitchToPLL(). Then disable HSI if needed.
 *
 * Do not forget to update Freq values after switching.
 *
 * AHB  freq max = 32 MHz;
 * APB1 freq max = 32 MHz;
 * APB2 freq max = 32 MHz;
 */

#define CRYSTAL_FREQ_HZ     8000000     // Freq of external crystal, change accordingly
#define HSI_FREQ_HZ         HSI_VALUE   // Freq of internal generator, not adjustable
#define LSI_FREQ_HZ         37000       // Freq of internal generator, not adjustable

enum ClkSrc_t {csHSI, csHSE, csPLL, csMSI};
enum PllMul_t {
    pllMul3 = 0b0000,
    pllMul4 = 0b0001,
    pllMul6 = 0b0010,
    pllMul8 = 0b0011,
    pllMul12= 0b0100,
    pllMul16= 0b0101,
    pllMul24= 0b0110,
    pllMul32= 0b0111,
    pllMul48= 0b1000,
};

enum PllDiv_t {pllDiv2=0b01, pllDiv3=0b10, pllDiv4=0b11};

enum AHBDiv_t {
    ahbDiv1=0b0000,
    ahbDiv2=0b1000,
    ahbDiv4=0b1001,
    ahbDiv8=0b1010,
    ahbDiv16=0b1011,
    ahbDiv64=0b1100,
    ahbDiv128=0b1101,
    ahbDiv256=0b1110,
    ahbDiv512=0b1111
};
enum APBDiv_t {apbDiv1=0b000, apbDiv2=0b100, apbDiv4=0b101, apbDiv8=0b110, apbDiv16=0b111};

class Clk_t {
private:
    uint8_t EnableHSE();
    uint8_t EnablePLL();
    uint8_t EnableMSI();
public:
    // Frequency values
    uint32_t AHBFreqHz;     // HCLK: AHB Bus, Core, Memory, DMA; 32 MHz max
    uint32_t APB1FreqHz;    // PCLK1: APB1 Bus clock; 32 MHz max
    uint32_t APB2FreqHz;    // PCLK2: APB2 Bus clock; 32 MHz max
    // SysClk switching
    uint8_t SwitchToHSI();
    uint8_t SwitchToHSE();
    uint8_t SwitchToPLL();
    uint8_t SwitchToMSI();
    void DisableHSE() { RCC->CR &= ~RCC_CR_HSEON; }
    uint8_t EnableHSI();
    void DisableHSI() { RCC->CR &= ~RCC_CR_HSION; }
    void DisablePLL() { RCC->CR &= ~RCC_CR_PLLON; }
    void DisableMSI() { RCC->CR &= ~RCC_CR_MSION; }
    void SetupBusDividers(AHBDiv_t AHBDiv, APBDiv_t APB1Div, APBDiv_t APB2Div);
    uint8_t SetupPLLMulDiv(PllMul_t PllMul, PllDiv_t PllDiv);
    void UpdateFreqValues();
    void UpdateSysTick() { SysTick->LOAD = AHBFreqHz / CH_FREQUENCY - 1; }
    void SetupFlashLatency(uint8_t AHBClk_MHz);
    // LSI
    void EnableLSI() {
        RCC->CSR |= RCC_CSR_LSION;
        while(!(RCC->CSR & RCC_CSR_LSIRDY));
    }
    void DisableLSI() { RCC->CSR &= RCC_CSR_LSION; }
    // LSE
    void StartLSE() {
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_DBP;
        RCC->CSR |= RCC_CSR_LSEON;
    }
    bool IsLseOn() { return (RCC->CSR & RCC_CSR_LSERDY); }
    void DisableLSE() {
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_DBP;
        RCC->CSR &= ~RCC_CSR_LSEON;
    }
};

extern Clk_t Clk;

// =============================== V Core ======================================
enum VCore_t {vcore1V2=0b11, vcore1V5=0b10, vcore1V8=0b01};
extern VCore_t VCore;
void SetupVCore(VCore_t AVCore);

#elif defined STM32F030
#include "stm32f0xx.h"

/*
 * Right after reset, CPU works on internal (HSI) source.
 * To switch to external src (HSE) without dividing (i.e. SysClk == CrystalFreq),
 * call SwitchToHSE(), and then optionally HSIDisable().
 * To switch from HSE to HSI, call SwitchToHSI() then optionally HSEDisable().
 * To switch to PLL, disable it first with PLLDisable(), then setup dividers
 * with SetupPLLDividers(), then call SwitchToPLL(). Then disable HSI if needed.
 *
 * Do not forget to update Freq values after switching.
 *
 * Keep in mind that Flash latency need to be increased at higher speeds.
 * Tune it with SetupFlashLatency.
 *
 * AHB  freq max = 48 MHz;
 * APB  freq max = 48 MHz;
 */

#define CRYSTAL_FREQ_HZ     8000000     // Freq of external crystal, change accordingly

enum ClkSrc_t {csHSI=0b00, csHSE=0b01, csPLL=0b10};
enum PllMul_t {
    pllMul2=0,
    pllMul3=1,
    pllMul4=2,
    pllMul5=3,
    pllMul6=4,
    pllMul7=5,
    pllMul8=6,
    pllMul9=7,
    pllMul10=8,
    pllMul11=9,
    pllMul12=10,
    pllMul13=11,
    pllMul14=12,
    pllMul15=13,
    pllMul16=14
};

enum PllSrc_t {plsHSIdiv2=0b00, plsHSI=0b01, plsHSE=0b10};

enum AHBDiv_t {
    ahbDiv1=0b0000,
    ahbDiv2=0b1000,
    ahbDiv4=0b1001,
    ahbDiv8=0b1010,
    ahbDiv16=0b1011,
    ahbDiv64=0b1100,
    ahbDiv128=0b1101,
    ahbDiv256=0b1110,
    ahbDiv512=0b1111
};
enum APBDiv_t {apbDiv1=0b000, apbDiv2=0b100, apbDiv4=0b101, apbDiv8=0b110, apbDiv16=0b111};

class Clk_t {
private:
    uint8_t EnableHSE();
    uint8_t EnableHSI();
    uint8_t EnablePLL();
    uint8_t EnableHSI48();
public:
    // Frequency values
    uint32_t AHBFreqHz;     // HCLK: AHB Bus, Core, Memory, DMA; 48 MHz max
    uint32_t APBFreqHz;     // PCLK: APB Bus clock; 48 MHz max
    // SysClk switching
    uint8_t SwitchTo(ClkSrc_t AClkSrc);
    void EnableCSS()  { RCC->CR |=  RCC_CR_CSSON; }
    void DisableCSS() { RCC->CR &= ~RCC_CR_CSSON; }
    void DisableHSE() { RCC->CR &= ~RCC_CR_HSEON; }
    void DisableHSI() { RCC->CR &= ~RCC_CR_HSION; }
    void DisablePLL() { RCC->CR &= ~RCC_CR_PLLON; }
    void SetupBusDividers(AHBDiv_t AHBDiv, APBDiv_t APBDiv);
    uint8_t SetupPLLDividers(uint8_t HsePreDiv, PllMul_t PllMul);
    void UpdateFreqValues();
    void SetupFlashLatency(uint32_t FrequencyHz);
};

extern Clk_t Clk;

/*
 * Early initialization code.
 * This initialization must be performed just after stack setup and before
 * any other initialization.
 */
extern "C" {
void __early_init(void);
}
#elif defined STM32F40_41xxx
#include "stm32f4xx.h"

#define CRYSTAL_FREQ_HZ     12000000    // Freq of external crystal, change accordingly
#define HSI_FREQ_HZ         HSI_VALUE   // Freq of internal generator, not adjustable
#define LSI_FREQ_HZ         32000       // Freq of low power internal generator, may vary depending on voltage, not adjustable

enum ClkSrc_t {csHSI, csHSE, csPLL};
enum AHBDiv_t {
    ahbDiv1=0b0000,
    ahbDiv2=0b1000,
    ahbDiv4=0b1001,
    ahbDiv8=0b1010,
    ahbDiv16=0b1011,
    ahbDiv64=0b1100,
    ahbDiv128=0b1101,
    ahbDiv256=0b1110,
    ahbDiv512=0b1111
};
enum APBDiv_t {apbDiv1=0b000, apbDiv2=0b100, apbDiv4=0b101, apbDiv8=0b110, apbDiv16=0b111};
enum PllSysDiv_P_t {pllSysDiv2=0b00, pllSysDiv4=0b01, pllSysDiv6=0b10, pllSysDiv8=0b11};
enum Mco1Src_t {mco1HSI=0x00000000, mco1LSE=0x00200000, mco1HSE=0x00400000, mco1PLL=0x00600000};
enum McoDiv_t {mcoDiv1=0x00000000, mcoDiv2=0x04000000, mcoDiv3=0x05000000, mcoDiv4=0x06000000, mcoDiv5=0x07000000};

class Clk_t {
private:
    uint8_t HSEEnable();
    uint8_t HSIEnable();
    uint8_t PLLEnable();
public:
    // Frequency values
    uint32_t AHBFreqHz;     // HCLK: AHB Buses, Core, Memory, DMA; 120 MHz max
    uint32_t APB1FreqHz;    // PCLK1: APB1 Bus clock; 30 MHz max
    uint32_t APB2FreqHz;    // PCLK2: APB2 Bus clock; 60 MHz max
    uint32_t UsbSdioFreqHz; // Clock is intended to be 48 MHz
    uint8_t TimerAPB1ClkMulti = 1;
    uint8_t TimerAPB2ClkMulti = 1;
    // Clk switching
    uint8_t SwitchToHSI();
    uint8_t SwitchToHSE();
    uint8_t SwitchToPLL();
    void HSEDisable() { RCC->CR &= ~RCC_CR_HSEON; }
    void HSIDisable() { RCC->CR &= ~RCC_CR_HSION; }
    void PLLDisable() { RCC->CR &= ~RCC_CR_PLLON; }
    void LsiEnable();
    // Dividers
    void SetupBusDividers(AHBDiv_t AHBDiv, APBDiv_t APB1Div, APBDiv_t APB2Div);
    uint8_t SetupPLLDividers(uint8_t InputDiv_M, uint16_t Multi_N, PllSysDiv_P_t SysDiv_P, uint8_t UsbDiv_Q);
    void UpdateFreqValues();
    uint8_t SetupFlashLatency(uint8_t AHBClk_MHz, uint16_t Voltage_mV=3300);
    // Systick init: the timer is required for OS
    void InitSysTick() {
        __disable_irq();
        SysTick->LOAD = AHBFreqHz / CH_FREQUENCY - 1;
        SysTick->VAL = 0;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
        __enable_irq();
    }
    // Special frequencies
    void SetFreq12Mhz() {
        if(AHBFreqHz < 12000000) SetupFlashLatency(12); // Rise flash latency now if current freq > required
        SetupBusDividers(ahbDiv4, apbDiv1, apbDiv1);
        UpdateFreqValues();
        SetupFlashLatency(AHBFreqHz/1000000);
    }
    void SetFreq48Mhz() {
        if(AHBFreqHz < 48000000) SetupFlashLatency(48);     // Rise flash latency now if current freq > required
//        SetupBusDividers(ahbDiv1, apbDiv2, apbDiv1);    // APB1: 30MHz max; APB2: 60MHz max
        SetupBusDividers(ahbDiv1, apbDiv4, apbDiv4);    // Peripheral freqs stay the same
        UpdateFreqValues();
        SetupFlashLatency(AHBFreqHz/1000000);
    }

    void PrintFreqs();

    // Clock output
    void MCO1Enable(Mco1Src_t Src, McoDiv_t Div);
    void MCO1Disable();
};

extern Clk_t Clk;

/*
 * Early initialization code.
 * This initialization must be performed just after stack setup and before
 * any other initialization.
 */
extern "C" {
void __early_init(void);
}
#endif

#endif /* CLOCKING_H_ */
