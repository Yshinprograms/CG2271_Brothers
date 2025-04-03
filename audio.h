#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>
	
// Initialize PWM with a given frequency.
void initPWM(int frequency);

// Blocking delay function (in milliseconds).
void delay_ms(uint32_t ms);

// Melody functions.
void playtune_melody1(void);    // Example: "Mary Had a Little Lamb"
void playtune_melody2(void);
// Extern flag to indicate run completion
extern volatile bool runComplete;


void playtune_supermario(void); // Super Mario Bros theme excerpt

// Audio thread function declaration
void audio_thread(void *argument);

#endif // AUDIO_H
