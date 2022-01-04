/*
 * Timeout.c
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


#include <avr/interrupt.h>
#include "Timeout.h"
#include "PortConfig.h"

#define TIMEOUT_MS		11



volatile uint8_t gTime; //global Time
volatile uint8_t gTick; // 10ms tick
volatile bool gPPS;
volatile uint8_t gTimeOut;

volatile uint8_t gDelayFlag;




