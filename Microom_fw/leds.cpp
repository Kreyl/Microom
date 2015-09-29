/*
 * leds.cpp
 *
 *  Created on: 27 ����. 2015 �.
 *      Author: Kreyl
 */

#include "leds.h"

PinOutput_t Led[8] = {
        {LED1_GPIO, LED1_PIN, omPushPull},
        {LED2_GPIO, LED2_PIN, omPushPull},
        {LED3_GPIO, LED3_PIN, omPushPull},
        {LED4_GPIO, LED4_PIN, omPushPull},
        {LED5_GPIO, LED5_PIN, omPushPull},
        {LED6_GPIO, LED6_PIN, omPushPull},
        {LED7_GPIO, LED7_PIN, omPushPull},
        {LED8_GPIO, LED8_PIN, omPushPull},
};

PinOutput_t LedAux {LEDAUX_GPIO, LEDAUX_PIN, omPushPull};
