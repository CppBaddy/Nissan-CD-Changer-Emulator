/*
 * Uart.h
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

#ifndef CDCHANGER_UART_H
#define CDCHANGER_UART_H

#include "Fifo.h"


enum eUartState
{
  Idle = 0,
  StartFrame,
  StartBit,
  Bit0,
  Bit1,
  Bit2,
  Bit3,
  Bit4,
  Bit5,
  Bit6,
  Bit7,
  ParityBit,
  StopBit = ParityBit,
  StopBit2
};


struct Uart
{
  uint8_t rxState;
  uint8_t rxBuf;
  uint8_t rxParity;
  struct Fifo rxFifo;

  uint8_t txState;
  uint8_t txBuf;
  uint8_t txParity;
  struct Fifo txFifo;
};


#endif //CDCHANGER_UART_H
