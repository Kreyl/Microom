/*
 * APDS_REG.h
 *
 *  Created on: 7 но€б. 2015 г.
 *      Author: Kreyl
 */

#ifndef APDS9960_H_
#define APDS9960_H_

#include "board.h"
#include "kl_lib.h"
#include "uart.h"

#if 1 // ==== Registers ====
#define APDS_REG_ENABLE         0x80
#define APDS_REG_ID             0x92
#define APDS_REG_ATIME          0x81
#define APDS_REG_WTIME          0x83
#define APDS_REG_AILTL          0x84
#define APDS_REG_AILTH          0x85
#define APDS_REG_AIHTL          0x86
#define APDS_REG_AIHTH          0x87
#define APDS_REG_PILT           0x89
#define APDS_REG_PIHT           0x8B
#define APDS_REG_PERS           0x8C
#define APDS_REG_CONFIG1        0x8D
#define APDS_REG_PPULSE         0x8E
#define APDS_REG_CONTROL        0x8F
#define APDS_REG_CONFIG2        0x90
#define APDS_REG_ID             0x92
#define APDS_REG_STATUS         0x93
#define APDS_REG_CDATAL         0x94
#define APDS_REG_CDATAH         0x95
#define APDS_REG_RDATAL         0x96
#define APDS_REG_RDATAH         0x97
#define APDS_REG_GDATAL         0x98
#define APDS_REG_GDATAH         0x99
#define APDS_REG_BDATAL         0x9A
#define APDS_REG_BDATAH         0x9B
#define APDS_REG_PDATA          0x9C
#define APDS_REG_POFFSET_UR     0x9D
#define APDS_REG_POFFSET_DL     0x9E
#define APDS_REG_CONFIG3        0x9F
#define APDS_REG_GPENTH         0xA0
#define APDS_REG_GEXTH          0xA1
#define APDS_REG_GCONF1         0xA2
#define APDS_REG_GCONF2         0xA3
#define APDS_REG_GOFFSET_U      0xA4
#define APDS_REG_GOFFSET_D      0xA5
#define APDS_REG_GOFFSET_L      0xA7
#define APDS_REG_GOFFSET_R      0xA9
#define APDS_REG_GPULSE         0xA6
#define APDS_REG_GCONF3         0xAA
#define APDS_REG_GCONF4         0xAB
#define APDS_REG_GFLVL          0xAE
#define APDS_REG_GSTATUS        0xAF
#define APDS_REG_IFORCE         0xE4
#define APDS_REG_PICLEAR        0xE5
#define APDS_REG_CICLEAR        0xE6
#define APDS_REG_AICLEAR        0xE7
#define APDS_REG_GFIFO_U        0xFC
#define APDS_REG_GFIFO_D        0xFD
#define APDS_REG_GFIFO_L        0xFE
#define APDS_REG_GFIFO_R        0xFF

// Bits
#define APDS_BIT_GVALID         0x01
#endif

#if 1 // ==== Default values ====
#define DEFAULT_GESTURE_PPULSE  0x89    // 16us, 10 pulses
#define DEFAULT_LDRIVE          ldv100mA
#define DEFAULT_PGAIN           pgv4x
#define DEFAULT_AGAIN           agv4x
#define DEFAULT_AILT            0xFFFF  // Force interrupt for calibration
#define DEFAULT_AIHT            0

#define DEFAULT_GGAIN           gGain4x
#define DEFAULT_GLDRIVE         ldv100mA
#define DEFAULT_GWTIME          gwTime2_8ms
#endif

#if 1 // Constants
// Modes
#define MODE_ALL_OFF            0
#define MODE_POWER              (1<<0)
#define MODE_ALS                (1<<1)
#define MODE_PROXIMITY          (1<<2)
#define MODE_WAIT               (1<<3)
#define MODE_AIEN               (1<<4)
#define MODE_PIEN               (1<<5)
#define MODE_GESTURE            (1<<6)

#define MAX_FIFO_CNT            32  // Const, do not touch

// LED Drive values
enum LedDrvValue_t {
    ldv100mA  = 0,
    ldv50mA   = 1,
    ldv25mA   = 2,
    ldv12_5mA = 3
};

enum LedBoost_t {
    lbst100 = (0<<4),
    lbst150 = (1<<4),
    lbst200 = (2<<4),
    lbst300 = (3<<4),
};

