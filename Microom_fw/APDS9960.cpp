/*
 * APDS9960.cpp
 *
 *  Created on: 7 но€б. 2015 г.
 *      Author: Kreyl
 */

#include "apds9960.h"

#if 1 // ==== Init values ====
static const RegValue_t InitValues[] = {
        {APDS_REG_ENABLE, 0},       // Disable all features
        // Default values for ambient light and proximity registers
        {APDS_REG_ATIME, 219},      // 103ms
        {APDS_REG_WTIME, 246},      // 27ms
        {APDS_REG_PPULSE, DEFAULT_GESTURE_PPULSE},
        {APDS_REG_POFFSET_UR, 0},   // 0 offset
        {APDS_REG_POFFSET_DL, 0},   // 0 offset
        {APDS_REG_CONFIG1, 0x60},   // No 12x wait (WTIME) factor
        {APDS_REG_PILT, 0},         // Low proximity threshold
        {APDS_REG_PIHT, 50},        // High proximity threshold
        {APDS_REG_PERS, 0x11},      // 2 consecutive prox or ALS for int.
        {APDS_REG_CONFIG2, 0x01},   // No saturation interrupts or LED boost
        {APDS_REG_CONFIG3, 0},      // Enable all photodiodes, no SAI
        // Default values for gesture sense registers
        {APDS_REG_GPENTH, 40},      // Threshold for entering gesture mode
        {APDS_REG_GEXTH, 30},       // Threshold for exiting gesture mode
        {APDS_REG_GCONF1, 0x40},    // 4 gesture events for int., 1 for exit
        {APDS_REG_GOFFSET_U, 0},    // No offset scaling for gesture mode
        {APDS_REG_GOFFSET_D, 0},    // No offset scaling for gesture mode
        {APDS_REG_GOFFSET_L, 0},    // No offset scaling for gesture mode
        {APDS_REG_GOFFSET_R, 0},    // No offset scaling for gesture mode
        {APDS_REG_GPULSE, 0xC9},    // 32us, 10 pulses
        {APDS_REG_GCONF3, 0},       // All photodiodes active during gesture
};
#define INIT_VALUES_CNT     countof(InitValues)
#endif

APDS9960_t Apds;

#if 1 // ================================ Gest_t ===============================
void GestData_t::Reset() {
    Count = 0;
    Filtered.dw32 = 0;
    for(int i=0; i<FILTER_LEN; i++) Buf[i].dw32 = 0;
    Indx = 0;
}

void GestData_t::Append(InputData_t New) {
    Buf[Indx++].dw32 = New.dw32;
    if(Indx >= FILTER_LEN) Indx = 0;
    // Calc average
    uint32_t u=0, d=0, r=0, l=0;
    for(int i=0; i<FILTER_LEN; i++) {
        u += Buf[i].U;
        d += Buf[i].D;
        r += Buf[i].R;
        l += Buf[i].L;
    }
    u /= FILTER_LEN;
    d /= FILTER_LEN;
    r /= FILTER_LEN;
    l /= FILTER_LEN;
    Filtered.U = u;
    Filtered.D = d;
    Filtered.R = r;
    Filtered.L = l;
}

RiseFall_t GestData_t::DetectEdge(uint8_t Current, uint8_t Prev) {
    // Ignore intermediate values
//    if(Current > LOW_THRESHOLD and Current < HIGH_THRESHOLD)
        return rfNone;

}

#endif

#if 1 // =========================== Task ======================================
static THD_WORKING_AREA(waAPDS, 256);
static THD_FUNCTION(APDSThread, arg) {
    chRegSetThreadName("Apds");
    Apds.ITask();
}

