/*
 * MpUart.h
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

#ifndef CDCHANGER_MPUART_H
#define CDCHANGER_MPUART_H

#include <stdbool.h>

#include "Uart.h"


void Mp_Init();
bool Mp_RxReady();
uint8_t Mp_GetByte();
void Mp_PutByte(uint8_t b);

void Mp_RxStart();

extern volatile struct Uart mp;


#endif //CDCHANGER_MPUART_H
