#include "audio.h"
#include "variables.c"
#include "MKL25Z4.h"
#include "cmsis_os2.h"
#include <stdbool.h>

// Frequencies for musical notes
#define A4 440
#define Bflat4 466
#define B4 494
#define C5 523
#define Csharp5 554
#define D5 587
#define Dsharp5 622
#define E5 659
#define F5 698
#define Fsharp5 740
#define G5 784
#define Aflat5 831
#define A5 880
#define Bflat5 932
#define B5 988
#define C6 1047

// Note durations in milliseconds
#define semi_note 250
#define one_note 500
#define two_note 1000

// External flag to allow early exit from playing a song
extern volatile bool runComplete;

/* Initialize PWM for audio output */
void initAudio(void) {
    // Enable Clock to PortA
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
    
    // Configure MUX to enable PWM for PortA
    PORTA->PCR[BUZZER] &= ~PORT_PCR_MUX_MASK;  //which is 
    PORTA->PCR[BUZZER] |= PORT_PCR_MUX(3);
    
    SIM_SCGC6 |= SIM_SCGC6_TPM1_MASK;
    
    SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
    TPM1->MOD = 7499; // 50Hz
    
    TPM1->SC &= ~(TPM_SC_CMOD_MASK | TPM_SC_PS_MASK);
    TPM1->SC |= TPM_SC_CMOD(1) | TPM_SC_PS(7);
    TPM1->SC &= ~(TPM_SC_CPWMS_MASK);
    
    TPM1_C0SC &= ~(TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA_MASK |
                   TPM_CnSC_MSB_MASK | TPM_CnSC_MSA_MASK);
    TPM1_C0SC |= TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1);
    
    TPM1_C0V = 3750; // 50% duty cycle
}

/* Set the tone frequency for PWM sound output */
void setTone(uint32_t frequency) {
    uint32_t mod = (48000000 / (frequency * 128)) - 1;
    TPM1->MOD = mod;
    TPM1_C0V = mod / 2;
    TPM1->SC |= TPM_SC_CMOD(1); // Update the counter
}

/* 
   Normal Run Song
   This song is divided into several segments:
     - Segments 1 & 2: A repeating pattern with E5
     - Segment 3: A rising pattern ending on a double-long E5
     - Segments 4-7: A mix of F5, E5, G5, D5, and a final C5
*/
void playTune(void) {
    uint32_t runMelody[] = {
        // Segment 1
        E5, E5, E5,
        // Segment 2
        E5, E5, E5,
        // Segment 3
        E5, G5, C5, D5, E5,
        // Segment 4
        F5, F5, F5, F5,
        // Segment 5
        F5, E5, E5, E5, E5,
        // Segment 6
        G5, G5, F5, D5,
        // Segment 7
        C5
    };
    
    uint32_t runDuration[] = {
        // Segment 1 durations
        one_note, one_note, two_note,
        // Segment 2 durations
        one_note, one_note, two_note,
        // Segment 3 durations
        one_note, one_note, one_note, one_note, (two_note * 2),
        // Segment 4 durations
        one_note, one_note, one_note, one_note,
        // Segment 5 durations
        one_note, one_note, one_note, semi_note, semi_note,
        // Segment 6 durations
        one_note, one_note, one_note, one_note,
        // Segment 7 duration
        (two_note * 2)
    };
    
    int numNotes = sizeof(runMelody) / sizeof(runMelody[0]);
    
    for (int i = 0; i < numNotes; i++) {
        // if (runComplete) return;
        setTone(runMelody[i]);
        osDelay(runDuration[i]);
    }
}

void playCelebratoryTune(void) {
    uint32_t celebratoryMelody[] = {
        // Segment 1
        E5, B5, A5, B5, C6, C6, B5, G5, G5,
        // Segment 2
        A5, A5, A5, G5, E5, E5, E5, E5, D5,
        // Segment 3
        E5, B5, A5, B5, C6, C6, B5, G5, G5,
        // Segment 4
        A5, A5, A5, G5, E5, E5
    };
    
    uint32_t celebratoryDuration[] = {
        // Segment 1 durations
        one_note, one_note, semi_note, semi_note, one_note, semi_note, one_note, semi_note, one_note,
        // Segment 2 durations
        one_note, one_note, semi_note, (semi_note * 3), one_note, semi_note, semi_note, semi_note, (semi_note * 3),
        // Segment 3 durations
        one_note, one_note, semi_note, semi_note, one_note, semi_note, one_note, semi_note, one_note,
        // Segment 4 durations
        one_note, one_note, semi_note, (semi_note * 3), (one_note * 2), (one_note * 2)
    };
    
    int numNotes = sizeof(celebratoryMelody) / sizeof(celebratoryMelody[0]);
    
    for (int i = 0; i < numNotes; i++) {
        // if (runComplete) return;
        setTone(celebratoryMelody[i]);
        osDelay(celebratoryDuration[i]);
    }
}



void audio_thread(void *argument) {
    // Continuously play the run song until the challenge is complete.
    while (!runComplete) {
         playTune();
    }
    // When the challenge run is complete, play the unique end tone.
    playCelebratoryTune();

    
    // Optionally, suspend the thread after finishing.
    for (;;) {
        osDelay(1000);
    }
}