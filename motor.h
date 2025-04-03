// motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include "cmsis_os2.h" // Include for osMutexId_t and RobotState enum
// --- Robot State --- (Keep Enum Definition in led.h - Conceptual Link to LEDs)
typedef enum {
    ROBOT_STATIONARY,
		ROBOT_MOVING,
    ROBOT_MOVING_LEFT,
		ROBOT_MOVING_RIGHT,
		ROBOT_MOVING_BACK,
		ROBOT_MOVING_FORWARD
} RobotState;

// --- Mutex Declaration (Declare as extern - defined in main.c) ---
extern osMutexId_t robot_state_mutex;

// --- Robot State Variable Declaration ---  (Explicitly declare robot_state as extern)
// This is just a declaration, not a definition and tells the compiler when
// compiling led.c that robot_state exists somewhere else (main.c)
extern volatile RobotState robot_state;

// --- Function Prototypes ---
void init_Motor(void);
void moveUp(void);
void moveLeft(void);
void moveBack(void);
void moveRight(void);
void moveStop(void);
void motor_control_test_thread(void *argument);
void motor_control_thread(void *argument);


#endif // LED_H
