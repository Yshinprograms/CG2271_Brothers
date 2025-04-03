// led.c
#include "led.h" // Include the header file
#include "motor.h" // Include motor.h for RobotState (needed by led.h)
#include "RTE_Components.h" // Still needed for some definitions potentially
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>

// --- Helper Macro --- (Keep Helper Macro)
#define MASK(x) (1UL << (x))

/* Delay Function */
/*
static void delay (volatile uint32_t nof) {
    while (nof!=0) {
        __asm("NOP");
        nof--;
    }
}
*/

// --- LED Initialization ---
void init_leds(void) {
    // Enable clock to ports used by LEDs (Port A, Port C, and Port D)
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;

		// Configuring MUX = 001 for GPIO
    // --- Configure Green LEDs (All on Port C) ---
    PORTC->PCR[GREEN_LED_0] = (PORTC->PCR[GREEN_LED_0] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_1] = (PORTC->PCR[GREEN_LED_1] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_2] = (PORTC->PCR[GREEN_LED_2] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_3] = (PORTC->PCR[GREEN_LED_3] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_4] = (PORTC->PCR[GREEN_LED_4] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_5] = (PORTC->PCR[GREEN_LED_5] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_6] = (PORTC->PCR[GREEN_LED_6] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_7] = (PORTC->PCR[GREEN_LED_7] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
		// Set direction to output
    GREEN_LED_PORT->PDDR |= (MASK(GREEN_LED_0) | MASK(GREEN_LED_1) | MASK(GREEN_LED_2) | MASK(GREEN_LED_3) |
                             MASK(GREEN_LED_4) | MASK(GREEN_LED_5) | MASK(GREEN_LED_6) | MASK(GREEN_LED_7));

    // --- Configure Red LEDs (Split across Port A, C and D) ---
		PORTA->PCR[RED_LED_0] = (PORTA->PCR[RED_LED_0] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTA->PCR[RED_LED_1] = (PORTA->PCR[RED_LED_1] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTD->PCR[RED_LED_2] = (PORTD->PCR[RED_LED_2] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1); // Port D
    PORTA->PCR[RED_LED_3] = (PORTA->PCR[RED_LED_3] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTA->PCR[RED_LED_4] = (PORTA->PCR[RED_LED_4] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTA->PCR[RED_LED_5] = (PORTA->PCR[RED_LED_5] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[RED_LED_6] = (PORTC->PCR[RED_LED_6] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
		PORTC->PCR[RED_LED_7] = (PORTC->PCR[RED_LED_7] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    RED_LED_PORT_A->PDDR   |= (MASK(RED_LED_0) | MASK(RED_LED_1) | MASK(RED_LED_3) | MASK(RED_LED_4) | MASK(RED_LED_5));
    RED_LED_PORT_C->PDDR   |= (MASK(RED_LED_6) | MASK(RED_LED_7));
    RED_LED_PORT_D->PDDR   |= MASK(RED_LED_2); // Port D

    // Turn off all LEDs initially
    all_green_leds_off();
    for(int i = 0; i < NUM_RED_LEDS; i++){
        set_red_led(i, led_off); // Use helper function
    }
}


// --- Helper function for setting individual green LED state ---
void set_green_led(int index, int state) {
		// Clear bit to turn LED on (active low)
		// Set bit to turn LED off (active low)
    switch(index) {
        case 0: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_0) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_0) : 0; break;
        case 1: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_1) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_1) : 0; break;
        case 2: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_2) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_2) : 0; break;
        case 3: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_3) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_3) : 0; break;
        case 4: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_4) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_4) : 0; break;
        case 5: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_5) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_5) : 0; break;
        case 6: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_6) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_6) : 0; break;
        case 7: GREEN_LED_PORT->PCOR = (state == led_on) ? MASK(GREEN_LED_7) : 0; GREEN_LED_PORT->PSOR = (state == led_off) ? MASK(GREEN_LED_7) : 0; break;
    }
}

// --- Helper function for setting individual red LED state ---
//  This needs to handle LEDs on *different* ports.
void set_red_led(int index, int state) {
  switch(index) {
      case 0: RED_LED_PORT_A->PCOR = (state == led_on) ? MASK(RED_LED_0) : 0; RED_LED_PORT_A->PSOR = (state == led_off) ? MASK(RED_LED_0) : 0; break;
      case 1: RED_LED_PORT_A->PCOR = (state == led_on) ? MASK(RED_LED_1) : 0; RED_LED_PORT_A->PSOR = (state == led_off) ? MASK(RED_LED_1) : 0; break;
      case 2: RED_LED_PORT_D->PCOR = (state == led_on) ? MASK(RED_LED_2) : 0; RED_LED_PORT_D->PSOR = (state == led_off) ? MASK(RED_LED_2) : 0; break; // Port D
      case 3: RED_LED_PORT_A->PCOR = (state == led_on) ? MASK(RED_LED_3) : 0; RED_LED_PORT_A->PSOR = (state == led_off) ? MASK(RED_LED_3) : 0; break;
      case 4: RED_LED_PORT_A->PCOR = (state == led_on) ? MASK(RED_LED_4) : 0; RED_LED_PORT_A->PSOR = (state == led_off) ? MASK(RED_LED_4) : 0; break;
      case 5: RED_LED_PORT_A->PCOR = (state == led_on) ? MASK(RED_LED_5) : 0; RED_LED_PORT_A->PSOR = (state == led_off) ? MASK(RED_LED_5) : 0; break;
      case 6: RED_LED_PORT_C->PCOR = (state == led_on) ? MASK(RED_LED_6) : 0; RED_LED_PORT_C->PSOR = (state == led_off) ? MASK(RED_LED_6) : 0; break;
      case 7: RED_LED_PORT_C->PCOR = (state == led_on) ? MASK(RED_LED_7) : 0; RED_LED_PORT_C->PSOR = (state == led_off) ? MASK(RED_LED_7) : 0; break;
  }
}

