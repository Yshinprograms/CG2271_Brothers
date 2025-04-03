// motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include "cmsis_os2.h" // Include for osMutexId_t and RobotState enum

// --- Motor Pin Definitions ---
#define LEFTENGINE_in 12 // PTC12
#define RIGHTENGINE_in 16 // PTC16
#define LEFTENGINE__out 13 // PTC13
#define RIGHTENGINE__out 17 // PTC17
#define MOTOR_PORT PTC

// --- Robot State --- (Unified Definition)
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
void motor_control_thread(void *argument);


#endif // MOTOR_H
