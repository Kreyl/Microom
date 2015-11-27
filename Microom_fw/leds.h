/*
 * leds.h
 *
 *  Created on: 27 ����. 2015 �.
 *      Author: Kreyl
 */

#ifndef LEDS_H_
#define LEDS_H_

#include "board.h"
#include "kl_lib.h"

extern PinOutput_t LedAux;

extern PinOutputPWM_t<LED_TOP_VALUE, invNotInverted, omPushPull> Led[4];


#endif /* LEDS_H_ */
