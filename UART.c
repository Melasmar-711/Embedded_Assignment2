/*
 * File:   UART.c
 * Author: ahmed
 *
 * Created on June 16, 2025, 7:56 PM
 */


#include "xc.h"
#include "UART.h"
void setup_uart1(){
    
    TRISDbits.TRISD11 = 1;
    RPINR18bits.U1RXR = 75; // map uart1 RX
    TRISDbits.TRISD0 = 0;
    RPOR0bits.RP64R = 1; // map uart1 TX
    
    U1BRG = 468;// (TIMER_CLOCK_FREQ_HZ/2)/(16*9600)-1; // (TIMER_CLOCK_FREQ_HZ / 2) / (16 ? 9600)? 1
    U1MODEbits.BRGH = 0;        // Low speed mode (standard)
    U1MODEbits.PDSEL = 0b00;    // 8-bit data, no parity
    U1MODEbits.STSEL = 0;       // 1 stop bit
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable U1TX (must be after UARTEN)
    
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1;
    IPC2bits.U1RXIP = 1;
}
void send_string_uart1(char* msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        char ch = msg[i];

        // Do something with each character
        while (U1STAbits.UTXBF);  // Wait if UART TX is busy
        U1TXREG = ch;
    }
}
