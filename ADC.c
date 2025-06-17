/*
 * File:   ADC.c
 * Author: ahmed
 *
 * Created on June 16, 2025, 2:17 AM
 */


#include "xc.h"
#include "ADC.h"
#include <math.h>

void setupADC(int is_auto_sample, int is_auto_convert)
{
    AD1CON3bits.ADCS = 8; // 1 TAD is 8 TCYs
    
    AD1CON1bits.ASAM = 0; // default is manual start
    if(is_auto_sample){
        AD1CON1bits.ASAM = 1; // auto start
    }
    
    AD1CON1bits.SSRC = 0; // default is manual conversion
    if(is_auto_convert)
    {
        AD1CON1bits.SSRC = 7;
        AD1CON3bits.SAMC = 8; // sampling time is 8 TADs
    }
    
    AD1CON2bits.CHPS = 0; // select channel 0
    
    
    
    AD1CON1bits.ADON = 1; // Turn ADC ON    
}
void setScanSeqMode()
{
    AD1CON2bits.CSCNA = 1;
}

void setAnalogPins()
{
    ANSELBbits.ANSB11 = 1;// make AN11 enabled
    ANSELBbits.ANSB15 = 1;// make AN15 enabled
    // set the RA3 pin to output and high for IR enable
    TRISAbits.TRISA3 = 0;
    LATAbits.LATA3 = 1;
    
}

void setScanPorts()
{
    AD1CON2bits.SMPI = 2-1;// for scan: set number of inputs after which done will be set
    AD1CSSLbits.CSS11 = 1; // tell to scan AN11
    AD1CSSLbits.CSS15 = 1; // tell to scan AN15
}

float readIRDist()
{
    float IR_reading = ADC1BUF1;
    IR_reading = (IR_reading*3.3/1023.0);
    IR_reading = 2.34 - 4.74*IR_reading + 4.06*pow(IR_reading,2) - 1.60*pow(IR_reading,3) + 0.24*pow(IR_reading,4);
    return IR_reading;
}
float readVBat()
{
    float vbattery = ADC1BUF0;
    vbattery = 3.0*(vbattery*3.3/1023.0);
    return vbattery;
}