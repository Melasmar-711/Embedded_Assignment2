/*
 * File:   scheduler.c
 * Author: mahmo
 *
 * Created on June 17, 2025, 3:18 PM
 */

#include "scheduler.h"





void scheduler(heartbeat schedInfo[], int nTasks) {
  int i;
  for (i = 0; i < nTasks; i++) {
    schedInfo[i].n++;
    if (schedInfo[i].enable == 1 &&schedInfo[i].n >= schedInfo[i].N) {
      schedInfo[i].f(schedInfo[i].params); 
      schedInfo[i].n = 0;
    }
  }
}
