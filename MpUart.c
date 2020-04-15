/*
 * MpUart.c
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
#include "MpUart.h"
#include "Timeout.h"

//PlayerControl flags
extern volatile uint8_t gDelayFlag;

volatile struct Uart mp;



void Mp_RxReset();
//void Mp_RxStart();
void Mp_TxStart();
//void Mp_TxStop();
void Mp_SetTxPort(uint8_t b);

void Mp_Init()
{
    mp.rxState = Idle;
    mp.rxFifo.head = 0;
    mp.rxFifo.tail = 0;

    mp.txState = Idle;
    mp.txFifo.head = 0;
    mp.txFifo.tail = 0;

    Mp_RxReset();
    Mp_SetTxPort(1); //SPACE

    Mp_TxStart(); //to keep timeout counter running
}

void Mp_RxStart()
{
    //GTCCR  |= _BV(PSR1);           //clear prescaler
    //TCNT1 = 0;                     //clear timer1
#if  TIMER1_RELOAD > 168
#error  This math does not work for timer reload value > 168
#endif
    OCR1A  = (TCNT1 + TIMER1_RELOAD/2) % TIMER1_RELOAD; //capture exact middle of the bit

    TIMSK |= _BV(OCIE1A);          //enable interrupt for CompareMatch1A

    mp.rxState = StartFrame;

    PCMSK &= ~_BV(MpRxPORT);       //disable pc interrupt for CD
}

void Mp_RxReset()
{
    TIMSK &= ~_BV(OCIE1A);         //disable interrupt for CompareMatch1A
    PCMSK |= _BV(MpRxPORT);        //enable pc interrupt for CD
    mp.rxState = Idle;
}

bool Mp_RxReady()
{
    return mp.rxFifo.tail != mp.rxFifo.head;
}

uint8_t Mp_GetByte()
{
    uint8_t b = mp.rxFifo.buf[mp.rxFifo.tail];
    mp.rxFifo.tail = (mp.rxFifo.tail + 1) % MAX_FIFO_BUF_SIZE;
    return b;
}

void Mp_PutByte(uint8_t b)
{
    uint8_t head = (mp.txFifo.head + 1) % MAX_FIFO_BUF_SIZE;

    while(head == mp.txFifo.tail); //busy wait for free space

    mp.txFifo.buf[mp.txFifo.head] = b;

    mp.txFifo.head = head;

//    Mp_TxStart(); //don't need to call, as we run continuously
}

void Mp_TxStart()
{
    TIMSK |= _BV(OCIE1B);          //enable interrupt for CompareMatch1B
}

//void Mp_TxStop()
//{
//    TIMSK &= ~_BV(OCIE1B);          //disable interrupt for CompareMatch1B
//}

void Mp_SetTxPort(uint8_t b)
{
    if(b)
    {
        PORTB |= _BV(MpTxPORT);
    }
    else
    {
        PORTB &= ~_BV(MpTxPORT);
    }
}

/* Timer1 interrupt handler */
//MP Serial Receive driver
ISR( TIMER1_COMPA_vect )
{
    if(mp.rxState != Idle)
    {
        ++(mp.rxState); //next state

        switch(mp.rxState)
        {
        case StartBit:
            if(PINB & _BV(MpRxPORT)) //false start?
            {
                Mp_RxReset();
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
            mp.rxBuf >>= 1;
            if(PINB & _BV(MpRxPORT))
            {
                mp.rxBuf |= 0x80;
            }
            break;
        case StopBit:
        case StopBit2:
            if(PINB & _BV(MpRxPORT))
            {
                volatile struct Fifo* fifo = &mp.rxFifo;

                //NOTE:
                // No check for available space!
                // Main loop must read out faster then new data come
                fifo->buf[fifo->head] = mp.rxBuf;
                fifo->head = (fifo->head + 1) % MAX_FIFO_BUF_SIZE;
            }

            Mp_RxReset();
            break;
        default:
            break;
        }
    }
}

/* Timer1 interrupt handler */
//MP Serial Transmit driver
ISR( TIMER1_COMPB_vect )
{
    if(mp.txState == Idle)
    {
        if(mp.txFifo.tail != mp.txFifo.head) //fifo is not empty
        {
            mp.txBuf = mp.txFifo.buf[mp.txFifo.tail];
            mp.txState = StartFrame;
            mp.txFifo.tail = (mp.txFifo.tail + 1) % MAX_FIFO_BUF_SIZE;
        }
    }
    else
    {
        ++(mp.txState); //next state

        switch(mp.txState)
        {
        case StartBit:
            Mp_SetTxPort(0);
            break;
        case Bit0:
        case Bit1:
        case Bit2:
        case Bit3:
        case Bit4:
        case Bit5:
        case Bit6:
        case Bit7:
            Mp_SetTxPort(mp.txBuf & 1);
            mp.txBuf >>= 1;
            break;
        case StopBit:
       	case StopBit2:
            Mp_SetTxPort(1);
            mp.txState = Idle;
            break;
        default:
            break;
        }
    }

    if(gTimeOut)
    {
    	--gTimeOut;
    }

    ++gTick;

    if(gTick == 96) //10ms
    {
    	gTick = 0;

    	++gTime;

    	if(gTime == gDelayFlag) //delay
    	{
    		gDelayFlag = 200;
    	}

    	if(gTime == 100) //1 second
    	{
    		gPPS = !gPPS;
    		gTime = 0;
    	}
    }
}
