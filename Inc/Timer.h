#include "stm32l476xx.h"

void initTIM2();
unsigned int getCNT();
unsigned int getCCR1();
void setDuty(int channel, float duty);
void initTIM3();
int getTIM3_TIF();
void resetTIM3_TIF();
uint32_t getDutyCycle(uint8_t servo);
