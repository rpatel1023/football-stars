// Sound.h
// Runs on TM4C123 or LM4F120
// Prototypes for basic functions to play sounds from the
// original Space Invaders.
// Jonathan Valvano
// November 17, 2014
#ifndef __SOUND_H__ // do not include more than once
#define __SOUND_H__
#include <stdint.h>

void Timer_Handler(void);
void Sound_Init(void);
void Sound_Play(const uint8_t *pt, uint32_t count);
void Sound_Throw(void);
void Sound_Fans(void);
void Sound_Caught(void);
void Sound_Intro(void);
void Sound_Touchdown(void);

#endif


