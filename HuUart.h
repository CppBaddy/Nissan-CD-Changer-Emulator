/*
 * HuUart.h
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

#ifndef CDCHANGER_HUUART_H
#define CDCHANGER_HUUART_H

#include "Uart.h"
#include <stdbool.h>


void Hu_Init();
bool Hu_RxReady();
uint8_t Hu_GetByte();
void Hu_PutByte(uint8_t b);

void Hu_RxStart();

void Hu_SetRTS();
void Hu_ClrRTS();

extern volatile struct Uart hu;


#endif //CDCHANGER_HUUART_H
