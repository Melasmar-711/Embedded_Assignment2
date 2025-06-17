/*
 * File:   timer.c
 * Author: stark
 *
 * Created on March 2, 2025, 6:47 AM
 */

#include "xc.h"
#include "timer.h"
#include "stdint.h"
#include <math.h>

// Global flag to indicate timer expiration
volatile uint8_t timer1_expired = 0;
volatile uint8_t timer2_expired = 0;
volatile uint8_t base_ms1 = 0;
volatile uint8_t base_ms2 = 0;

volatile uint16_t toggleStateButtonCounter = 0;
volatile int toggleStateButtonCanBePressed = 1;

void tmr_setup_period(int timer, int ms) {
    if (ms <= 0 || ms > 200) {
        // Handle invalid input
        return;
    }
    

    uint16_t count = CALCULATE_COUNT_VAL(ms);

    if (timer == TIMER1) {
        base_ms1 = ms;
        T1CONbits.TON = 0; // Disable Timer1
        TMR1 = 0;          // Reset Timer1 counter
        T1CONbits.TCKPS = TIMER_PRESCALER_256; // Set prescaler to 1:256
        PR1 = count;       // Set period register
        IFS0bits.T1IF = 0; // Clear Timer1 interrupt flag
        IEC0bits.T1IE = 1; // Enable Timer1 interrupt
        T1CONbits.TON = 1; // Enable Timer1
    } else if (timer == TIMER2) {
        base_ms2 = ms;
        T2CONbits.TON = 0; // Disable Timer2
        TMR2 = 0;          // Reset Timer2 counter
        T2CONbits.TCKPS = TIMER_PRESCALER_256; // Set prescaler to 1:256
        PR2 = count;       // Set period register
        IFS0bits.T2IF = 0; // Clear Timer2 interrupt flag
        IEC0bits.T2IE = 1; // Enable Timer2 interrupt
        T2CONbits.TON = 1; // Enable Timer2
    } else {
        // Handle invalid timer input
        return;
    }
}


// Timer1 Interrupt Service Routine (ISR)
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; // Clear Timer1 interrupt flag
    timer1_expired = 1; // Set global flag
}

// Timer2 Interrupt Service Routine (ISR)
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0; // Clear Timer2 interrupt flag
    timer2_expired = 1; // Set global flag
//    timer2_counter ++;
//    if(timer2_counter >= 500){
//        LATAbits.LATA0 = !LATAbits.LATA0;
//        timer2_counter = 0;
//        U1TXREG = 'G';
//    }
    if(!toggleStateButtonCanBePressed)
    {
        toggleStateButtonCounter++;
        if(toggleStateButtonCounter >= 500)
        {
            toggleStateButtonCanBePressed= 1;
            toggleStateButtonCounter = 0;
        }
    }
}

int tmr_wait_period(int timer) {
    
    if (timer == TIMER1) {
        if(IFS0bits.T1IF == 1)
            return 1;
        while (timer1_expired == 0) {
               asm("pwrsav #1"); 
        }
        timer1_expired = 0;
    } else if (timer == TIMER2) {
        if(IFS0bits.T2IF == 1)
            return 1;
        while (timer2_expired == 0) {
            asm("pwrsav #1"); 
        }
        timer2_expired = 0;
    } else {
        // Handle invalid timer input
        return 1; 
    }
    return 0;
    
}

void tmr_wait_ms(int timer, int ms)
{
    uint16_t steps = 0;
    if (timer == TIMER1)
        steps = floor(ms/base_ms1);
    else
        steps = floor(ms/base_ms2);
        
    for(uint16_t i=0;i<steps;i++){
        tmr_wait_period(timer);
    }
}