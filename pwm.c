#include "pwm.h"



volatile double right_speed;
volatile double left_speed;

void OCx_PWM_Init(void) {
    // Configure OC1 (left motor) using peripheral clock
    OC1CON1bits.OCTSEL = 0b111; // Select peripheral clock as OC1 clock source (system clock)
    OC1CON1bits.OCM = 0b110;   // Set OC1 to edge-aligned PWM mode
    OC1R = 0;                  // Initialize OC1 primary register to 0% duty cycle
    OC1RS = PTPER_VALUE;       // Set OC1 secondary register to period value for 10kHz

    // Configure OC2 (right motor) using peripheral clock  
    OC2CON1bits.OCTSEL = 0b111; // Select peripheral clock as OC2 clock source (system clock)
    OC2CON1bits.OCM = 0b110;   // Set OC2 to edge-aligned PWM mode
    OC2R = 0;                  // Initialize OC2 primary register to 0% duty cycle
    OC2RS = PTPER_VALUE;       // Set OC2 secondary register to period value for 10kHz
    
    OC3CON1bits.OCTSEL = 0b111; // Select peripheral clock as OC1 clock source (system clock)
    OC3CON1bits.OCM = 0b110;   // Set OC1 to edge-aligned PWM mode
    OC3R = 0;                  // Initialize OC1 primary register to 0% duty cycle
    OC3RS = PTPER_VALUE;       // Set OC1 secondary register to period value for 10kHz

    // Configure OC2 (right motor) using peripheral clock  
    OC4CON1bits.OCTSEL = 0b111; // Select peripheral clock as OC2 clock source (system clock)
    OC4CON1bits.OCM = 0b110;   // Set OC2 to edge-aligned PWM mode
    OC4R = 0;                  // Initialize OC2 primary register to 0% duty cycle
    OC4RS = PTPER_VALUE;       // Set OC2 secondary register to period value for 10kHz

    // Configure synchronization for both OC modules
    OC1CON2bits.SYNCSEL = 0x1F; // Use internal sync signal (0b11111) for OC1
    OC2CON2bits.SYNCSEL = 0x1F; // Use internal sync signal (0b11111) for OC2
    OC3CON2bits.SYNCSEL = 0x1F; // Use internal sync signal (0b11111) for OC4
    OC4CON2bits.SYNCSEL = 0x1F; // Use internal sync signal (0b11111) for OC5
    
    
    RPOR0bits.RP65R = 16; // map RD1 to OC1 left
    RPOR1bits.RP66R = 17; // map RD2 to OC2 left
    RPOR1bits.RP67R = 18; // map RD3 to OC3 right
    RPOR2bits.RP68R = 19; // map RD4 to OC4 right
}

void move_motors(int speed_percentage, int yaw_rate_percentage) {
    double desired_speed = MAX_LINEAR_VELOCITY * speed_percentage / 100;
    double desired_angular_vel = MAX_ANGULAR_VELOCITY * yaw_rate_percentage / 100;

    /*
    v = (v_l+v_r)/2   (0)  ----> desired_speed is v
     w = (v_r -v_l)/L      where L is track width  desired_angular_vel is w


    2v = vr +vl  (1)    vl is left_side_speed and vr is rigth_side_speed
     w*L =vr-vl   (2)    L is TRACK_WIDTH
     adding (1)&(2)  gives us:
     2v+(w*l) =2vr  ----      vr=(2v+(w*l))/2         (3)
    */

    // Differential drive calculations
    right_speed = ((2 * desired_speed) + (desired_angular_vel * TRACK_WIDTH)) / 2;  // equation (3)
    left_speed = (2 * desired_speed) - right_speed;          //substituting vr in (0)
    
    // Speed clamping in case the speed and yaw_rate percentages were more that 100% and that resulted in left or rigth mortor speed exceeding the maximum linear velocity
    right_speed = (right_speed > MAX_LINEAR_VELOCITY) ? MAX_LINEAR_VELOCITY : (right_speed < -MAX_LINEAR_VELOCITY) ? -MAX_LINEAR_VELOCITY : right_speed;
                 
    left_speed = (left_speed > MAX_LINEAR_VELOCITY) ? MAX_LINEAR_VELOCITY : (left_speed < -MAX_LINEAR_VELOCITY) ? -MAX_LINEAR_VELOCITY : left_speed;

}

// Enable PWM output by activating Output Compare modules
void Start_PWM(void) {
    OC4CON1bits.OCM = 0b110; // Enable OC1 in edge-aligned PWM mode
    OC2CON1bits.OCM = 0b110; // Enable OC2 in edge-aligned PWM mode
    OC3CON1bits.OCM = 0b110; // Enable OC1 in edge-aligned PWM mode
    OC1CON1bits.OCM = 0b110; // Enable OC1 in edge-aligned PWM mode
}

// Disable PWM output and reset duty cycles to zero
void Stop_PWM(void) {
    OC4CON1bits.OCM = 0b000; // Disable OC1 output (no compare mode)
    OC2CON1bits.OCM = 0b000; // Disable OC2 output (no compare mode)
    OC3CON1bits.OCM = 0b000; // Disable OC1 output (no compare mode)
    OC1CON1bits.OCM = 0b000; // Disable OC1 output (no compare mode)
    
    // Reset duty cycles to 0% (motor stopped)
    OC4R = 0;               // Set OC1 duty cycle to 0% (motor stopped)
    OC2R = 0;               // Set OC2 duty cycle to 0% (motor stopped)
    OC3R = 0;               // Set OC3 duty cycle to 0% (motor stopped)
    OC1R = 0;               // Set OC1 duty cycle to 0% (motor stopped)
}
