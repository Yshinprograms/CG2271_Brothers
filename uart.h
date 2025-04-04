#ifndef UART_H
#define UART_H

#include "MKL25Z4.h"
#include "cmsis_os2.h"

// Define the message queue handle for UART data
extern osMessageQueueId_t uart_msg_queue_id;

// Function prototypes
void UART2_Init(uint32_t baud_rate);
char UART2_ReceiveChar(void); // Optional blocking receive
void UART2_TransmitChar(char data); // Optional blocking transmit

#endif // UART_H
