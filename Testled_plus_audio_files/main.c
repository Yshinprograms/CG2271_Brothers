// main.c
#include "RTE_Components.h"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include "led.h"
#include "audio.h" // Include the audio header



// --- Mutex Definition and Initialization (Moved to main.c) ---
osMutexId_t robot_state_mutex;

// --- Robot State Variable (Moved to main.c) ---
volatile RobotState robot_state = ROBOT_STATIONARY; // Initial state
volatile bool runComplete = false;

// --- Motor Control Thread (Moved to main.c) ---
void motor_control_thread(void *argument) {
    // Simulate robot movement for testing
    for (;;) {
        // Move for 5 seconds
        osMutexAcquire(robot_state_mutex, osWaitForever);
        robot_state = ROBOT_MOVING;
        osMutexRelease(robot_state_mutex);
        osDelay(5000); // Simulate moving for 5 seconds

        // Stop for 5 seconds
        osMutexAcquire(robot_state_mutex, osWaitForever);
        robot_state = ROBOT_STATIONARY;
        osMutexRelease(robot_state_mutex);
        osDelay(5000); // Simulate stationary for 5 seconds
			
			  // For testing: Toggle runComplete flag to switch melodies.
        runComplete = !runComplete;
    }
}


// --- Main Function ---
int main (void) {
    // System Initialization
    SystemCoreClockUpdate();
    init_leds(); // Initialize LEDs

    osKernelInitialize();

    robot_state_mutex = osMutexNew(NULL); // Create the mutex here in main.c
    if (robot_state_mutex == NULL) {
        // Handle mutex creation error (e.g., print an error message)
        return -1; // Or some other error indication
    }

    osThreadNew(led_control_thread, NULL, NULL);
    osThreadNew(motor_control_thread, NULL, NULL); // Create motor control thread (now defined in main.c)
    
		osThreadNew(audio_thread, NULL, NULL); // Create the audio thread

    osKernelStart();
    for (;;) {}
}
