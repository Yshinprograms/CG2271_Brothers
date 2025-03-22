#include "RTE_Components.h"
#include "MKL25Z4.h" 
#include "cmsis_os2.h"
#include <stdbool.h>

// --- LED Pin Definitions ---
// We'll define the pins individually, grouped by port and function (Green/Red)
// This makes it easier to see the mapping and modify later.

// GREEN LEDs (8 pins)
#define GREEN_LED_0  7  // PTC7
#define GREEN_LED_1  0  // PTC0
#define GREEN_LED_2  3  // PTC3
#define GREEN_LED_3  4  // PTC4
#define GREEN_LED_4  5  // PTC5
#define GREEN_LED_5  6  // PTC6
#define GREEN_LED_6  10 // PTC10
#define GREEN_LED_7  11 // PTC11
#define GREEN_LED_PORT PTC

// RED LEDs (8 pins)
#define RED_LED_0   1  // PTA1
#define RED_LED_1   2  // PTA2
#define RED_LED_2   4  // PTD4  <-- Note: Port D
#define RED_LED_3   12 // PTA12
#define RED_LED_4   4  // PTA4
#define RED_LED_5   5  // PTA5
#define RED_LED_6   8  // PTC8
#define RED_LED_7   9  // PTC9

// RED LEDs are split across two ports. Create separate port macros.
#define RED_LED_PORT_A PTA
#define RED_LED_PORT_C PTC
#define RED_LED_PORT_D PTD

// --- Other Definitions ---

#define NUM_GREEN_LEDS 8
#define NUM_RED_LEDS 8

#define led_on    1
#define led_off   0

// --- Robot State ---
typedef enum {
    ROBOT_STATIONARY,
    ROBOT_MOVING
} RobotState;

volatile RobotState robot_state = ROBOT_STATIONARY; // Initial state
osMutexId_t robot_state_mutex; // Mutex for protecting robot_state

// --- Helper Macro ---
#define MASK(x) (1UL << (x))

// --- Function Prototypes ---
void init_leds(void);
void running_green_leds(void); // Function for the running green LED effect
void all_green_leds_on(void);
void all_green_leds_off(void); //Add helper function to turn all green leds off
void red_leds_moving_flash(void);
void red_leds_stationary_flash(void);
void led_control_thread(void *argument);
void set_green_led(int index, int state); //helper function for running_green_leds
void set_red_led(int index, int state);    // Helper function for red LEDs

/* Delay Function */
static void delay (volatile uint32_t nof) {
    while (nof!=0) {
        asm("NOP");
        nof--;
    }
}



// --- LED Initialization ---
void init_leds(void) {
    // Enable clock to ports used by LEDs (Port A, Port C, and Port D)
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK;

    // --- Configure Green LEDs (All on Port C) ---
    PORTC->PCR[GREEN_LED_0] = (PORTC->PCR[GREEN_LED_0] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_1] = (PORTC->PCR[GREEN_LED_1] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_2] = (PORTC->PCR[GREEN_LED_2] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_3] = (PORTC->PCR[GREEN_LED_3] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_4] = (PORTC->PCR[GREEN_LED_4] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_5] = (PORTC->PCR[GREEN_LED_5] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_6] = (PORTC->PCR[GREEN_LED_6] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
    PORTC->PCR[GREEN_LED_7] = (PORTC->PCR[GREEN_LED_7] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(1);
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



// --- LED Control Functions ---

void running_green_leds(void) {
    static int current_led = 0; // Keep track of the currently lit LED

    // Turn off all LEDs except the current one
    for (int i = 0; i < NUM_GREEN_LEDS; i++) {
        set_green_led(i, (i == current_led) ? led_on : led_off);
    }

    current_led = (current_led + 1) % NUM_GREEN_LEDS; // Move to the next LED (circular)
    osDelay(100); // Adjust delay for the running speed (e.g., 100ms)
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

void red_leds_moving_flash(void) {
    for (int i = 0; i < NUM_RED_LEDS; i++) {
			//Determine the correct port
			GPIO_Type * port = (i==2) ? RED_LED_PORT_D : ((i==6) || (i==7)) ? RED_LED_PORT_C : RED_LED_PORT_A;
        set_red_led(i, (port->PDIR & MASK( (i==0) ? RED_LED_0 :
                                                  (i==1) ? RED_LED_1 :
                                                  (i==2) ? RED_LED_2 :
                                                  (i==3) ? RED_LED_3 :
                                                  (i==4) ? RED_LED_4 :
                                                  (i==5) ? RED_LED_5 :
                                                  (i==6) ? RED_LED_6 :
                                                           RED_LED_7
                                                )) ? led_off : led_on);  // Read current state, then toggle
    }
    osDelay(500); // 500ms ON, 500ms OFF
}

void red_leds_stationary_flash(void) {
     for (int i = 0; i < NUM_RED_LEDS; i++) {
			//Determine the correct port
			GPIO_Type * port = (i==2) ? RED_LED_PORT_D : ((i==6) || (i==7)) ? RED_LED_PORT_C : RED_LED_PORT_A;	 
        set_red_led(i, (port->PDIR & MASK( (i==0) ? RED_LED_0 :
                                                  (i==1) ? RED_LED_1 :
                                                  (i==2) ? RED_LED_2 :
                                                  (i==3) ? RED_LED_3 :
                                                  (i==4) ? RED_LED_4 :
                                                  (i==5) ? RED_LED_5 :
                                                  (i==6) ? RED_LED_6 :
                                                           RED_LED_7
                                                )) ? led_off : led_on);  // Read current state and toggle
    }
    osDelay(250); // 250ms ON, 250ms OFF
}

// --- LED Control Thread ---

void led_control_thread(void *argument) {
  for (;;) {
    // Acquire mutex to protect access to robot_state
    osMutexAcquire(robot_state_mutex, osWaitForever);
    RobotState current_state = robot_state; // Make a local copy
    osMutexRelease(robot_state_mutex);

        if (current_state == ROBOT_MOVING) {
            running_green_leds();
            red_leds_moving_flash();
        } else { // ROBOT_STATIONARY
            all_green_leds_on();
            red_leds_stationary_flash();
        }
    //No osDelay here, it is handled inside the functions.
  }
}


// --- Example Motor Control Thread (for testing) ---
//  (This thread would be provided by your teammates)

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
    }
}