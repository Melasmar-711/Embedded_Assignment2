/*
 * File:   tasks.c
 * Author: ahmed
 *
 * Created on June 16, 2025, 7:29 PM
 */

#include "tasks.h"
#include "spi.h"
#include "ADC.h"
#include "UART.h"
#include "parser.h"
#include "timer.h"
#include "pwm.h"
#include <string.h>


volatile int16_t accel_buffer[ACCEL_BUF_SIZE][3];  // A buffer to hold X, Y, Z accel field samples
volatile uint8_t accel_buf_head = 0; 
volatile float avg_x, avg_y, avg_z;   // Variables to hold the average X, Y, Z values of accel data
volatile float IR_reading;
volatile double vbattery;
int emergencyEndCounter = 0;
volatile STATE carState = WAIT;  // Define and optionally initialize the variable


void configureBoard(){
    carState = WAIT;
    tmr_setup_period(TIMER1,2);
    tmr_setup_period(TIMER2,1);
    OCx_PWM_Init();
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
    
    TRISFbits.TRISF1 = 0;
    LATFbits.LATF1 = 0;
    // Toggle State Button 
    TRISEbits.TRISE8 = 1;
    
    
    TRISEbits.TRISE8 = 1;
    RPINR0bits.INT1R=0b1011000;
    INTCON2bits.INT1EP=1;
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;
    
    
}



void calculate_average(){
    // Temporary variables to store the sum of X, Y, Z values
    int16_t sum_x = 0, sum_y = 0, sum_z = 0;
    // Loop over the buffer to sum the X, Y, Z values

    for (uint8_t i = 0; i < ACCEL_BUF_SIZE; i++) {
        sum_x += accel_buffer[i][0];
        sum_y += accel_buffer[i][1];
        sum_z += accel_buffer[i][2];
    }
    avg_x = sum_x / ACCEL_BUF_SIZE;
    avg_y = sum_y / ACCEL_BUF_SIZE;
    avg_z = sum_z / ACCEL_BUF_SIZE;
}

void toggleState()
{
    if(carState == EMERGENCY){
        return;
    }
    else if(carState == WAIT)
    {
        carState = MOVING;
    }else if(carState == MOVING)
    {
        carState = WAIT;
    }
}

void handleEmergencyState()
{
    if(carState != EMERGENCY && IR_reading <= DIST_THRESHOLD){
        // Emergency situation detected
        emergencyEndCounter = 0;
        carState = EMERGENCY;
        sendMessage("$MEMRG,1*");
    }else if(carState == EMERGENCY && IR_reading > DIST_THRESHOLD)
    {
        // Cool down after Emergency
        emergencyEndCounter++;
        if(emergencyEndCounter > 2500){ // task is every 500 HZ, multiplied by 5 seconds
            carState = WAIT;
            LATFbits.LATF1 = 0;
            LATBbits.LATB8 = 0;
            sendMessage("$MEMRG,0*");
        }
    }else if(carState == EMERGENCY && IR_reading <= DIST_THRESHOLD)
    {
        // Emergency state continuing
        emergencyEndCounter = 0;
    }
}

void readADCData()
{
    if (!AD1CON1bits.DONE)
        return;
    
    AD1CON1bits.DONE = 0;
    
    vbattery = readVBat();
    IR_reading = readIRDist();
    handleEmergencyState();
}

void readAccelerometer()
{
    accel_read_xyz(&accel_buffer[accel_buf_head][0], &accel_buffer[accel_buf_head][1], &accel_buffer[accel_buf_head][2]);
    accel_buf_head ++; // Increment the buffer head to the next slot
    accel_buf_head %= ACCEL_BUF_SIZE;  // Wrap the buffer head back to 0 after reaching buffer size
    calculate_average(); // Compute the average of the last 5 accelerometer samples
}

void toggleLEDs()
{
    LATAbits.LATA0 = !LATAbits.LATA0;
    if(carState == EMERGENCY)
    {
        // blink left and right-side lights
        LATFbits.LATF1 = !LATFbits.LATF1;
        LATBbits.LATB8 = !LATBbits.LATB8;

    }
}