// Proximity Gain (PGAIN) values
enum ProxGainValue_t {
    pgv1x = (0<<2),
    pgv2x = (1<<2),
    pgv4x = (2<<2),
    pgv8x = (3<<2)
};

// ALS Gain (AGAIN) values
enum AmbGainValue_t {
    agv1x  = 0,
    agv4x  = 1,
    agv16x = 2,
    agv64x = 3
};

// Gesture Gain (GGAIN) values
enum GestGain_t {
    gGain1x = (0<<5),
    gGain2x = (1<<5),
    gGain4x = (2<<5),
    gGain8x = (3<<5),
};

// Gesture wait time values
enum GestWTime_t {
    gwTime0ms   = 0,
    gwTime2_8ms = 1,
    gwTime5_6ms = 2,
    gwTime8_4ms = 3,
    gwTime14_0ms = 4,
    gwTime22_4ms = 5,
    gwTime30_8ms = 6,
    gwTime39_2ms = 7
};

enum Gesture_t {
    gstNone, gstUp, gstDown, gstLeft, gstRight
};
#endif

struct RegValue_t {
    uint8_t Reg, Value;
} __attribute__((packed));
#define REG_VALUE_SZ    2

union InputData_t {
    uint32_t dw32;
    struct {
        uint8_t U, D, L, R;
    } __attribute__((packed));
} __attribute__((packed));
#define IN_DATA_SZ      4

#if 1 // ==== Gesture recognition ====
#define TOO_OLD_INTERVAL_MS     4005
#define FILTER_LEN              9
#define LOW_THRESHOLD           11
#define HIGH_THRESHOLD          36
class GestData_t {
private:
    uint32_t Indx;
    InputData_t Buf[FILTER_LEN];

public:
    uint32_t Count;
    InputData_t Filtered;
    RiseFall_t DetectEdge(uint8_t Current, uint8_t Prev);
    void Append(InputData_t New);
    void Reset();
};
#endif

class APDS9960_t {
private:
    InputData_t InputData[MAX_FIFO_CNT];
    i2c_t ii2c = {APDS_I2C, APDS_I2C_GPIO, APDS_SCL_PIN, APDS_SDA_PIN,
            APDS_I2C_BITRATE_HZ, APDS_DMA_TX, APDS_DMA_RX};
    uint8_t ReadReg(uint8_t Reg, uint8_t *p) { return ii2c.WriteRead(APDS_ADDR, &Reg, 1, p, 1); }
    uint8_t WriteReg(RegValue_t RV) {
//        Uart.Printf("\rWReg: %02X %02X", RV.Reg, RV.Value);
        return ii2c.Write(APDS_ADDR, (uint8_t*)&RV, REG_VALUE_SZ);
    }
    uint8_t EnableMode(uint8_t Mode);
    uint8_t SetLedDrv(LedDrvValue_t Value);
    uint8_t SetProximityGain(ProxGainValue_t Gain);
    uint8_t SetAmbientLightGain(AmbGainValue_t Gain);
    // Get and set light interrupt thresholds
    uint8_t SetLightIntLowThreshold(uint16_t Threshold);
    uint8_t SetLightIntHighThreshold(uint16_t Threshold);
    // Gesture
    uint8_t SetGestureGain(GestGain_t Gain);
    uint8_t SetGestureLEDDrive(LedDrvValue_t Value);
    uint8_t SetGestureWaitTime(GestWTime_t Time);
    uint8_t EnableGestureIrq();
    uint8_t EnableGestureMode();
    uint8_t EnablePower() { return EnableMode(MODE_POWER); }
    uint8_t DisableGestureIrq();
    uint8_t SetLedBoost(LedBoost_t Boost);
    // Data manipulation
    GestData_t Gest;
    uint8_t ReadFifo(uint8_t Count) {
        uint8_t Addr = APDS_REG_GFIFO_U;
        return ii2c.WriteRead(APDS_ADDR, &Addr, 1, (uint8_t*)InputData, (Count * IN_DATA_SZ));
    }
public:
    uint8_t Init();
    uint8_t EnableGestureSns();
    Gesture_t LastGesture = gstNone;
    // Inner use
    void ITask();
};

extern APDS9960_t Apds;

#endif /* APDS9960_H_ */
