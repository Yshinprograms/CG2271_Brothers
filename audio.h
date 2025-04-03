#ifndef AUDIO_H  
#define AUDIO_H  

#define BUZZER  13  // PTA13

void initAudio (void);

void setTone (uint32_t frequency);

void playTune (void);
void playCelebratoryTune (void);

#endif
