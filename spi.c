/*
 * File:   spi.c
 * Author: Ahmed Salah, Mahmoud El Asmar, Mahmoud Magdy, Mohamed Ismail
 *
 * Created on June 17, 2025, 3:18 PM
 */
#include "spi.h"
#include "timer.h"

void setup_spi()
{
    TRISAbits.TRISA1 = 1;           // MISO (RPI17)
    TRISFbits.TRISF13 = 0;          // MOSI (RP109)
    TRISFbits.TRISF12 = 0;          // SCK  (RP108)
    
    // Set CS1 as output
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB4 = 0;
    TRISDbits.TRISD6 = 0;

    // disable Chip Select
    LATBbits.LATB3 = 1;             
    LATBbits.LATB4 = 1;
    LATDbits.LATD6 = 1;
    
    // === Configure SPI1 ===
    SPI1STATbits.SPIEN = 0;         // Disable SPI before config
    SPI1CON1bits.MSTEN = 1;         // Master mode
    SPI1CON1bits.MODE16 = 0;        // 8-bit mode
    SPI1CON1bits.CKP = 1;           // Clock idle low (CPOL = 0)
    SPI1CON1bits.CKE = 0;           // Data changes on transition from active to idle (CPHA = 0)
    SPI1CON1bits.SPRE = 0b101;      // Secondary prescaler = 3:1
    SPI1CON1bits.PPRE = 0b10;       // Primary prescaler = 4:1
    SPI1STATbits.SPIEN = 1;         // Enable SPI
    
    // === Remap SPI1 Pins ===
    RPINR20bits.SDI1R = 0x11;  // SDI1 (MISO) <- RPI17 (RA1)
    RPOR12bits.RP109R = 0x05;   // SDO1 (MOSI) -> RP109 (RF13)
    RPOR11bits.RP108R = 0x06;   // SCK1 -> RP108 (RF12)
    
    
    // Step 1: Sleep mode
    LATBbits.LATB3 = 0;
    tmr_wait_ms(TIMER2, 3);
    spi_write(0x4B & 0x7F); // 
    spi_write(0x01);  // Sleep
    LATBbits.LATB3 = 1; // Set CS3 HIGH (deselect magnetometer)
    tmr_wait_ms(TIMER2, 3); // Wait for 3ms

    // Step 3: Setting Output Data Rate to 25Hz
    LATBbits.LATB3 = 0;
    tmr_wait_ms(TIMER2, 3);
    spi_write(0x4C & 0x7F); // ODR register
    spi_write(0x00);        // 25Hz
    LATBbits.LATB3 = 1;
    tmr_wait_ms(TIMER2, 3);
}

unsigned int spi_write(unsigned int data) {
    while (SPI1STATbits.SPITBF == 1);  // Wait until transmit buffer is empty
    SPI1BUF = data;                    // Send data
    while (SPI1STATbits.SPIRBF == 0); // Wait for receive buffer full
    return SPI1BUF;                    // Return received data
}


unsigned char spi_read_register(unsigned char reg) {
    unsigned char val;

    LATBbits.LATB3 = 0;  // CS LOW
    spi_write(reg | 0x80); // Set MSB for READ
    val = spi_write(0x00); // Dummy write to receive data
    LATBbits.LATB3 = 1;  // Set CS HIGH 
    return val;// Return the data read from the register
}
// Function to read the X, Y, and Z axis values from the magnetometer

void accel_read_xyz( volatile int16_t* x,volatile int16_t* y,volatile int16_t* z) {
    unsigned char x_lsb = spi_read_register(0x02); // Low byte of X data
    unsigned char x_msb = spi_read_register(0x03); // High byte of X data
    x_lsb &= 0x00F8;
    *x = (int16_t)((((x_msb << 8) | x_lsb))>>3)/2;  // Combine the MSB and LSB to form the 16-bit value

    // Read the Y axis data (2 bytes)

    unsigned char y_lsb = spi_read_register(0x04); // Low byte of Y data
    unsigned char y_msb = spi_read_register(0x05); // High byte of Y data
    y_lsb &= 0x00F8;
    *y = (int16_t)((((y_msb << 8) | y_lsb))>>3)/2;  // Combine the MSB and LSB to form the 16-bit value


    unsigned char z_lsb = spi_read_register(0x06);  // Low byte of Z data
    unsigned char z_msb = spi_read_register(0x07);// High byte of Z data
    z_lsb &= 0x00F8;
    *z = (int16_t)((((z_msb << 8) | z_lsb))>>3)/2;  // Combine the MSB and LSB to form the 16-bit value
}