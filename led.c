#include "RTE_Components.h"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include "motor.h" // Includes RobotState enum and robot_state_mutex extern declaration
#include "led.h"   // Includes LED pin definitions

// --- Other Definitions ---
#define NUM_GREEN_LEDS 8
#define NUM_RED_LEDS 8

#define led_on    1 // Logical state: ON
#define led_off   0 // Logical state: OFF

// --- Helper Macro ---
#define MASK(x) (1UL << (x))

// --- LED Initialization ---
// This function remains the same - configures pins as GPIO outputs.
// The initial state setting at the end now uses the inverted helper functions.
void init_leds(void) {
    // Enable clock to ports used by LEDs (Port A, Port C, and Port D)
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;

    // --- Configure Green LEDs (All on Port C) ---
    PORTC->PCR[GREEN_LED_0] = PORT_PCR_MUX(1); // Set MUX to GPIO
    PORTC->PCR[GREEN_LED_1] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_2] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_3] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_4] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_5] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_6] = PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_7] = PORT_PCR_MUX(1);
    // Set Port C Green LED pins as outputs
    GREEN_LED_PORT->PDDR |= (MASK(GREEN_LED_0) | MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_3) |
                             MASK(GREEN_LED_4) | MASK(GREEN_LED_5) | MASK(GREEN_LED_6) | MASK(GREEN_LED_7));

    // --- Configure Red LEDs (Split across Port A, C and D) ---
    PORTA->PCR[RED_LED_0] = PORT_PCR_MUX(1); // Set MUX to GPIO
    PORTA->PCR[RED_LED_1] = PORT_PCR_MUX(1);
    PORTD->PCR[RED_LED_2] = PORT_PCR_MUX(1); // Port D
    PORTA->PCR[RED_LED_3] = PORT_PCR_MUX(1);
    PORTA->PCR[RED_LED_4] = PORT_PCR_MUX(1);
    PORTA->PCR[RED_LED_5] = PORT_PCR_MUX(1);
    PORTC->PCR[RED_LED_6] = PORT_PCR_MUX(1); // Port C
    PORTC->PCR[RED_LED_7] = PORT_PCR_MUX(1); // Port C

    // Set Red LED pins as outputs on respective ports
    RED_LED_PORT_A->PDDR   |= (MASK(RED_LED_0) | MASK(RED_LED_1) | MASK(RED_LED_3) | MASK(RED_LED_4) | MASK(RED_LED_5));
    RED_LED_PORT_C->PDDR   |= (MASK(RED_LED_6) | MASK(RED_LED_7));
    RED_LED_PORT_D->PDDR   |= MASK(RED_LED_2);

    // Turn off all LEDs initially using the *inverted* logic helpers
    all_green_leds_off(); // Should now call PCOR
    for(int i = 0; i < NUM_RED_LEDS; i++){
        set_red_led(i, led_off); // Should now call PCOR on the respective port
    }
}


// --- Helper function for setting individual green LED state ---
// INVERTED for ACTIVE HIGH (High = ON, Low = OFF)
void set_green_led(int index, int state) {
    uint32_t pin_mask = 0;
    switch(index) {
        case 0: pin_mask = MASK(GREEN_LED_0); break;
        case 1: pin_mask = MASK(GREEN_LED_1); break;
        case 2: pin_mask = MASK(GREEN_LED_2); break;
        case 3: pin_mask = MASK(GREEN_LED_3); break;
        case 4: pin_mask = MASK(GREEN_LED_4); break;
        case 5: pin_mask = MASK(GREEN_LED_5); break;
        case 6: pin_mask = MASK(GREEN_LED_6); break;
        case 7: pin_mask = MASK(GREEN_LED_7); break;
        default: return; // Invalid index
    }

    if (state == led_on) {
        GREEN_LED_PORT->PSOR = pin_mask; // Set bit HIGH to turn ON
    } else {
        GREEN_LED_PORT->PCOR = pin_mask; // Clear bit LOW to turn OFF
    }
}

// --- Helper function for setting individual red LED state ---
// INVERTED for ACTIVE HIGH (High = ON, Low = OFF)
void set_red_led(int index, int state) {
    GPIO_Type *port;
    uint32_t pin_mask = 0;

    switch(index) {
        case 0: port = RED_LED_PORT_A; pin_mask = MASK(RED_LED_0); break;
        case 1: port = RED_LED_PORT_A; pin_mask = MASK(RED_LED_1); break;
        case 2: port = RED_LED_PORT_D; pin_mask = MASK(RED_LED_2); break; // Port D
        case 3: port = RED_LED_PORT_A; pin_mask = MASK(RED_LED_3); break;
        case 4: port = RED_LED_PORT_A; pin_mask = MASK(RED_LED_4); break;
        case 5: port = RED_LED_PORT_A; pin_mask = MASK(RED_LED_5); break;
        case 6: port = RED_LED_PORT_C; pin_mask = MASK(RED_LED_6); break; // Port C
        case 7: port = RED_LED_PORT_C; pin_mask = MASK(RED_LED_7); break; // Port C
        default: return; // Invalid index
    }

    if (state == led_on) {
        port->PSOR = pin_mask; // Set bit HIGH to turn ON
    } else {
        port->PCOR = pin_mask; // Clear bit LOW to turn OFF
    }
}

