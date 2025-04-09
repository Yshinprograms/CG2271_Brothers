// main.c
#include "RTE_Components.h"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include "led.h"
#include "audio.h" // Include the audio header
#include "motor.h" // Include the motor header
#define RED_LED    18  // PortB Pin 18
#define GREEN_LED  19  // PortB Pin 19
#define BLUE_LED  1    // PortD Pin 1
#define MASK(x) (1 << (x))

// --- Mutex Definition and Initialization (Moved to main.c) ---
osMutexId_t robot_state_mutex;

// --- Robot State Variable (Moved to main.c) ---
volatile RobotState robot_state = ROBOT_STATIONARY; // Initial state
volatile bool runComplete = false;
volatile char received_command = 0;  // Stores received command


void InitGPIO(void)
{
  // Enable Clock to PORTB and PORTD
  SIM->SCGC5 |= ((SIM_SCGC5_PORTB_MASK) | (SIM_SCGC5_PORTD_MASK));
  
  // Configure MUX settings to make all 3 pins GPIO
  
  PORTB->PCR[RED_LED] &= ~PORT_PCR_MUX_MASK;
  PORTB->PCR[RED_LED] |= PORT_PCR_MUX(1);
  
  PORTB->PCR[GREEN_LED] &= ~PORT_PCR_MUX_MASK;
  PORTB->PCR[GREEN_LED] |= PORT_PCR_MUX(1);
  
  PORTD->PCR[BLUE_LED] &= ~PORT_PCR_MUX_MASK;
  PORTD->PCR[BLUE_LED] |= PORT_PCR_MUX(1);
  
  // Set Data Direction Registers for PortB and PortD
  PTB->PDDR |= (MASK(RED_LED) | MASK(GREEN_LED));
  PTD->PDDR |= MASK(BLUE_LED);
  
}


// UART2 Initialization
void UART2_Init(void) {
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;  // Enable clock for UART2
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;  // Enable clock for Port E

    PORTE->PCR[23] = PORT_PCR_MUX(4);  // Set PTE22 as UART2_RX
    PORTE->PCR[22] = PORT_PCR_MUX(4);  // Set PTE23 as UART2_TX

    UART2->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);  // Disable TX and RX during config

    UART2->BDH = 0x00;
    UART2->BDL = 0x1A;  // Set baud rate to 9600 (assuming 24MHz bus clock)
    UART2->C4 = 0x0F;   // Oversampling ratio

    UART2->C2 |= UART_C2_RE_MASK;  // Enable Receiver
    UART2->C2 |= UART_C2_RIE_MASK; // Enable Receive Interrupt

    NVIC_EnableIRQ(UART2_IRQn);  // Enable UART2 interrupt in NVIC

    UART2->C2 |= UART_C2_TE_MASK;  // Enable Transmitter
	
		InitGPIO();
}


void UART2_IRQHandler(void) {				
	  PTB->PSOR = (MASK(RED_LED) | MASK(GREEN_LED));
  PTD->PSOR = MASK(BLUE_LED);

		PTB->PCOR = MASK(RED_LED); //turn on red led
    if (UART2->S1 & UART_S1_RDRF_MASK) {  // Check if receive buffer is full
        received_command = UART2->D;  // Read received character


        // Process only valid commands
        if (received_command == 'F' || received_command == 'L'  || received_command == 'R' ||
            received_command == 'B' || received_command == 'D' || received_command == 'S') {
            //control_motors(received_command);
        }
    }
}



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
    //init_leds(); // Initialize LEDs
    //init_Motor(); // Initialize Motors (Corrected name)
    UART2_Init();  // Initialize UART2 for interrupt-based reception

    // Enable global interrupts
    __enable_irq();
  
    osKernelInitialize();

    robot_state_mutex = osMutexNew(NULL); // Create the mutex here in main.c
    if (robot_state_mutex == NULL) {
        // Handle mutex creation error (e.g., print an error message)
        return -1; // Or some other error indication
    }
		//osThreadNew(led_control_thread, NULL, NULL);
    //osThreadNew(motor_control_thread, NULL, NULL); // Create motor control thread (uses definition from motor.c)
    //osThreadNew(audio_thread, NULL, NULL); // Create the audio thread
    //osThreadNew(test_sequence_thread, NULL, NULL); // Create the test sequence thread

    osKernelStart();
    for (;;) {}
}
