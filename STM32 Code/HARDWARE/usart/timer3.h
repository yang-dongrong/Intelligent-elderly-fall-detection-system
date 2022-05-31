#ifndef __TIMER3_H
#define __TIMER3_H
#include "sys.h"

void TIM3_Int_Init(u16 arr,u16 psc);
unsigned long millis(void);
extern unsigned long RunTime;
#endif
