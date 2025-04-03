// main.c
#include "RTE_Components.h"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include "led.h"
#include "audio.h" // Include the audio header
#include "motor.h" // Include the motor header

// --- Mutex Definition and Initialization (Moved to main.c) ---
osMutexId_t robot_state_mutex;

// --- Robot State Variable (Moved to main.c) ---
volatile RobotState robot_state = ROBOT_STATIONARY; // Initial state
volatile bool runComplete = false;

// --- Test Sequence Thread (Optional - can replace the simple motor logic above) ---
// This thread now sets the state for the motor_control_thread in motor.c to act upon.
void test_sequence_thread(void *argument) {
	for(;;) {
		// Move Forward
		osMutexAcquire(robot_state_mutex, osWaitForever);
		robot_state = ROBOT_MOVING_FORWARD;
		runComplete = false; // Example: Play melody 1 when moving
		osMutexRelease(robot_state_mutex);
		osDelay(2000); // Move forward for 2 seconds
			
		// Curve Left
		osMutexAcquire(robot_state_mutex, osWaitForever);
		robot_state = ROBOT_CURVING_LEFT;
		runComplete = false;
		osMutexRelease(robot_state_mutex);
		osDelay(2000); // Turn left for 2 second
			
		// Curve Right
		osMutexAcquire(robot_state_mutex, osWaitForever);
		robot_state = ROBOT_CURVING_RIGHT;
		runComplete = false;
		osMutexRelease(robot_state_mutex);
		osDelay(2000); // Turn right for 2 second
			
		// Move Back
		osMutexAcquire(robot_state_mutex, osWaitForever);
		robot_state = ROBOT_MOVING_BACK;
		runComplete = false;
		osMutexRelease(robot_state_mutex);
		osDelay(2000); // Move back for 2 seconds
			
		// Stationary
		osMutexAcquire(robot_state_mutex, osWaitForever);
		robot_state = ROBOT_STATIONARY;
		runComplete = true; // Example: Play melody 2 when stopped
		osMutexRelease(robot_state_mutex);
		osDelay(5000); // Stay stationary for 2 seconds
	}
}


// --- Main Function ---
int main (void) {
    // System Initialization
    SystemCoreClockUpdate();
    init_leds(); // Initialize LEDs
	init_Motor(); // Initialize Motors (Corrected name)

    osKernelInitialize();

    robot_state_mutex = osMutexNew(NULL); // Create the mutex here in main.c
    if (robot_state_mutex == NULL) {
        // Handle mutex creation error (e.g., print an error message)
        return -1; // Or some other error indication
    }

    osThreadNew(led_control_thread, NULL, NULL);
    osThreadNew(motor_control_thread, NULL, NULL); // Create motor control thread (uses definition from motor.c)
	osThreadNew(audio_thread, NULL, NULL); // Create the audio thread
	osThreadNew(test_sequence_thread, NULL, NULL); // Create the test sequence thread

    osKernelStart();
    for (;;) {}
}
