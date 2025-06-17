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
    
    while(1){
        
        scheduler(schedInfo,n_tasks);
        tmr_wait_period(TIMER1);
    
    }
    return 0;
}
