// motor.c
#include "motor.h" // Include the header file
#include "RTE_Components.h" // Still needed for some definitions potentially
#include "MKL25Z4.h" //Devide header file
#include "cmsis_os2.h"
#include <stdbool.h>

#define MASK(x) (1 << (x))

// Initialize motor pins
void init_Motor() {
	//if the speed is too much we probably can use a timer(PWM) to vary duty cycledx
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

	MOTOR_PORT->PCR[LEFTENGINE_in] &= ~PORT_PCR_MUX_MASK;
  MOTOR_PORT->PCR[LEFTENGINE_in] |= PORT_PCR_MUX(1);
  MOTOR_PORT->PCR[LEFTENGINE__out] &= ~PORT_PCR_MUX_MASK;
  MOTOR_PORT->PCR[LEFTENGINE__out] |= PORT_PCR_MUX(1);
	
	MOTOR_PORT->PCR[RIGHTENGINE_in] &= ~PORT_PCR_MUX_MASK;
  MOTOR_PORT->PCR[RIGHTENGINE_in] |= PORT_PCR_MUX(1);
  MOTOR_PORT->PCR[RIGHTENGINE__out] &= ~PORT_PCR_MUX_MASK;
  MOTOR_PORT->PCR[RIGHTENGINE__out] |= PORT_PCR_MUX(1);
  
	PTC->PDDR |= MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PDDR |= MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	
	//turn off motors initially
	stopMotors();
}


// --- Basic Motor Control Functions --- 
// These functions SET the motor pins and return IMMEDIATELY.
// Duration is handled by the calling task (tMotorControl).

void moveForward() { 
	//both sides move forward
	PTC->PSOR = MASK(LEFTENGINE_in)| MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

// Sharp Turn Left: Right Forward, Left Backward
void sharpTurnLeft() {
	//right side move forward
	PTC->PSOR = MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(RIGHTENGINE__out);
	
	//left side move back
	PTC->PCOR = MASK(LEFTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out);
}

// Sharp Turn Right: Right Backward, Left Forward
void sharpTurnRight() {
	//right side move back
	PTC->PCOR = MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(RIGHTENGINE__out);
	
	//left side move forward
	PTC->PSOR = MASK(LEFTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out);
}

// Renamed from moveBack
void moveBackward() {
	//both sides move back
	PTC->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

void stopMotors() {
	//set both sides to stop (both inputs low or both high, depends on driver)
	// Assuming setting both LOW stops the motor
	PTC->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

// Curve Left: Stop left motor, right motor forward
void curveLeft() {
    // Stop Left Motor
    PTC->PCOR = MASK(LEFTENGINE_in) | MASK(LEFTENGINE__out);
    // Right Motor Forward
    PTC->PSOR = MASK(RIGHTENGINE_in);
    PTC->PCOR = MASK(RIGHTENGINE__out);
}

// Curve Right: Stop right motor, left motor forward
void curveRight() {
    // Stop Right Motor
    PTC->PCOR = MASK(RIGHTENGINE_in) | MASK(RIGHTENGINE__out);
    // Left Motor Forward
    PTC->PSOR = MASK(LEFTENGINE_in);
    PTC->PCOR = MASK(LEFTENGINE__out);
}

// Placeholder for special movement command from tMotorControl
// Define what this should do if needed.
void specialMovement() {
	// Example: Stop motors, or perform a specific sequence?
	stopMotors();
}


// --- Motor Control Thread ---
// Controls movement based on the shared robot_state
void motor_control_thread(void *argument) {
    RobotState current_state;
    for (;;) {
        osMutexAcquire(robot_state_mutex, osWaitForever);
        current_state = robot_state;
        osMutexRelease(robot_state_mutex);

        switch (current_state) {
            case ROBOT_MOVING_FORWARD:
                moveForward();
                break;
            case ROBOT_MOVING_BACK:
                moveBackward();
                break;
            case ROBOT_MOVING_LEFT:
                sharpTurnLeft();
                break;
            case ROBOT_MOVING_RIGHT:
                sharpTurnRight();
                break;
            case ROBOT_CURVING_LEFT: // Add this case
                curveLeft();
                break;
            case ROBOT_CURVING_RIGHT: // Add this case
                curveRight();
                break;
            case ROBOT_STATIONARY:
            default:
                stopMotors();
                // Optional: Add a small delay here if stopping frequently
                // osDelay(10); 
                break;
        }
         // If using moveStop(), no osDelay is needed here as moveStop is non-blocking.
         // If you add delays to moveStop or want a general loop delay, add osDelay here.
    }
}

