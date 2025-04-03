#include "MKL25Z4.h"  // Device header
#include "audio.h"

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

void delay_ms(uint32_t ms) {
    // Simple blocking delay loop (not power efficient).
    for (volatile uint32_t i = 0; i < (ms * 4000); i++);
}

// Melody 1: Mary Had a Little Lamb
// Note frequencies (Hz): C=262, D=294, E=330, G=392
void playtune_melody1(void) {
    int melody[] = {
        330, 294, 262, 294, 330, 330, 330,
        294, 294, 294,
        330, 392, 392,
        330, 294, 262, 294, 330, 330, 330
    };
    int numNotes = sizeof(melody) / sizeof(melody[0]);
    int noteDuration = 500; // 500ms per note
    
    for (int i = 0; i < numNotes; i++) {
        initPWM(melody[i]);
        delay_ms(noteDuration);
    }
    // Stop PWM after the melody finishes
    TPM1->SC &= ~TPM_SC_CMOD_MASK; // Disable TPM1 counter
}

// Melody 2: Twinkle Twinkle Little Star (example melody)
void playtune_melody2(void) {
    int melody[] = {
        262, 262, 392, 392, 440, 440, 392, 
        349, 349, 330, 330, 294, 294, 262
    };
    int numNotes = sizeof(melody) / sizeof(melody[0]);
    int noteDuration = 500; // 500ms per note

    for (int i = 0; i < numNotes; i++) {
        initPWM(melody[i]);
        delay_ms(noteDuration);
    }
    TPM1->SC &= ~TPM_SC_CMOD_MASK; // Disable TPM1 counter
}


// --- Audio Thread Function ---
// Original:
// void audio_thread(void *argument) {
//     playtune_melody1();
//     // Thread will terminate after playing the melody once in this test setup.
// }

// Modified:
void audio_thread(void *argument) {
    for (;;) {
        if (runComplete) {
            playtune_melody2();
        } else {
            playtune_melody1();
        }
        osDelay(1000); // Optional delay between plays
    }
}



//void audio_thread(void *argument) {
//    // Initialize PWM once before starting the loop (optional, could be done in main)
//    // initPWM(1000); // Initialize with a default frequency or handle inside playtune

//	playtune_melody1();
//    // --- Play Super Mario theme ---
////    playtune_supermario(); // Play Super Mario theme once

//    // --- Original loop (commented out) ---
//    /*
//    for (;;) {
//        // Example: Play Super Mario theme repeatedly with a delay
//        playtune_supermario();
//        osDelay(5000); // Wait 5 seconds before playing again
//        // Add logic here to decide when/what to play based on robot state or events
//    }
//    */
//   // Thread will terminate after playing the melody once in this test setup.
//   // For continuous operation based on state, the loop below should be used.
//}


