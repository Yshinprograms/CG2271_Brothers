// led.h
#ifndef LED_H
#define LED_H

#include "cmsis_os2.h" // Include for osMutexId_t and RobotState enum

// --- LED Pin Definitions (Keep in header for easy access) ---
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

// --- Robot State --- (Keep Enum Definition in led.h - Conceptual Link to LEDs)
typedef enum {
    ROBOT_STATIONARY,
    ROBOT_MOVING
} RobotState;

// --- Mutex Declaration (Declare as extern - defined in main.c) ---
extern osMutexId_t robot_state_mutex;

// --- Robot State Variable Declaration ---  (Explicitly declare robot_state as extern)
// This is just a declaration, not a definition and tells the compiler when
// compiling led.c that robot_state exists somewhere else (main.c)
extern volatile RobotState robot_state;

// --- Function Prototypes ---
void init_leds(void);
void running_green_leds(void);
void all_green_leds_on(void);
void all_green_leds_off(void);
void red_leds_moving_flash(void);
void red_leds_stationary_flash(void);
void led_control_thread(void *argument);
void set_green_led(int index, int state);
void set_red_led(int index, int state);

#endif // LED_H