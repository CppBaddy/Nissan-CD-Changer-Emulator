/*
 * main.c
 *
 * Copyright (c) 2018 Yulay Rakhmangulov.
 *
 * Schematics and PCB design can be found here:
 *       https://easyeda.com/Yulay/nissan-cd-changer-emulator
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

#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>

#ifndef F_CPU
    #define F_CPU   8000000
#endif

#include <avr/io.h>
#include <avr/iotn85.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//#include <avr/eeprom.h>

#include "PortConfig.h"
#include "Uart.h"
#include "MpUart.h"
#include "HuUart.h"
#include "CdControl.h"
#include "PlayerControl.h"

void setup();
void WDT_Init();


int main( void )
{
    setup(); //setup ports

    Player_LoadState();

    for(;;) //loop scheduler
    {
    	Player_Update();

    	HeadUnit_Update();
    }

    return 0;
}


inline void setup()
{
    //setup clock frequency
    CLKPR = _BV(CLKPCE); //enable Clock Prescale Register write
    CLKPR = 0;           //change prescaler to 1, effectively set 8MHz system clock

    //setup input/output ports
    //PB0 - HuRxPORT input
    //PB1 - /HuTxPORT output
    //PB2 - /HuReqPORT output
    //PB3 - MpRxPORT input
    //PB4 - MpTxPORT output
    //PB5 - Reset (can be reused as separate pin later)
    DDRB |= _BV(HuTxPORT) | _BV(HuReqPORT) | _BV(MpTxPORT); /* set PBx to output */
    PORTB |= _BV(MpTxPORT);

    PCMSK = _BV(HuRxPORT) | _BV(MpRxPORT); //enable mask for RX inputs
    GIFR  = _BV(PCIF);               //clear PortC pin change interrupt flag
    GIMSK = _BV(PCIE);               //enable PortC pin change interrupt


    //Timer0 UART clock 9600 baud
    //GTCCR  |= _BV(TSM) | _BV(PSR0); //stop timer0 and clear prescaler
    TCCR0A |= _BV(WGM01);           //CTC mode - clear on compare match 0A
    TCCR0B  = _BV(CS01);            //prescaler 8
    //TCNT0   = 0;
    OCR0A   = TIMER0_RELOAD;        //reload value for selected UART baud rate
    //OCR0B   = TIMER0_RELOAD/2;      //read out in the middle of the bit
    //TIMSK  |= _BV(OCIE0B);          //enable CompareMatch0B interrupt

    //Timer1 UART clock 9600 baud
    //GTCCR  |= _BV(PSR1);           //clear prescaler
    TCCR1 = _BV(CTC1) | _BV(CS12);   //clear Timer1 on CompareMatch1C; prescaler 8
    //TCNT1 = 0;
    //OCR1A = TIMER1_RELOAD/2;         //read out in the middle of the bit
    OCR1B = 0;                       //transmit out on COMPB
    OCR1C = TIMER1_RELOAD;
    //TIMSK |= _BV(OCIE1A);          //enable interrupt for CompareMatch1A

    Mp_Init();
    Hu_Init();

    sei(); //Enable interrupts
}

void WDT_Init()
{
    cli();

    wdt_reset();

    // Use Timed Sequence for disabling Watchdog System Reset Mode if it has been enabled unintentionally.
    MCUSR &= ~_BV(WDRF);          // Clear WDRF if it has been unintentionally set.

    WDTCR = _BV(WDCE) | _BV(WDE); // Enable configuration change.
    WDTCR = _BV(WDIF) | _BV(WDIE) // Enable Watchdog Interrupt Mode.
          | _BV(WDCE) | _BV(WDE)  // Disable Watchdog System Reset Mode if unintentionally enabled.
          | _BV(WDP0);            // Set Watchdog Timeout period to 32 ms.
    	  //; 			          // Set Watchdog Timeout period to 16 ms.

    sei();
}

/* External input interrupt handler */
//ISR( INT0_vect )
//{
//}

/* Pin change interrupt handler */
ISR( PCINT0_vect )
{
    if(PCMSK & _BV(HuRxPORT)) //if HU input enabled
    {
        if(hu.rxState == Idle && ((PINB & _BV(HuRxPORT)) == 0)) //start frame detected
        {
            Hu_RxStart();
        }
    }

    if(PCMSK & _BV(MpRxPORT)) //if MP input enabled
    {
        if(mp.rxState == Idle && ((PINB & _BV(MpRxPORT)) == 0)) //start frame detected
        {
            Mp_RxStart();
        }
    }
}

/* Timer1 interrupt handler */
//ISR( TIMER1_COMPA_vect )
//implemented in MpUart.c

/* Timer1 interrupt handler */
//ISR( TIMER1_OVF_vect )
//{}

/* Timer0 interrupt handler */
//ISR( TIMER0_OVF_vect )
//{}

/* EEPROM Ready interrupt handler */
//ISR( EE_RDY_vect )
//{} implemented in EEPROM.c

/* Analog comparator interrupt handler */
//ISR( ANA_COMP_vect )
//{}

/* ADC interrupt service routine */
//ISR( ADC_vect )
//{}

/* Timer1 interrupt handler */
//ISR( TIMER1_COMPB_vect )
//implemented in MpUart.c

/* Timer0 compare match A interrupt handler */
//ISR( TIMER0_COMPA_vect )
//implemented in HuUart.c

/* Timer0 compare match B interrupt handler */
//ISR( TIMER0_COMPB_vect )
//implemented in HuUart.c


/* Watchdog interrupt handler */
//ticks each 16ms
//ISR( WDT_vect )
//{
//    ++gTime;
//}

/* USI_START */
//ISR( USI_START_vect )
//{}

/* USI Overflow */
//ISR( USI_OVF_vect )
//{}