// Melody 3: Super Mario Bros Theme (simplified excerpt)
// Note frequencies (Hz):
// E7 ~2640, C7 ~2096, G7 ~3136, G6 ~1568
// A zero value indicates a rest (pause).
void playtune_supermario(void) {
    int melody[] = {
        // Main theme intro
        2640, 0, 2640, 0, 0, 2640, 0, 0, 2096, 0, 2640, 0, 0, 3136, 0, 0, 0, 0,
        
        // Main theme part 1
        1568, 0, 0, 0, 1046, 0, 0, 1568, 0, 0, 2093, 0, 0, 1760, 0, 1568, 0, 0, 1319, 0, 
        1046, 0, 0, 0, 1319, 0, 0, 1760, 0, 0, 2093, 0, 0, 3136, 0, 0, 3520, 0, 0, 2793, 0, 2637, 0, 0,
        
        // Main theme part 2
        2093, 0, 0, 0, 2793, 0, 2637, 0, 0, 2093, 0, 0, 0, 1760, 0, 0, 1976, 0, 0, 1865, 0, 
        1760, 0, 0, 0, 1568, 0, 2093, 0, 0, 2640, 0, 0, 3136, 0, 0, 3520, 0, 0,
        
        // Underground theme
        1319, 0, 1568, 0, 2093, 0, 1319, 0, 1568, 0, 2093, 0, 0, 0,
        1319, 0, 1568, 0, 2093, 0, 1319, 0, 1568, 0, 2093, 0, 0, 0,
        1175, 0, 1397, 0, 1865, 0, 1175, 0, 1397, 0, 1865, 0, 0, 0,
        1175, 0, 1397, 0, 1865, 0, 1175, 0, 1397, 0, 1865, 0, 0, 0,
        
        // Starman invincibility theme
        2093, 0, 2093, 0, 2093, 0, 1568, 0, 2093, 0, 2637, 0, 0, 0,
        2093, 0, 2093, 0, 2093, 0, 1568, 0, 2093, 0, 0, 0,
        2349, 0, 2349, 0, 2349, 0, 1760, 0, 2349, 0, 2793, 0, 0, 0,
        2349, 0, 2349, 0, 2349, 0, 1760, 0, 2349, 0, 0, 0,
        
        // Death sound
        2093, 0, 0, 0, 0, 0, 1319, 0, 0, 0, 0, 0, 1046, 0, 0, 0, 0, 0,
        
        // Level complete fanfare
        1318, 0, 1567, 0, 2093, 0, 2093, 0, 2093, 0, 2093, 0, 
        2093, 0, 1567, 0, 1318, 0, 1046, 0, 1046, 0, 1046, 0, 
        1046, 0, 1318, 0, 1567, 0, 2093, 0, 2093, 0, 2093, 0, 
        2794, 0, 2794, 0, 2794, 0, 3136, 0, 0, 0, 0, 0
    };
    
    int durations[] = {
        // Main theme intro
        100, 25, 100, 25, 100, 100, 25, 100, 100, 25, 100, 25, 100, 100, 25, 100, 200, 100,
        
        // Main theme part 1
        200, 25, 175, 100, 200, 25, 100, 200, 25, 100, 150, 25, 100, 150, 25, 150, 25, 100, 150, 25, 
        300, 25, 175, 100, 150, 25, 100, 150, 25, 100, 150, 25, 100, 150, 25, 100, 150, 25, 100, 150, 25, 150, 25, 200,
        
        // Main theme part 2
        200, 25, 175, 100, 150, 25, 150, 25, 100, 200, 25, 175, 100, 150, 25, 100, 150, 25, 100, 150, 25,
        300, 25, 175, 100, 150, 25, 150, 25, 100, 150, 25, 100, 150, 25, 100, 150, 25, 300,
        
        // Underground theme
        100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 100, 25, 200, 100,
        
        // Starman invincibility theme
        100, 25, 100, 25, 100, 25, 100, 25, 150, 25, 150, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 150, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 150, 25, 150, 25, 200, 100,
        100, 25, 100, 25, 100, 25, 100, 25, 150, 25, 200, 100,
        
        // Death sound
        100, 25, 50, 50, 50, 50, 100, 25, 50, 50, 50, 50, 100, 25, 50, 50, 50, 200,
        
        // Level complete fanfare
        150, 25, 150, 25, 150, 25, 150, 25, 150, 25, 150, 25,
        150, 25, 150, 25, 150, 25, 150, 25, 150, 25, 150, 25,
        150, 25, 150, 25, 150, 25, 150, 25, 150, 25, 150, 25,
        150, 25, 150, 25, 150, 25, 300, 25, 100, 100, 100, 100
    };

    int numNotes = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < numNotes; i++) {
        // Lock the mutex before accessing the PWM hardware.
        if (melody[i] != 0) {
            initPWM(melody[i]);
        }
        // Release the mutex immediately after setting up the note.
    
        
        // Delay for the note's specified duration.
        delay_ms(durations[i]);
        
         // Disable PWM channel output if the note frequency is 0 (rest)
         if (melody[i] == 0) {
              TPM1_C0SC &= ~(TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK); // Disconnect channel output
         }
     }
    // Stop PWM after the melody finishes
    TPM1->SC &= ~TPM_SC_CMOD_MASK; // Disable TPM1 counter
}
