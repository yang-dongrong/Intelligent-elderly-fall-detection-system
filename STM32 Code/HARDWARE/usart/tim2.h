#ifndef __TIM2_H
#define __TIM2_H	
#include "sys.h"
#include "uart.h"

extern u32 gpstime;
extern u32 HeartRatetime;
void NVIC_config(void);
void time2_config(void);
void TIM2_IRQHandler(void);

#endif

