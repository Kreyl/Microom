/*
 * cmd_uart.h
 *
 *  Created on: 15.04.2013
 *      Author: kreyl
 */

#ifndef UART_H_
#define UART_H_

#include <kl_lib.h>
#include "kl_sprintf.h"
#include <cstring>
#include "cmd.h"

// Set to true if RX needed
#define UART_RX_ENABLED     FALSE

// UART
#define UART                USART1
#define UART_RCC_ENABLE()   rccEnableUSART1(FALSE)
#define UART_RCC_DISABLE()  rccDisableUSART1(FALSE)

#if defined STM32L1XX_MD
#define UART_AF             AF7 // for USART1 @ GPIOA
#define UART_TX_REG         UART->DR
#define UART_RX_REG         UART->DR
#define UART_DMA_TX         STM32_DMA1_STREAM4
#define UART_DMA_RX         STM32_DMA1_STREAM5
#elif defined STM32F030
#define UART_AF             AF1 // for USART1 @ GPIOA
#define UART_TX_REG         UART->TDR
#define UART_RX_REG         UART->RDR
#define UART_DMA_TX         STM32_DMA1_STREAM2
#define UART_DMA_RX         STM32_DMA1_STREAM3
#elif defined STM32F2XX || defined STM32F4XX
#define UART_AF             AF7
#define UART_TX_REG         UART->DR
#define UART_RX_REG         UART->DR
#define UART_DMA_TX         STM32_DMA2_STREAM7
#define UART_DMA_RX         STM32_DMA2_STREAM5
#endif
#define UART_DMA_CHNL       4

// ==== TX ====
#define UART_TXBUF_SZ       207
#define UART_GPIO           GPIOB
#define UART_TX_PIN         6

#define UART_DMA_TX_MODE    STM32_DMA_CR_CHSEL(UART_DMA_CHNL) | \
                            DMA_PRIORITY_LOW | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_M2P |    /* Direction is memory to peripheral */ \
                            STM32_DMA_CR_TCIE         /* Enable Transmission Complete IRQ */

#if UART_RX_ENABLED // ==== RX ====
#define UART_RXBUF_SZ       99 // unprocessed bytes
#define UART_CMD_BUF_SZ     54 // payload bytes
#define UART_RX_PIN         7

#define UART_RX_POLLING_MS  99
#define UART_DMA_RX_MODE    STM32_DMA_CR_CHSEL(UART_DMA_CHNL) | \
                            DMA_PRIORITY_MEDIUM | \
                            STM32_DMA_CR_MSIZE_BYTE | \
                            STM32_DMA_CR_PSIZE_BYTE | \
                            STM32_DMA_CR_MINC |       /* Memory pointer increase */ \
                            STM32_DMA_CR_DIR_P2M |    /* Direction is peripheral to memory */ \
                            STM32_DMA_CR_CIRC         /* Circular buffer enable */


typedef Cmd_t<99> UartCmd_t;
#endif

class Uart_t {
private:
    char TXBuf[UART_TXBUF_SZ];
    char *PRead, *PWrite;
    bool IDmaIsIdle;
    uint32_t IFullSlotsCount, ITransSize;
    uint32_t IBaudrate;
#if UART_RX_ENABLED
    int32_t SzOld, RIndx;
    uint8_t IRxBuf[UART_RXBUF_SZ];
    Thread *IPThd;
#endif
    void ISendViaDMA();
public:
    void Printf(const char *S, ...);
    void PrintfI(const char *S, ...);
    void FlushTx() { while(!IDmaIsIdle); }  // wait DMA
    void PrintfNow(const char *S, ...);
    void Init(uint32_t ABaudrate);
    void DeInit() {
        UART->CR1 &= ~USART_CR1_UE; // UART Disable
        UART_RCC_DISABLE();
    }
    void OnAHBFreqChange();
    // Inner use
    void IRQDmaTxHandler();
    void IPutChar(char c);
    void IPrintf(const char *format, va_list args);
#if UART_RX_ENABLED
    UartCmd_t Cmd;
    void SignalCmdProcessed();
    void IRxTask();
    // Command and reply
    void Reply(const char* CmdCode, int32_t Data) { Printf("%S,%d\r\n", CmdCode, Data); }
    void Ack(int32_t Result) { Printf("\r\nAck %d\r\n", Result); }
#endif
    Uart_t() {
//        for(uint32_t i=0; i<UART_TXBUF_SZ; i++) TXBuf[i] = 0;
        PWrite = TXBuf;
        PRead = TXBuf;
        IDmaIsIdle = true;
        IFullSlotsCount = 0;
        ITransSize = 0;
        IBaudrate = 115200;
#if UART_RX_ENABLED
//        for(uint32_t i=0; i<UART_RXBUF_SZ; i++) IRxBuf[i] = 0;
        SzOld=0;
        RIndx=0;
        IPThd = nullptr;
#endif
    }
};

extern Uart_t Uart;

#endif /* UART_H_ */