// --- Helper function to turn all green LEDs ON ---
// INVERTED for ACTIVE HIGH
void all_green_leds_on(void) {
    // Use PSOR to turn all green LEDs ON (set bits high)
    GREEN_LED_PORT->PSOR = (MASK(GREEN_LED_0) | MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_3) |
                            MASK(GREEN_LED_4) | MASK(GREEN_LED_5) | MASK(GREEN_LED_6) | MASK(GREEN_LED_7));
}

// --- Helper function to turn all green LEDs OFF ---
// INVERTED for ACTIVE HIGH
void all_green_leds_off(void) {
    // Use PCOR to turn all green LEDs OFF (clear bits low)
    GREEN_LED_PORT->PCOR = (MASK(GREEN_LED_0) | MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_3) |
                            MASK(GREEN_LED_4) | MASK(GREEN_LED_5) | MASK(GREEN_LED_6) | MASK(GREEN_LED_7));
}


// --- LED Control Thread ---
// The logic calling the helpers remains the same, but the helpers now do the inverted action.
void led_control_thread(void *argument) {
    // State variables for timing and patterns
    uint32_t last_green_update_tick = 0;
    uint32_t last_red_toggle_tick = 0;
    int current_green_led_index = 0;
    int red_leds_state = led_off; // Start with red LEDs logically OFF

    // Timing intervals (in milliseconds)
    const uint32_t green_running_interval_ms = 100; // Requirement 1 speed
    const uint32_t red_moving_interval_ms = 500;    // Requirement 3 half-period
    const uint32_t red_stationary_interval_ms = 250;// Requirement 4 half-period

    // Initialize hardware LEDs once before starting the loop
    init_leds(); // Uses the new active-high helpers for initial OFF state

    for (;;) {
        uint32_t current_tick = osKernelGetTickCount(); // Get current system time in ms

        // --- 1. Get Robot State (Safely) ---
        osMutexAcquire(robot_state_mutex, osWaitForever);
        RobotState current_robot_state = robot_state; // Make a local copy
        osMutexRelease(robot_state_mutex);

        // --- 2. Handle LED Logic Based on State ---
        uint32_t current_red_interval_ms;

        if (current_robot_state == ROBOT_STATIONARY) {
            // --- STATIONARY State ---
            // Requirement 2: All green LEDs on continuously
            all_green_leds_on(); // Calls PSOR via helper -> All ON
            // Reset running state variables (good practice)
            current_green_led_index = 0;
            last_green_update_tick = current_tick;

            // Requirement 4: Red LEDs flash at 250ms ON, 250ms OFF
            current_red_interval_ms = red_stationary_interval_ms;

        } else {
            // --- MOVING State (Any state other than STATIONARY) ---
            // Requirement 1: Running Green LEDs (1 LED at a time ON)
            if (current_tick - last_green_update_tick >= green_running_interval_ms) {
                // Turn previous LED OFF
                int prev_green_led_index = (current_green_led_index + NUM_GREEN_LEDS - 1) % NUM_GREEN_LEDS;
                set_green_led(prev_green_led_index, led_off); // Calls PCOR via helper -> OFF

                // Turn current LED ON
                set_green_led(current_green_led_index, led_on); // Calls PSOR via helper -> ON

                // Advance to the next LED for the *next* iteration
                current_green_led_index = (current_green_led_index + 1) % NUM_GREEN_LEDS;

                // Record the time of this update
                last_green_update_tick = current_tick;
            }
            // If not enough time has passed, green LEDs remain as they were

            // Requirement 3: Red LEDs flash at 500ms ON, 500ms OFF
            current_red_interval_ms = red_moving_interval_ms;
        }

        // --- 3. Update Red LEDs (Common logic for both states, interval changes) ---
        // Check if it's time to toggle the red LEDs
        if (current_tick - last_red_toggle_tick >= current_red_interval_ms) {
            // Flip the desired logical state (ON to OFF or OFF to ON)
            red_leds_state = (red_leds_state == led_on) ? led_off : led_on;

            // Apply the new state to all red LEDs using the inverted helper
            for (int i = 0; i < NUM_RED_LEDS; i++) {
                set_red_led(i, red_leds_state); // Calls PSOR/PCOR via helper
            }

            // Record the time of this toggle
            last_red_toggle_tick = current_tick;
        }
        // If not enough time has passed, red LEDs remain as they were

        // --- 4. Yield CPU ---
        // Allow other threads to run. Adjust delay for responsiveness vs CPU usage.
        osDelay(20); // Check status and update LEDs roughly every 20ms
    }
}
