/*
 * apds9960.h
 *
 *  Created on: 7 но€б. 2015 г.
 *      Author: Kreyl
 */

#ifndef APDS9960_H_
#define APDS9960_H_

#include "board.h"
#include "kl_lib.h"

class APDS9960_t {
private:
    i2c_t ii2c = {APDS_I2C, APDS_I2C_GPIO, APDS_SCL_PIN, APDS_SDA_PIN,
            APDS_I2C_BITRATE_HZ, APDS_DMA_TX, APDS_DMA_RX};

public:
    void Init();
    void Ping();
};




#endif /* APDS9960_H_ */
