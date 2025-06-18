/*
 * File:   main.c
 * Author: ahmed
 *
 * Created on June 16, 2025, 7:28 PM
 */


#include "xc.h"
#include "tasks.h"
#include "timer.h"
#include "scheduler.h"

#define NULL ((void*)0)
heartbeat schedInfo[MAX_TASKS];


int main(void) {
    
    configureBoard();
    
    
    // PWM Update task
    schedInfo[0].n = 0;
    schedInfo[0].N = 1;
    schedInfo[0].f = setDutyCycles;
    schedInfo[0].params = NULL;
    schedInfo[0].enable = 1;
    
    // IR reading task
    schedInfo[1].n = 0;
    schedInfo[1].N = 1;
    schedInfo[1].f = readADCData;
    schedInfo[1].params = NULL;
    schedInfo[1].enable = 1;
    
    // Toggling Led task
    schedInfo[2].n = 0;
    schedInfo[2].N = 500;
    schedInfo[2].f = toggleLEDs;
    schedInfo[2].params = NULL;
    schedInfo[2].enable = 1; 
    
    //  task
    schedInfo[3].n = 0;
    schedInfo[3].N = 50;
    schedInfo[3].f = readAccelerometer;
    schedInfo[3].params = NULL;
    schedInfo[3].enable = 1; 
    
    //  task
    schedInfo[4].n = 0;
    schedInfo[4].N = 50;
    schedInfo[4].f = handleUserMsgs;
    schedInfo[4].params = NULL;
    schedInfo[4].enable = 1; 
    
    //  task
    schedInfo[5].n = 0;
    schedInfo[5].N = 500;
    schedInfo[5].f = logBattery;
    schedInfo[5].params = NULL;
    schedInfo[5].enable = 1; 
    
    schedInfo[6].n = 0;
    schedInfo[6].N = 50;
    schedInfo[6].f = logIrAndAccel;
    schedInfo[6].params = NULL;
    schedInfo[6].enable = 1; 
    

    TRISGbits.TRISG9 = 0;
    LATGbits.LATG9 = 0;
    while(1){
        scheduler(schedInfo,n_tasks);
        int expired = tmr_wait_period(TIMER1);
        if(expired == 1)
            LATGbits.LATG9 = 1;
    }
    return 0;
}
