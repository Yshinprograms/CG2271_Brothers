#include "MKL25Z4.h"  // Device header
#include "audio.h"
#include "cmsis_os2.h" // Include for osDelay
#include "led.h" // Include for robot_state_mutex extern declaration

#define PTB0_Pin 0
#define PTB1_Pin 1

void initPWM(int frequency) 
{
  // Enable clock gating for PORTB.
  SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
  
  // Configure Mode 3 for PWM pin configuration.
  PORTB->PCR[PTB0_Pin] &= ~PORT_PCR_MUX_MASK;
  PORTB->PCR[PTB0_Pin] |= PORT_PCR_MUX(3);
  
  PORTB->PCR[PTB1_Pin] &= ~PORT_PCR_MUX_MASK;
  PORTB->PCR[PTB1_Pin] |= PORT_PCR_MUX(3);
  
  // Enable clock gating for Timer 1.
  SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
  
  // Select clock source.
  SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
  SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
  
  // Set MOD register for PWM frequency calculation (integer ceiling).
  TPM1->MOD = (48000000 / 128 + frequency - 1) / frequency;
  TPM1_C0V = (TPM1->MOD + 1) / 2; // 50% duty cycle
  
  // Set edge-aligned PWM mode.
  TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK));
  TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7));
  TPM1->SC &= ~(TPM_SC_CPWMS_MASK);
  
   TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) |
                  (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
   // Set Mode to Edge-aligned PWM, High-true pulses (clear output on match, set on reload)
   TPM1_C0SC |= (TPM_CnSC_MSB(1) | TPM_CnSC_ELSB(1)); 
 }

// Define Melodies and Durations (can be moved to audio.h if preferred)
// Melody 1: Mary Had a Little Lamb (Continuous Run Song)
const int melody1_notes[] = {
    330, 294, 262, 294, 330, 330, 330, // E D C D E E E
    294, 294, 294,                   // D D D
    330, 392, 392,                   // E G G
    330, 294, 262, 294, 330, 330, 330, // E D C D E E E
    0 // Add a small pause at the end before repeating
};
const int melody1_durations[] = { // Corresponds to melody1_notes
    400, 400, 400, 400, 400, 400, 800, // Durations in ms
    400, 400, 800,
    400, 400, 800,
    400, 400, 400, 400, 400, 400, 800,
    200 // Pause duration
};
const int melody1_numNotes = sizeof(melody1_notes) / sizeof(melody1_notes[0]);

// Melody 2: Twinkle Twinkle (Completion Tone)
const int melody2_notes[] = {
    262, 262, 392, 392, 440, 440, 392, // C C G G A A G
    349, 349, 330, 330, 294, 294, 262  // F F E E D D C
};
const int melody2_durations[] = { // Corresponds to melody2_notes
    500, 500, 500, 500, 500, 500, 1000,
    500, 500, 500, 500, 500, 500, 1000
};
const int melody2_numNotes = sizeof(melody2_notes) / sizeof(melody2_notes[0]);


// --- Audio Thread Function (Refactored) ---
// Plays Melody 1 continuously until runComplete is true, then plays Melody 2 once.
void audio_thread(void *argument) {
    int current_note_index = 0;
    bool should_play_completion = false;

    for (;;) { // Loop indefinitely until completion
        // --- Check runComplete flag --- 
        osMutexAcquire(robot_state_mutex, osWaitForever);
        should_play_completion = runComplete; // Check if run is complete
        osMutexRelease(robot_state_mutex);

        if (should_play_completion) {
            break; // Exit the continuous song loop
        }

        // --- Play the current note of Melody 1 --- 
        int frequency = melody1_notes[current_note_index];
        int duration = melody1_durations[current_note_index];

        if (frequency > 0) {
            initPWM(frequency); // Set the note frequency
        } else {
            // Stop PWM for rests (frequency 0)
            TPM1->SC &= ~TPM_SC_CMOD_MASK; // Disable TPM1 counter
        }
        
        // Delay for the note's duration
        if (duration > 0) {
            osDelay(duration); 
        }
        
        // Move to the next note, wrap around
        current_note_index = (current_note_index + 1) % melody1_numNotes;
    }

    // --- Play Completion Tone (Melody 2) --- 
    // Stop previous sound first
    TPM1->SC &= ~TPM_SC_CMOD_MASK;
    osDelay(50); // Small pause

    for (int i = 0; i < melody2_numNotes; i++) {
        int frequency = melody2_notes[i];
        int duration = melody2_durations[i];

        if (frequency > 0) {
            initPWM(frequency);
        } else {
            TPM1->SC &= ~TPM_SC_CMOD_MASK;
        }

        if (duration > 0) {
            osDelay(duration);
        }
    }
    
    // Stop PWM after completion tone
    TPM1->SC &= ~TPM_SC_CMOD_MASK;
    
    // Thread can now optionally terminate or wait indefinitely
    // osThreadTerminate(osThreadGetId()); 
    for(;;) { osDelay(osWaitForever); } // Wait indefinitely
}
