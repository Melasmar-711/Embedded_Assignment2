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


heartbeat schedInfo[MAX_TASKS];


int main(void) {
    
    configureBoard();
    void *ptr;
    
    // PWM Update task
    schedInfo[0].n = 0;
    schedInfo[0].N = 1;
    schedInfo[0].f = setDutyCycles;
    schedInfo[0].params = ptr;
    schedInfo[0].enable = 1;
    
    // IR reading task
    schedInfo[1].n = 0;
    schedInfo[1].N = 1;
    schedInfo[1].f = readADCData;
    schedInfo[1].params = ptr;
    schedInfo[1].enable = 1;
    
    // Toggling Led task
    schedInfo[2].n = 0;
    schedInfo[2].N = 500;
    schedInfo[2].f = toggleLEDs;
    schedInfo[2].params = ptr;
    schedInfo[2].enable = 1; 
    
    //  task
    schedInfo[3].n = 0;
    schedInfo[3].N = 50;
    schedInfo[3].f = readAccelerometer;
    schedInfo[3].params = ptr;
    schedInfo[3].enable = 1; 
    
    
    while(1){
        
        //scheduler(schedInfo,n_tasks);
        tmr_wait_period(TIMER1);
    
    }
    return 0;
}
