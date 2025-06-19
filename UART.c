
/*
 * File:   uart.c
 * Author: Mahmoud El Asmar, Ahmed Salah, Mahmoud Magdy, Mohamed Ismail
 *
 * Created on April 29, 2025, 8:00 AM
 */

#include "xc.h"         // Include device-specific configuration
#include "UART.h"        // Include UART-related functions
#include "parser.h"
// Define buffer size for UART messages

#define TX_UART_BUF_SIZE 128
#define RX_UART_BUF_SIZE 64

parser_state pstate;

char tx_msg_buffer[TX_UART_BUF_SIZE];
char rx_msg_buffer[RX_UART_BUF_SIZE];
int tx_insert_idx = 0;
int tx_consume_idx = 0;
int rx_insert_idx = 0;
int rx_consume_idx = 0;



void setup_uart1(){
    
    TRISDbits.TRISD11 = 1; // Set RD11 as input (RX pin)
    RPINR18bits.U1RXR = 75; // map uart1 RX
    TRISDbits.TRISD0 = 0;  // Set RD0 as output (TX pin)
    RPOR0bits.RP64R = 1; // map uart1 TX
    
    U1BRG = 468;// (TIMER_CLOCK_FREQ_HZ/2)/(16*9600)-1; // (TIMER_CLOCK_FREQ_HZ / 2) / (16 ? 9600)? 1
    U1MODEbits.BRGH = 0;        // Low speed mode (standard)
    U1MODEbits.PDSEL = 0b00;    // 8-bit data, no parity
    U1MODEbits.STSEL = 0;       // 1 stop bit
    U1MODEbits.UARTEN = 1; // enable UART
    U1STAbits.UTXEN = 1; // enable U1TX (must be after UARTEN)
    
    IFS0bits.U1RXIF = 0; // Clear the UART RX interrupt flag
    IEC0bits.U1RXIE = 1;   // Enable RX interrupt
    IPC2bits.U1RXIP = 1;  // Set RX interrupt priority to 1
    
    pstate.state = STATE_DOLLAR;
	pstate.index_type = 0; 
	pstate.index_payload = 0;

}

 void sendMessage(char* msg)
 {
     IEC0bits.U1TXIE = 0;
    for (int i=0;i<256;i++){
        
        tx_msg_buffer[tx_insert_idx] = msg[i];
        tx_insert_idx++;
        tx_insert_idx %= TX_UART_BUF_SIZE;
        if (msg[i] == '\0')
            break;  // Stop copying once the null terminator is encountered
    }
     IEC0bits.U1TXIE = 1;
 }      
 
 int getNextMsg(char*msgPayload,char*msgType)
 {
     // check if the buffer is empty
     if(rx_insert_idx == rx_consume_idx){
         return 0;
     }
     // loop over the bytes and parse
     for (int i=0;i<RX_UART_BUF_SIZE;i++)
     {
         IEC0bits.U1RXIE = 0;
         // parse the byte
         int is_new_msg = parse_byte(&pstate,rx_msg_buffer[rx_consume_idx]);
         // advance the index
         rx_consume_idx++;
         rx_consume_idx %= RX_UART_BUF_SIZE;
         IEC0bits.U1RXIE = 1;
         if(is_new_msg==1)
         {
             // return the  message
             for(int i=0;i<100;i++){
                *(msgPayload+i) = pstate.msg_payload[i];
                if(pstate.msg_payload[i]=='\0')
                    break;
             }
             
             for(int i=0;i<6;i++){
                *(msgType+i) = pstate.msg_type[i];
             }
             *(msgType+5) = '\0';

             return 1;
         }
         // if the buffer is empty return
         if(rx_insert_idx == rx_consume_idx){
            return 0; 
         }
     }
     return 0;
 }
 
 void sendChar()
 {
     // if the buffer is empty return  
    if(tx_insert_idx == tx_consume_idx)
    {
        IEC0bits.U1TXIE = 0;
        return; 
    }
    
    IEC0bits.U1TXIE = 0;
    char temp;
    while(!U1STAbits.UTXBF && tx_insert_idx != tx_consume_idx){
        temp = tx_msg_buffer[tx_consume_idx];
        tx_consume_idx++;
        tx_consume_idx%=TX_UART_BUF_SIZE;

        if(temp=='\0')
            break;
        U1TXREG = temp;
    }
    IEC0bits.U1TXIE = 1;
    
 }
 void triggerSend()
 {
     sendChar();
 }
 
 
 void __attribute__((__interrupt__, __auto_psv__)) _U1RXInterrupt(){ 
	IFS0bits.U1RXIF = 0;  // Clear the RX interrupt flag
    if (U1STAbits.OERR) {
        U1STAbits.OERR = 0; // Clear overrun error
    }
    while(U1STAbits.URXDA){
        char received = U1RXREG;  // Read the received character
        rx_msg_buffer[rx_insert_idx] = received;
        rx_insert_idx++;
        rx_insert_idx%=RX_UART_BUF_SIZE;
    }
 }
 
 void __attribute__((__interrupt__, __auto_psv__)) _U1TXInterrupt(){ 
     
     IFS0bits.U1TXIF = 0;  // Clear the TX interrupt flag
     sendChar();
 }