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
  
	MOTOR_PORT->PDDR |= MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	MOTOR_PORT->PDDR |= MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	
	//turn off motors initially
	moveStop();
}


// Removed osDelay from individual move functions
void moveUp() { 
	//both sides move forward
	MOTOR_PORT->PSOR = MASK(LEFTENGINE_in)| MASK(RIGHTENGINE_in);
	MOTOR_PORT->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveLeft() {
	//right side move forward
	MOTOR_PORT->PSOR = MASK(RIGHTENGINE_in);
	MOTOR_PORT->PCOR = MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
	
	//left side move back
	MOTOR_PORT->PCOR = MASK(LEFTENGINE_in);
	MOTOR_PORT->PSOR = MASK(LEFTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveRight() {
	//right side move back
	MOTOR_PORT->PCOR = MASK(RIGHTENGINE_in);
	MOTOR_PORT->PSOR = MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
	
	//left side move forward
	MOTOR_PORT->PSOR = MASK(LEFTENGINE_in);
	MOTOR_PORT->PCOR = MASK(LEFTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveBack() {
	//both sides move back
	MOTOR_PORT->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	MOTOR_PORT->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); // Move for 0.5s interval
}

void moveStop() {
	//set both sides to stop (both inputs low or both high, depends on driver)
	// Assuming setting both LOW stops the motor
	MOTOR_PORT->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	MOTOR_PORT->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	// Alternatively, if setting both HIGH stops:
	// MOTOR_PORT->PSOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	// MOTOR_PORT->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

// --- Motor Control Thread ---
// Controls movement based on the shared robot_state
void motor_control_thread (void *argument) {
	//actually control the movement
	RobotState current_state; 
	for (;;) {
        osMutexAcquire(robot_state_mutex, osWaitForever);
        current_state = robot_state; // Read shared state safely
        osMutexRelease(robot_state_mutex);
		    
		    switch (current_state) {
					// Corrected case statements (no bitwise OR)
					case ROBOT_MOVING_FORWARD:
					case ROBOT_MOVING: // Treat generic MOVING as FORWARD for now
						moveUp();
					break;
					case ROBOT_MOVING_LEFT:
						moveLeft();
					break;
					case ROBOT_MOVING_RIGHT:
						moveRight();
					break;
					case ROBOT_MOVING_BACK:
						moveBack();
					break;
					case ROBOT_STATIONARY:
					default: // Stop for STATIONARY or any unknown state
						moveStop();
					break;
				}
				
				// Add a small delay to yield CPU time
				osDelay(20); 
    }
}