void setDutyCycles(){
        // Left motor PWM assignment
    if((carState == WAIT) || (carState == EMERGENCY)){
        Pause_PWM();
        return;
    }
    if(left_speed >0){    
        OC1R = 0;                                                    // setting the duty cycle to 0 for backward motion in case it had been set before
        OC2R = PTPER_VALUE*(left_speed/MAX_LINEAR_VELOCITY);    // calculating duty cycle based on the speed percentage
    }
    else
    {
        OC2R = 0;                                                   // setting the duty cycle to 0 for forward motion in case it had been set before
        OC1R = PTPER_VALUE*(-1*left_speed/MAX_LINEAR_VELOCITY); // calculating duty cycle based on the speed percentage
    }

    // Right motor PWM assignment
    if(right_speed >0){
        OC3R = 0;                                                   // setting the duty cycle to 0 for backward motion in case it had been set before
        OC4R = PTPER_VALUE*(right_speed/MAX_LINEAR_VELOCITY); // calculating duty cycle based on the speed percentage
    }
    else
    {
        OC4R = 0;                                                       // setting the duty cycle to 0 for forward motion in case it had been set before
        OC3R = PTPER_VALUE*(-1*right_speed/MAX_LINEAR_VELOCITY); // calculating duty cycle based on the speed percentage
    }
}



void __attribute__((interrupt, auto_psv)) _INT1Interrupt(void){
    
    IEC1bits.INT1IE = 0;
    IFS1bits.INT1IF = 0;
    toggleState();    
    IEC1bits.INT1IE = 1;
}
void handleStateSwitch(STATE state)
{
//    if(carState == WAIT)
//        U1TXREG = 'G';
//    else if(carState == MOVING)
//        U1TXREG = 'H';
    if(carState == EMERGENCY)
    {
        sendMessage(CHANGE_STATE_ACK0);
    }else
    {
        carState = state;
        sendMessage(CHANGE_STATE_ACK1);
    }
        
}

void handleUserMsg(char*msgPayload,char* msgType)
{
    if(memcmp(msgType,"PCREF",5)==0)
    {
        sendMessage("PCREF");
        // speed and yaw rate message
        int speed = extract_integer(msgPayload);
        int idx = next_value(msgPayload,0);
        int yaw_rate = extract_integer(msgPayload+idx);

        char speedStr[12];  // enough for a 32-bit int, including sign and null terminator
        sprintf(speedStr, "%d", speed);
        sendMessage(speedStr);

        char yawStr[12];  // enough for a 32-bit int, including sign and null terminator
        sprintf(yawStr, "%d", yaw_rate);
        sendMessage(yawStr);


        move_motors(speed,yaw_rate);

    }else if(memcmp(msgType,"PCSTP",5)==0)
    {
        // Switch to wait
        sendMessage("PCSTP");
        handleStateSwitch(WAIT);
    }else if(memcmp(msgType,"PCSTT",5)==0)
    {
        // Switch to moving
        handleStateSwitch(MOVING);
        sendMessage("PCSTT");
    }else
    {
        sendMessage(msgType);
//        U1TXREG = 'F';
    }
    
}

void handleUserMsgs()
{
    int msg_received = 1;
    while(msg_received==1){
        char msgType[6];
        char msgPayload[100];
        // process all user messages
        msg_received = getNextMsg(msgPayload,msgType);
        if(msg_received==1){
//            U1TXREG = 'E';
//            sendMessage(msgType);
//            sendMessage(msgPayload);
            handleUserMsg(msgPayload,msgType);
            
            
        }
    }
}

void logBattery()
{
    char msg[16];
    sprintf(msg, "$MBATT,%.2f*", vbattery);
    sendMessage(msg);
}



void logIrAndAccel()
{
    char msg[16];
    sprintf(msg, "$MDIST,%d*", (int16_t)(IR_reading*100));
    sendMessage(msg);
    sprintf(msg, "$MACC,%d,%d,%d*", (int16_t)avg_x, (int16_t)avg_y, (int16_t)avg_z);
    sendMessage(msg);
    triggerSend();
}
