#include "uart.h"

#define UART_OVERSAMPLE_RATE 16
#define BUS_CLOCK 24000000 // Assuming 24 MHz bus clock
#define UART_RX_PIN 22
#define UART_TX_PIN 23

// Message Queue ID defined externally (e.g., in main.c)
osMessageQueueId_t uart_msg_queue_id;

void UART2_Init(uint32_t baud_rate) {
    uint16_t sbr;

    // 1. Enable Clock to UART2 and Port E
    SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

    // 2. Configure UART pins (PTE22 as RX, PTE23 as TX)
    PORTE->PCR[UART_RX_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[UART_RX_PIN] |= PORT_PCR_MUX(4); // Alt 4 for UART2_RX
    PORTE->PCR[UART_TX_PIN] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[UART_TX_PIN] |= PORT_PCR_MUX(4); // Alt 4 for UART2_TX

    // 3. Disable UART Tx/Rx before configuration
    UART2->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);

    // 4. Set Baud Rate
    sbr = (uint16_t)((BUS_CLOCK) / (baud_rate * UART_OVERSAMPLE_RATE));
    UART2->BDH &= ~UART_BDH_SBR_MASK; 
    UART2->BDH |= UART_BDH_SBR(sbr >> 8);
    UART2->BDL = UART_BDL_SBR(sbr);

    // 5. Set Oversampling Ratio (OSR)
    UART2->C4 = UART_C4_OSR(UART_OVERSAMPLE_RATE - 1);
    UART2->C5 |= UART_C5_BOTHEDGE_MASK; // Enable Both Edge Sampling

    // 6. Enable UART Tx/Rx and Receive Interrupt
    UART2->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK);

    // 7. Enable UART2 interrupts in NVIC
    NVIC_SetPriority(UART2_IRQn, 128); // Low priority
    NVIC_ClearPendingIRQ(UART2_IRQn);
    NVIC_EnableIRQ(UART2_IRQn);
}

// Basic blocking character transmit (optional)
void UART2_TransmitChar(char data) {
    while (!(UART2->S1 & UART_S1_TDRE_MASK)); 
    UART2->D = data;
}

// Basic blocking character receive (optional)
char UART2_ReceiveChar(void) {
    while (!(UART2->S1 & UART_S1_RDRF_MASK)); 
    return UART2->D;
}

// UART2 Interrupt Service Routine
void UART2_IRQHandler(void) {
    char received_char;
    // Check RDRF flag
    if (UART2->S1 & UART_S1_RDRF_MASK) {
        received_char = UART2->D; // Read data (clears flag)
        // Send character to the message queue (non-blocking from ISR)
        osMessageQueuePut(uart_msg_queue_id, &received_char, 0U, 0U);
        // Add error handling for queue full if needed
    }
    // Add checks for other UART flags (overrun, framing error) if needed
}