__attribute__((__noreturn__))
void APDS9960_t::ITask() {
    uint8_t b=0, r, FifoLvl=0;
    systime_t LastDataTime = 0;
    while(true) {
        chThdSleepMilliseconds(27);
        // Check if there is data
        if((r = ReadReg(APDS_REG_GSTATUS, &b)) != OK) continue;
        if(!(b & APDS_BIT_GVALID)) {    // No Data
            // Reset data if too old
            if(Gest.Count != 0) {
                chSysLock();
                systime_t Now = chVTGetSystemTimeX();
                if(chVTTimeElapsedSinceX(LastDataTime) > TOO_OLD_INTERVAL_MS) {
                    Gest.Reset();
                    LastDataTime = Now;
                }
                chSysUnlock();
            }
            continue;
        }
        // Get FIFO level
        if((r = ReadReg(APDS_REG_GFLVL, &FifoLvl)) != OK) continue;
//        Uart.Printf("FifoLvl=%u\r", FifoLvl);
        if(FifoLvl > 0) {
            if((r = ReadFifo(FifoLvl)) != OK) continue;
//            systime_t Now = chVTGetSystemTime();
            // Search edges
//            RiseFall_t Edge;
            for(uint8_t i=0; i<FifoLvl; i++) {
//                Gest.Append(InputData[i]);
//                Uart.Printf("%u %u %u %u\r", InputData[i].U, InputData[i].D, InputData[i].L, InputData[i].R);
                Uart.Printf("%u\t%u\r", InputData[i].U, InputData[i].D);
//                Uart.Printf("%u\t%u\r", Gest.Filtered.U, Gest.Filtered.D);
//                Uart.Printf("%u %u %u %u\r", Gest.Filtered.U, Gest.Filtered.D, Gest.Filtered.L, Gest.Filtered.R);

                //                Edge = Gest.DetectEdge(InputData[i].D, Gest.Prev.D);
//                if(Edge == rfRising) Uart.Printf("%u D In\r", Gest.ID);
//                else if(Edge == rfFalling) Uart.Printf("%u D Out\r", Gest.ID);
//                Gest.Prev.D = InputData[i].D;
//
//                Edge = Gest.DetectEdge(InputData[i].U, Gest.Prev.U);
//                if(Edge == rfRising) Uart.Printf("%u U In\r", Gest.ID);
//                else if(Edge == rfFalling) Uart.Printf("%u U Out\r", Gest.ID);
//                Gest.Prev.U = InputData[i].U;
//                if(
            }

        }
    } // while true
}
#endif

uint8_t APDS9960_t::Init() {
    ii2c.Init();
//    ii2c.ScanBus();
    uint8_t b = 0, r;
    // Check ID
    r = ReadReg(APDS_REG_ID, &b);
    if(r == OK) {
        if(!(b == 0xAB or b == 0x9C)) {
            Uart.Printf("\rBad APDS ID: 0x%X", b);
            return FAILURE;
        }
    }
    else {
        Uart.Printf("\rAPDS read err %u", r);
        return FAILURE;
    }

    // Write default values
    for(uint32_t i=0; i<INIT_VALUES_CNT; i++) {
        r = WriteReg(InitValues[i]);
        if(r != OK) {
            Uart.Printf("\rAPDS write err %u at %u", r, i);
            return FAILURE;
        }
    }

    if(SetLedDrv(DEFAULT_LDRIVE) != OK) return FAILURE;
    if(SetProximityGain(DEFAULT_PGAIN) != OK) return FAILURE;
    if(SetAmbientLightGain(DEFAULT_AGAIN) != OK) return FAILURE;
    if(SetLightIntLowThreshold(DEFAULT_AILT) != OK) return FAILURE;
    if(SetLightIntHighThreshold(DEFAULT_AIHT) != OK) return FAILURE;

    if(SetGestureGain(DEFAULT_GGAIN) != OK) return FAILURE;
    if(SetGestureLEDDrive(DEFAULT_GLDRIVE) != OK) return FAILURE;
    if(SetGestureWaitTime(DEFAULT_GWTIME) != OK) return FAILURE;
    if(DisableGestureIrq() != OK) return FAILURE;

    // Thread
    chThdCreateStatic(waAPDS, sizeof(waAPDS), NORMALPRIO, APDSThread, NULL);
    return OK;
}

