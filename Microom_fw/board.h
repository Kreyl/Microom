/*
 * board.h
 *
 *  Created on: 12 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <inttypes.h>

// General
#define CRYSTAL_FREQ_HZ     12000000    // Freq of external crystal

// USB
#define USBDrv      	USBD1   // USB driver to use

#if 1 // ========================== GPIO =======================================
// UART
#define UART_GPIO       GPIOB
#define UART_TX_PIN     6
#define UART_RX_PIN     7
#define UART_AF         AF7

// LEDs
#define LED1_PIN        2
#define LED1_GPIO       GPIOB
#define LED2_PIN        11
#define LED2_GPIO       GPIOB
#define LED3_PIN        1
#define LED3_GPIO       GPIOC
#define LED4_PIN        4
#define LED4_GPIO       GPIOC
#define LED5_PIN        8
#define LED5_GPIO       GPIOB
#define LED6_PIN        0
#define LED6_GPIO       GPIOC
#define LED7_PIN        9
#define LED7_GPIO       GPIOB
#define LED8_PIN        10
#define LED8_GPIO       GPIOA
#define LEDAUX_PIN      10
#define LEDAUX_GPIO     GPIOB

// USB
#define USB_GPIO		GPIOA
#define USB_DM_PIN		11
#define USB_DP_PIN		12
#endif

#if 1 // =========================== SPI =======================================

#endif

#if 1 // ========================== USART ======================================
#define UART            USART1
#define UART_TX_REG     UART->DR
#define UART_RX_REG     UART->DR
#endif

#if 1 // =========================== DMA =======================================
// Uart
#define UART_DMA_TX     STM32_DMA2_STREAM7
#define UART_DMA_RX     STM32_DMA2_STREAM5
#define UART_DMA_CHNL   4

// ADC
#define ADC_DMA         STM32_DMA2_STREAM4
#define ADC_DMA_MODE    STM32_DMA_CR_CHSEL(0) |   /* DMA2 Stream4 Channel0 */ \
                        DMA_PRIORITY_LOW | \
                        STM32_DMA_CR_MSIZE_HWORD | \
                        STM32_DMA_CR_PSIZE_HWORD | \
                        STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                        STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                        STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */

#endif

#if 1 // =========================== ADC =======================================
// Clock divider: clock is generated from the APB2
#define ADC_CLK_DIVIDER		adcDiv2	// 32MHz / 2 = 16MHz

// ADC channels: LED3, LED6
#define SNS_CHNL0 	        10
#define SNS_CHNL1 	        11

#define ADC_VREFINT_CHNL    17  // All 4xx devices. Do not change.
const uint8_t AdcChannels[] = { SNS_CHNL0, SNS_CHNL1 };
#define ADC_CHANNEL_CNT     2   // Do not use countof(AdcChannels) as preprocessor does not know what is countof => cannot check
#define ADC_SAMPLE_TIME     ast55d5Cycles
#define ADC_SAMPLE_CNT      8   // How many times to measure every channel

#define ADC_MAX_SEQ_LEN     16  // 1...16	// Const, see ref man
#define ADC_SEQ_LEN         (ADC_SAMPLE_CNT * ADC_CHANNEL_CNT)
#if (ADC_SEQ_LEN > ADC_MAX_SEQ_LEN) || (ADC_SEQ_LEN == 0)
#error "Wrong ADC channel count and sample count"
#endif
#endif

#endif /* BOARD_H_ */
