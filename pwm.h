#ifndef PWM_H
#define PWM_H

#include <xc.h>

// System constants
#define FCY 72000000UL      // Instruction cycle frequency
#define PWM_FREQ 10000      // 10 kHz PWM frequency
#define PTPER_VALUE ((FCY / PWM_FREQ) - 1)

// Robot kinematic parameters
#define MAX_LINEAR_VELOCITY 0.5    // m/s
#define MAX_ANGULAR_VELOCITY 10    // rad/s
#define TRACK_WIDTH 0.108          // meters

// PWM control functions
void OCx_PWM_Init(void);
void Start_PWM(void);
void Stop_PWM(void);
void move_motors(int speed_percentage, int yaw_rate_percentage);


extern volatile double right_speed;
extern volatile double left_speed;

#endif
