/*
 * File:   util.c
 * Author: ahmed
 *
 * Created on June 17, 2025, 3:52 AM
 */


#include "xc.h"
#include"timer.h"

void configureBoard(){
    carState = Emergency;
    tmr_setup_period(TIMER1,10);
    tmr_setup_period(TIMER2,1);
    setupADC(1,1);
    setScanSeqMode();
    setup_spi();
    setup_uart1();
    // Set bins
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000;
    setAnalogPins();
    setScanPorts();
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 0;
    
    TRISBbits.TRISB8 = 0;
    LATBbits.LATB8 = 0;
    
    TRISBbits.TRISB2 = 0;
    LATBbits.LATB2 = 0;
}