uint8_t APDS9960_t::EnableGestureSns() {
    uint8_t r;
    Gest.Reset();
    if((r = WriteReg({APDS_REG_WTIME, 0xFF})) != OK) return r;
    if((r = WriteReg({APDS_REG_PPULSE, DEFAULT_GESTURE_PPULSE})) != OK) return r;
    if((r = SetLedBoost(lbst300)) != OK) return r;
    if((r = EnableGestureMode()) != OK) return r;
    if((r = EnablePower()) != OK) return r;
    return EnableMode(MODE_WAIT | MODE_PROXIMITY | MODE_GESTURE);
}

#if 1 // ====================== Auxilary subroutines ===========================
uint8_t APDS9960_t::EnableMode(uint8_t Mode) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_ENABLE, &b)) != OK) return r;
    // Set bits in register to given value
    b |= Mode;
    return WriteReg({APDS_REG_ENABLE, b});
}

uint8_t APDS9960_t::SetLedDrv(LedDrvValue_t Value) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_CONTROL, &b)) != OK) return r;
    // Set bits in register to given value
    b &= 0b00111111;
    b |= ((uint8_t)Value) << 6;
    return WriteReg({APDS_REG_CONTROL, b});
}

uint8_t APDS9960_t::SetProximityGain(ProxGainValue_t Gain) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_CONTROL, &b)) != OK) return r;
    b &= 0b11110011;
    b |= (uint8_t)Gain;
    return WriteReg({APDS_REG_CONTROL, b});
}

uint8_t APDS9960_t::SetAmbientLightGain(AmbGainValue_t Gain) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_CONTROL, &b)) != OK) return r;
    b &= 0b11111100;
    b |= (uint8_t)Gain;
    return WriteReg({APDS_REG_CONTROL, b});
}

uint8_t APDS9960_t::SetLightIntLowThreshold(uint16_t Threshold) {
    uint8_t r;
    if((r = WriteReg({APDS_REG_AILTL, (uint8_t)(Threshold & 0xFF)})) != OK) return r;
    return WriteReg({APDS_REG_AILTH, (uint8_t)(Threshold >> 8)});
}
uint8_t APDS9960_t::SetLightIntHighThreshold(uint16_t Threshold) {
    uint8_t r;
    if((r = WriteReg({APDS_REG_AIHTL, (uint8_t)(Threshold & 0xFF)})) != OK) return r;
    return WriteReg({APDS_REG_AIHTH, (uint8_t)(Threshold >> 8)});
}

uint8_t APDS9960_t::SetGestureGain(GestGain_t Gain) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_GCONF2, &b)) != OK) return r;
    b &= 0b10011111;
    b |= (uint8_t)Gain;
    return WriteReg({APDS_REG_GCONF2, b});
}

uint8_t APDS9960_t::SetGestureLEDDrive(LedDrvValue_t Value) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_GCONF2, &b)) != OK) return r;
    // Set bits in register to given value
    b &= 0b11100111;
    b |= ((uint8_t)Value) << 3;
    return WriteReg({APDS_REG_GCONF2, b});
}

uint8_t APDS9960_t::SetGestureWaitTime(GestWTime_t Time) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_GCONF2, &b)) != OK) return r;
    // Set bits in register to given value
    b &= 0b11111000;
    b |= (uint8_t)Time;
    return WriteReg({APDS_REG_GCONF2, b});
}

uint8_t APDS9960_t::DisableGestureIrq() {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_GCONF4, &b)) != OK) return r;
    // Set bits in register to given value
    b &= 0b11111101;
    return WriteReg({APDS_REG_GCONF4, b});
}

uint8_t APDS9960_t::SetLedBoost(LedBoost_t Boost) {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_CONFIG2, &b)) != OK) return r;
    // Set bits in register to given value
    b &= 0b11001111;
    b |= (uint8_t)Boost;
    return WriteReg({APDS_REG_CONFIG2, b});
}

uint8_t APDS9960_t::EnableGestureMode() {
    uint8_t b=0, r;
    if((r = ReadReg(APDS_REG_GCONF4, &b)) != OK) return r;
    // Set bits in register to given value
    b |= 1;
    return WriteReg({APDS_REG_GCONF4, b});
}

#endif
