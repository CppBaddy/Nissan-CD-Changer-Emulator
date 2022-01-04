/*
 * Timeout.h
 *
 * Copyright (c) 2018 Yulay Rakhmangulov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include <stdbool.h>

#define resetTimeout10ms()	resetTimeout()

#define kPpsPeriod			38

void resetTimeout();

void resetTimeout1ms();

bool isTimedOut();

extern volatile bool gPPS; //Pulse Per Second
extern volatile uint8_t gTime; //global Time Tick
extern volatile uint8_t gTick;
extern volatile uint8_t gTimeOut;

extern volatile uint8_t gDelayFlag;

#define kTimeoutTicks	156 //(TIMEOUT_MS * UART_9600BAUD)/1000

#define kDelay1msTicks	15

inline void resetTimeout()
{
	gTimeOut = kTimeoutTicks;
}

inline void resetTimeout1ms()
{
	gTimeOut = kDelay1msTicks;
}

inline bool isTimedOut()
{
	return (0 == gTimeOut);
}


#endif /* TIMEOUT_H_ */
