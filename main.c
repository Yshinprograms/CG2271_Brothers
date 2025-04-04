// main.c
#include "RTE_Components.h"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include "led.h"
#include "audio.h" // Include the audio header
#include "motor.h" // Include the motor header
#include "uart.h"  // Include the UART header

// --- Task Function Prototypes ---
void tBrain(void *argument);          // Brain task to process commands
void tMotorControl(void *argument);   // Motor control task
// led_control_thread and audio_thread prototypes assumed in their respective .h files

// --- Message Queue Definitions ---
osMessageQueueId_t uart_msg_queue_id;     // Queue for UART ISR -> tBrain
osMessageQueueId_t motor_cmd_queue_id;    // Queue for tBrain -> tMotorControl
#define UART_QUEUE_SIZE 16
#define MOTOR_QUEUE_SIZE 8

// --- Command Definitions ---
typedef enum {
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_SPECIAL,
    CMD_STOP
} MotorCommand_t;

// --- Mutex Definition and Initialization (Moved to main.c) ---
osMutexId_t robot_state_mutex;

// --- Robot State Variable (Moved to main.c) ---
volatile RobotState robot_state = ROBOT_STATIONARY; // Initial state
volatile bool runComplete = false;

// --- Brain Task (Processes Commands from UART) ---
void tBrain(void *argument) {
    char received_char;
    osStatus_t status;

    for (;;) {
        // Wait for a character from the UART interrupt queue
        status = osMessageQueueGet(uart_msg_queue_id, &received_char, NULL, osWaitForever);

        if (status == osOK) {
            MotorCommand_t cmd_to_send; // Command to send to motor task
            bool valid_command = true;

            // Process the received character and prepare command for tMotorControl
            switch (received_char) {
                case 'F': cmd_to_send = CMD_FORWARD;  break;
                case 'B': cmd_to_send = CMD_BACKWARD; break;
                case 'L': cmd_to_send = CMD_LEFT;     break;
                case 'R': cmd_to_send = CMD_RIGHT;    break;
                case 'D': cmd_to_send = CMD_SPECIAL;  break; // Assuming 'D' maps to special
                case 'S': cmd_to_send = CMD_STOP;     break;
                default:  valid_command = false;      break; // Ignore invalid characters
            }

            if (valid_command) {
                // Send the command to the motor control task queue
                osMessageQueuePut(motor_cmd_queue_id, &cmd_to_send, 0U, 0U);

                // Update the global robot state (protected by mutex)
                osMutexAcquire(robot_state_mutex, osWaitForever);
                if (cmd_to_send == CMD_STOP) {
                    robot_state = ROBOT_STATIONARY;
                    runComplete = true; // Example: set flag when stopping
                } else {
                    robot_state = ROBOT_MOVING; // Generic moving state
                    // You might want more specific moving states here based on cmd_to_send
                    runComplete = false; // Example: clear flag when moving
                }
                osMutexRelease(robot_state_mutex);
            }
        }
    }
}

// --- Motor Control Task (Waits for commands from tBrain) ---
void tMotorControl(void *argument) {
    MotorCommand_t received_command;
    osStatus_t status;

    for (;;) {
        // Wait for a command from the brain task queue
        status = osMessageQueueGet(motor_cmd_queue_id, &received_command, NULL, osWaitForever);

        if (status == osOK) {
            // Execute the motor action based on the command
            switch(received_command) {
                case CMD_FORWARD:   moveForward();    break; // Assumes functions in motor.c
                case CMD_BACKWARD:  moveBackward();   break;
                case CMD_LEFT:      turnLeft();       break;
                case CMD_RIGHT:     turnRight();      break;
                case CMD_SPECIAL:   specialMovement();break; // Define this function
                case CMD_STOP:      stopMotors();     break;
            }
        }
    }
}

// --- Main Function ---
int main (void) {
    // System Initialization
    SystemCoreClockUpdate();

    // Initialize Peripherals
    init_leds();    // Initialize LEDs
    init_Motor();   // Initialize Motors (Assuming function in motor.c)
    UART2_Init(9600); // Initialize UART2 with 9600 baud

    osKernelInitialize();

    // Create Mutex
    robot_state_mutex = osMutexNew(NULL); // Create the mutex here in main.c
    if (robot_state_mutex == NULL) {
        // Handle mutex creation error (e.g., print an error message)
        return -1; // Or some other error indication
    }

    // Create Message Queues
    uart_msg_queue_id = osMessageQueueNew(UART_QUEUE_SIZE, sizeof(char), NULL);
    motor_cmd_queue_id = osMessageQueueNew(MOTOR_QUEUE_SIZE, sizeof(MotorCommand_t), NULL);
    if (uart_msg_queue_id == NULL || motor_cmd_queue_id == NULL) {
        // Handle queue creation error
        return -1;
    }

    // Create Threads
    osThreadNew(led_control_thread, NULL, NULL);
    osThreadNew(audio_thread, NULL, NULL);
    osThreadNew(tBrain, NULL, NULL);           // Add Brain task
    osThreadNew(tMotorControl, NULL, NULL);    // Add Motor Control task

    osKernelStart();
    for (;;) {}
}
