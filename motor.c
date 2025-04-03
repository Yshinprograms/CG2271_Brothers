// led.c
#include "motor.h" // Include the header file
#include "RTE_Components.h" // Still needed for some definitions potentially
#include "MKL25Z4.h" //Devide header file
#include "cmsis_os2.h"
#include <stdbool.h>

#define LEFTENGINE_in 12 //input for four engines
#define RIGHTENGINE_in 16

#define LEFTENGINE__out 13 //output for four engines
#define RIGHTENGINE__out 17

#define MASK(x) (1 << (x))

void initMotor() {
	//if the speed is too much we probably can use a timer(PWM) to vary duty cycledx
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;

	PORTC->PCR[LEFTENGINE_in] &= ~PORT_PCR_MUX_MASK;
  PORTC->PCR[LEFTENGINE_in] |= PORT_PCR_MUX(1);
  PORTC->PCR[LEFTENGINE__out] &= ~PORT_PCR_MUX_MASK;
  PORTC->PCR[LEFTENGINE__out] |= PORT_PCR_MUX(1);
	
	PORTC->PCR[RIGHTENGINE_in] &= ~PORT_PCR_MUX_MASK;
  PORTC->PCR[RIGHTENGINE_in] |= PORT_PCR_MUX(1);
  PORTC->PCR[RIGHTENGINE__out] &= ~PORT_PCR_MUX_MASK;
  PORTC->PCR[RIGHTENGINE__out] |= PORT_PCR_MUX(1);
  
	PTC->PDDR |= MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PDDR |= MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	
	//turn off motors 
}


void moveUp() { 
	//both sides move forward
	PTC->PSOR = MASK(LEFTENGINE_in)| MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); //move for half a second
}

void moveLeft() {
	//left side move backward  
	PTC->PSOR = MASK(LEFTENGINE__out);
	PTC->PCOR = MASK(LEFTENGINE_in);
	//right side move forward
	PTC->PSOR = MASK(RIGHTENGINE_in);
	PTC->PCOR = MASK(RIGHTENGINE__out);
	osDelay(500); //move for half a second
}

void moveRight() {
	//right side move backward  
	PTC->PSOR = MASK(RIGHTENGINE__out);
	PTC->PCOR = MASK(RIGHTENGINE_in);
	//left side move forward
	PTC->PSOR = MASK(LEFTENGINE_in);
	PTC->PCOR = MASK(LEFTENGINE__out);
	osDelay(500); //move for half a second
}

void moveBack() {
	//both sides move back
	PTC->PCOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
	osDelay(500); //move for half a second
}

void moveStop() {
	//set both sides
	PTC->PSOR = MASK(LEFTENGINE_in) | MASK(RIGHTENGINE_in);
	PTC->PSOR = MASK(LEFTENGINE__out) | MASK(RIGHTENGINE__out);
}

// --- Motor Control Thread ---
// to test out the motor functions
void motor_control__test_thread (void *argument) {
// Simulate robot movement for testing
	RobotState robot_state;
    for (;;) {
        // Move forward for 5 seconds
			  robot_state = ROBOT_MOVING_FORWARD;
				motor_control_thread(NULL);
        //osDelay(5000); // Simulate moving for 5 seconds
			
        // Stop for 5 seconds
        robot_state = ROBOT_STATIONARY;
				motor_control_thread(NULL);
        osDelay(5000); 

				// Move left for 5 seconds
				robot_state = ROBOT_MOVING_LEFT;
				motor_control_thread(NULL);
        //osDelay(5000); // Simulate moving for 5 seconds
			
			  // Stop for 5 seconds
        robot_state = ROBOT_STATIONARY;
				motor_control_thread(NULL);
        osDelay(5000); 

				// Move right for 5 seconds
				robot_state = ROBOT_MOVING_RIGHT;
				motor_control_thread(NULL);
        //osDelay(5000); // Simulate moving for 5 seconds
			
			  // Stop for 5 seconds
        robot_state = ROBOT_STATIONARY;
				motor_control_thread(NULL);
        osDelay(5000); 
				
				// Move back for 5 seconds
				robot_state = ROBOT_MOVING_BACK;
				motor_control_thread(NULL);
        //osDelay(5000); // Simulate moving for 5 seconds
			
			  // Stop for 5 seconds
        robot_state = ROBOT_STATIONARY;
				motor_control_thread(NULL);
        osDelay(5000); 
    }
}

// --- Motor Control Thread ---

void motor_control_thread (void *argument) {
	//actually control the movement
	for (;;) {
        osMutexAcquire(robot_state_mutex, osWaitForever);
        RobotState current_state = robot_state;
		    switch (current_state) {
					case (ROBOT_MOVING_FORWARD | ROBOT_MOVING) :
						moveUp();
						moveStop();
					break;
					case (ROBOT_MOVING_LEFT):
						moveLeft();
						moveStop();
					break;
					case (ROBOT_MOVING_RIGHT):
						moveRight();
						moveStop();
					break;
					case (ROBOT_MOVING_BACK) :
						moveBack();
						moveStop();
					break;
					default: 
						moveStop();
					break;
				}
        osMutexRelease(robot_state_mutex);
    }
}
