#include "RTE_Components.h"
#include "MKL25Z4.h" 
#include "cmsis_os2.h"
#include <stdbool.h>
#include "led.c"

// --- Main Function ---
int main (void) {
    // System Initialization
    SystemCoreClockUpdate();
    init_leds(); // Initialize LEDs

    osKernelInitialize();

    robot_state_mutex = osMutexNew(NULL); // Create the mutex for robot_state
    if (robot_state_mutex == NULL) {
        // Handle mutex creation error (e.g., print an error message)
        return -1; // Or some other error indication
    }

    osThreadNew(led_control_thread, NULL, NULL);
    osThreadNew(motor_control_thread, NULL, NULL); // Create motor control thread (for testing)

    osKernelStart();
    for (;;) {}
}