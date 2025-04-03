// motor.c
#include "motor.h" // Include the header file
#include "RTE_Components.h" // Still needed for some definitions potentially
#include "MKL25Z4.h" //Devide header file
#include "cmsis_os2.h"
#include <stdbool.h>

#define MASK(x) (1 << (x))

// Renamed to match declaration in motor.h
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
	moveStop();
}


// Removed osDelay from individual move functions
void moveUp() { 
	//both sides move forward
	PTC->PSOR = MASK(LEFTENGINE_in)| MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveLeft() {
	//right side move forward
	PTC->PSOR = MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
	
	//left side move back
	PTC->PCOR = MASK(LEFTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveRight() {
	//right side move back
	PTC->PCOR = MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
	
	//left side move forward
	PTC->PSOR = MASK(LEFTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveBack() {
	//both sides move back
	PTC->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveStop() {
	//set both sides to stop (both inputs low or both high, depends on driver)
	// Assuming setting both LOW stops the motor
	PTC->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	// Alternatively, if setting both HIGH stops:
	// PTC->PSOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	// PTC->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

// Curve Left: Stop left motor, right motor forward
void curveLeft() {
    // Stop Left Motor
    PTC->PCOR = MASK(LEFTENGINE_in) | MASK(LEFTENGINE__out);
    // Right Motor Forward
    PTC->PSOR = MASK(RIGHTENGINE_in);
    PTC->PCOR = MASK(RIGHTENGINE__out);
    osDelay(500); // Curve for 0.5s interval (adjust as needed)
}

// Curve Right: Stop right motor, left motor forward
void curveRight() {
    // Stop Right Motor
    PTC->PCOR = MASK(RIGHTENGINE_in) | MASK(RIGHTENGINE__out);
    // Left Motor Forward
    PTC->PSOR = MASK(LEFTENGINE_in);
    PTC->PCOR = MASK(LEFTENGINE__out);
    osDelay(500); // Curve for 0.5s interval (adjust as needed)
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
                moveUp();
                break;
            case ROBOT_MOVING_BACK:
                moveBack();
                break;
            case ROBOT_MOVING_LEFT:
                moveLeft();
                break;
            case ROBOT_MOVING_RIGHT:
                moveRight();
                break;
            case ROBOT_CURVING_LEFT: // Add this case
                curveLeft();
                break;
            case ROBOT_CURVING_RIGHT: // Add this case
                curveRight();
                break;
            case ROBOT_STATIONARY:
            default:
                moveStop();
                // Optional: Add a small delay here if stopping frequently
                // osDelay(10); 
                break;
        }
         // If using moveStop(), no osDelay is needed here as moveStop is non-blocking.
         // If you add delays to moveStop or want a general loop delay, add osDelay here.
    }
}
