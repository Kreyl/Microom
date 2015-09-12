/*
 * board.h
 *
 *  Created on: 12 сент. 2015 г.
 *      Author: Kreyl
 */

#ifndef BOARD_H_
#define BOARD_H_

#if 1 // ========================== GPIO =======================================
// UART
#define UART_GPIO       GPIOB
#define UART_TX_PIN     6
#define UART_RX_PIN     7
#define UART_AF         AF7

// PCM
#define PCM_CS_GPIO     GPIOA
#define PCM_CS_PIN      4

#define PCM_SPI_GPIO    GPIOA
#define PCM_SCK         5
#define PCM_MISO        6
#define PCM_MOSI        7
#define PCM_SPI_AF      AF5

#endif

#if 1 // =========================== SPI =======================================
#define PCM_SPI         SPI1
#define PCM_I2S         SPI2
#define PCM_I2S_RccEnable() rccEnableSPI2(FALSE);

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

// PCM
#define PCM_DMA_STREAM  STM32_DMA1_STREAM3
#define PCM_DMA_CHNL    0

#endif




#endif /* BOARD_H_ */
