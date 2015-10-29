/*
 * board.h
 *
 *  Created on: 12 ����. 2015 �.
 *      Author: Kreyl
 */

#ifndef BOARD_H_
#define BOARD_H_

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

#endif

#endif /* BOARD_H_ */
