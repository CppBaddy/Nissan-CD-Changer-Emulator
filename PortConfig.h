/*
 * PortConfig.h
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

#ifndef CDCHANGER_PORTCONFIG_H
#define CDCHANGER_PORTCONFIG_H

#include <avr/io.h>

#define TIMER0_PRESCALER    8
#define TIMER1_PRESCALER    8

#define UART_9600BAUD       9600

#define TIMER0_RELOAD       (F_CPU/TIMER0_PRESCALER/UART_9600BAUD)

#if (TIMER0_RELOAD > 255)
#error "Timer0 reload value is greater then 8 bits. Consider increasing prescaler value."
#elif (TIMER0_RELOAD == 0)
#error "Timer0 reload value is zero. Consider decreasing prescaler value."
#endif

#define TIMER1_RELOAD       (F_CPU/TIMER1_PRESCALER/UART_9600BAUD)

#if (TIMER1_RELOAD > 255)
#error "Timer1 reload value is greater then 8 bits. Consider increasing prescaler value."
#elif (TIMER1_RELOAD == 0)
#error "Timer1 reload value is zero. Consider decreasing prescaler value."
#endif


#define HuRxPORT  PB0  // from head unit: Rx
#define HuTxPORT  PB1  // to head unit: /Tx
#define HuReqPORT PB2  // to head unit: /Request


#define MpRxPORT  PB3  // from player: Rx
#define MpTxPORT  PB4  // to player: Tx


#endif //CDCHANGER_PORTCONFIG_H