// --- Helper function to set ALL red LEDs to a specific state ---
void set_all_red_leds(int state) {
	for (int i = 0; i < NUM_RED_LEDS; i++) {
		set_red_led(i, state);
	}
}

// --- LED Control Functions (Modified: No internal delays) ---

// Sets the green LEDs to the running state (one on, others off)
void running_green_leds(int current_led_index) {
    // Turn off all LEDs except the current one
    for (int i = 0; i < NUM_GREEN_LEDS; i++) {
        set_green_led(i, (i == current_led_index) ? led_on : led_off);
    }
		// Delay removed - Timing is handled by the calling thread
}

void all_green_leds_on(void) {
    for (int i = 0; i < NUM_GREEN_LEDS; i++) {
        set_green_led(i, led_on);
    }
}

void all_green_leds_off(void) {
    for (int i = 0; i < NUM_GREEN_LEDS; i++) {
        set_green_led(i, led_off);
    }
}

// --- LED Control Thread (Refactored for correct timing) ---

void led_control_thread(void *argument) {
	RobotState current_state = ROBOT_STATIONARY; // Local variable to hold state
	RobotState previous_state = ROBOT_STATIONARY;
	int green_led_index = 0;
	bool red_led_state = false; // false = off, true = on
	uint32_t last_green_change_tick = 0;
	uint32_t last_red_toggle_tick = 0;
	uint32_t green_running_interval = 100; // ms between green LED changes
	uint32_t red_moving_interval = 500;    // ms ON / ms OFF for moving red LEDs
	uint32_t red_stationary_interval = 250;// ms ON / ms OFF for stationary red LEDs
	uint32_t tick_freq = osKernelGetTickFreq(); // Ticks per second

	// Convert ms intervals to ticks
	uint32_t green_running_ticks = (green_running_interval * tick_freq) / 1000;
	uint32_t red_moving_ticks = (red_moving_interval * tick_freq) / 1000;
	uint32_t red_stationary_ticks = (red_stationary_interval * tick_freq) / 1000;

  // Initial LED state
	all_green_leds_on();
	set_all_red_leds(led_off); // Start with red LEDs off
	last_red_toggle_tick = osKernelGetTickCount();
	last_green_change_tick = osKernelGetTickCount();

  for (;;) {
    uint32_t now = osKernelGetTickCount();

    // Acquire mutex to protect access to robot_state
    osMutexAcquire(robot_state_mutex, osWaitForever); // Access mutex declared in main.c via led.h
    current_state = robot_state; // Make a local copy
    osMutexRelease(robot_state_mutex);

		// --- Handle State Transitions ---
		if (current_state != previous_state) {
			last_red_toggle_tick = now; // Reset timers on state change
			last_green_change_tick = now;
			green_led_index = 0; // Reset green index
			if (current_state == ROBOT_STATIONARY) {
				all_green_leds_on();
				red_led_state = false; // Ensure red starts OFF for stationary
				set_all_red_leds(led_off);
			} else { // Moving state
				running_green_leds(green_led_index); // Set initial running green LED
				red_led_state = true; // Ensure red starts ON for moving
				set_all_red_leds(led_on);
			}
			previous_state = current_state;
		}

		// --- Update LEDs based on current state and timing ---
    if (current_state == ROBOT_MOVING || 
				current_state == ROBOT_MOVING_FORWARD ||
				current_state == ROBOT_MOVING_LEFT ||
				current_state == ROBOT_MOVING_RIGHT ||
				current_state == ROBOT_MOVING_BACK) 
		{ // --- Moving State --- 
			// Green Running LEDs (Update every green_running_interval ms)
			if (now - last_green_change_tick >= green_running_ticks) {
				green_led_index = (green_led_index + 1) % NUM_GREEN_LEDS;
				running_green_leds(green_led_index);
				last_green_change_tick = now;
			}
			// Red Flashing LEDs (Toggle every red_moving_interval ms)
			if (now - last_red_toggle_tick >= red_moving_ticks) {
				red_led_state = !red_led_state;
				set_all_red_leds(red_led_state ? led_on : led_off);
				last_red_toggle_tick = now;
			}
    } else { // --- Stationary State --- 
			// Green LEDs: Ensure they stay ON (handled by state transition)
			// Red Flashing LEDs (Toggle every red_stationary_interval ms)
			if (now - last_red_toggle_tick >= red_stationary_ticks) {
				red_led_state = !red_led_state;
				set_all_red_leds(red_led_state ? led_on : led_off);
				last_red_toggle_tick = now;
			}
    }
    osDelay(10); // Loop delay for responsiveness (adjust if needed)
  }
}
