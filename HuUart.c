/*
 * HuUart.c
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

#include <avr/io.h>
#include <avr/iotn85.h>
#include <avr/interrupt.h>

#include "PortConfig.h"
#include "HuUart.h"
#include "Timeout.h"


volatile struct Uart hu;



void Hu_RxReset();
//void Hu_RxStart();
void Hu_TxStart();
void Hu_TxStop();
void Hu_SetTxPort(uint8_t b);

void Hu_SetRTS() //request to send
{
    PORTB |= _BV(HuReqPORT);
}

void Hu_ClrRTS()
{
    PORTB &= ~_BV(HuReqPORT);
}

void Hu_Init()
{
    hu.rxState = Idle;
    hu.rxFifo.head = 0;
    hu.rxFifo.tail = 0;

    hu.txState = Idle;
    hu.txFifo.head = 0;
    hu.txFifo.tail = 0;

    Hu_RxReset();
    Hu_SetTxPort(1); //SPACE

    Hu_ClrRTS(); //REQ
}

void Hu_RxStart()
{
    //TCNT0  = 0;                    //clear timer0
    //GTCCR &= ~_BV(TSM);            //start timer0 and clear prescaler
#if  TIMER0_RELOAD > 168
#error  This math does not work for timer reload value > 168
#endif
    OCR0B  = (TCNT0 + TIMER0_RELOAD/2) % TIMER0_RELOAD; //capture exact middle of the bit

    TIMSK |= _BV(OCIE0B);          //enable CompareMatch0B interrupt

    hu.rxState = StartFrame;

    PCMSK &= ~_BV(HuRxPORT);       //disable pc interrupt for HU
}

void Hu_RxReset()
{
    //GTCCR |= _BV(TSM) | _BV(PSR0); //stop timer0
    TIMSK &= ~_BV(OCIE0B);         //disable CompareMatch0B interrupt
    PCMSK |= _BV(HuRxPORT);        //enable pc interrupt for HU
    hu.rxState = Idle;
}

bool Hu_RxReady()
{
    return hu.rxFifo.tail != hu.rxFifo.head;
}

uint8_t Hu_GetByte()
{
    uint8_t b = hu.rxFifo.buf[hu.rxFifo.tail];
    hu.rxFifo.tail = (hu.rxFifo.tail + 1) % MAX_FIFO_BUF_SIZE;
    return b;
}

uint8_t calcParityBit(uint8_t b)
{
    b ^= (b >> 4);
    b ^= (b >> 2);
    b ^= (b >> 1);

    return b & 1;
}

void Hu_PutByte(uint8_t b)
{
    uint8_t head = (hu.txFifo.head + 1) % MAX_FIFO_BUF_SIZE;

    while(head == hu.txFifo.tail) ; //busy wait for free space

    hu.txFifo.buf[hu.txFifo.head] = b;

    hu.txFifo.head = head;

    Hu_TxStart();
}

inline void Hu_TxStart()
{
    TIMSK |= _BV(OCIE0A);          //enable interrupt for CompareMatch0A
}

inline void Hu_TxStop()
{
    TIMSK &= ~_BV(OCIE0A);          //disable interrupt for CompareMatch0A
}

void Hu_SetTxPort(uint8_t b)
{
    if(b) //logic polarity control
    {
        PORTB &= ~_BV(HuTxPORT);
    }
    else
    {
        PORTB |= _BV(HuTxPORT);
    }
}

/* Timer0 compare match A interrupt handler */
//HU Serial Transmit driver
ISR( TIMER0_COMPA_vect )
{
    if(hu.txState == Idle)
    {
        if(hu.txFifo.tail != hu.txFifo.head)
        {
            hu.txBuf = hu.txFifo.buf[hu.txFifo.tail];
            hu.txState = StartFrame;
            hu.txFifo.tail = (hu.txFifo.tail + 1) % MAX_FIFO_BUF_SIZE;

            hu.txParity = calcParityBit(hu.txBuf);

            resetTimeout(); //reset timeout before each Tx
        }
    }
    else
    {
        ++(hu.txState); //next state

        switch(hu.txState)
        {
        case StartBit:
            Hu_SetTxPort(0);
            break;
        case Bit0:
        case Bit1:
        case Bit2:
        case Bit3:
        case Bit4:
        case Bit5:
        case Bit6:
        case Bit7:
            Hu_SetTxPort(hu.txBuf & 1);
            hu.txBuf >>= 1;
            break;
        case ParityBit:
            Hu_SetTxPort(hu.txParity);
            break;
        case StopBit2:
        {
            Hu_SetTxPort(1);

            volatile struct Fifo* fifo = &hu.txFifo;

            if(fifo->tail == fifo->head) //no more data to send
            {
                Hu_TxStop();
            }

            hu.txState = Idle;
            break;
        }

        default:
            break;
        }
    }
}

/* Timer0 compare match B interrupt handler */
//HU Serial Receive driver
ISR( TIMER0_COMPB_vect )
{
    if(hu.rxState != Idle)
    {
        ++(hu.rxState); //next state

        switch(hu.rxState)
        {
        case StartBit:
            if(PINB & _BV(HuRxPORT)) //false start?
            {
                Hu_RxReset();
            }
            break;
        case Bit0:
        case Bit1:
        case Bit2:
        case Bit3:
        case Bit4:
        case Bit5:
        case Bit6:
        case Bit7:
            hu.rxBuf >>= 1;
            if(PINB & _BV(HuRxPORT))
            {
                hu.rxBuf |= 0x80;
            }
            break;
        case ParityBit:
            if(calcParityBit(hu.rxBuf) != ((PINB & _BV(HuRxPORT)) >> HuRxPORT)) //parity error
            {
                Hu_RxReset();
            }
            break;
        case StopBit2:
            if(PINB & _BV(HuRxPORT))
            {
                volatile struct Fifo* fifo = &hu.rxFifo;

                //NOTE:
                // No check for available space!
                // Main loop must read out faster then new data come
                fifo->buf[fifo->head] = hu.rxBuf;
                fifo->head = (fifo->head + 1) % MAX_FIFO_BUF_SIZE;
            }

            Hu_RxReset();
            break;
        default:
            break;
        }
    }
}
