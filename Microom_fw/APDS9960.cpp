/*
 * APDS9960.cpp
 *
 *  Created on: 7 но€б. 2015 г.
 *      Author: Kreyl
 */

#include "apds9960.h"
#include "uart.h"

void APDS9960_t::Init() {
    ii2c.Init();
//    ii2c.ScanBus();
}

void APDS9960_t::Ping() {
    uint8_t b[2];
    b[0] = 0x80;
    b[1] = 0;
    uint8_t r = ii2c.Write(APDS_ADDR, b, 2);
    Uart.Printf("\r WR = %u", r);
}

