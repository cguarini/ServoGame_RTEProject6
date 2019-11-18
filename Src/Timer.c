#include "Timer.h"

#include <string.h>
#include <stdio.h>

#define MAX_POS (10.0)
#define MIN_POS (2.0)
char buf[100];

int TIM3_TIF = 0;

int getTIM3_TIF(){
  return TIM3_TIF;
}

void resetTIM3_TIF(){
  TIM3_TIF = 0;
}

void setDuty(int channel, float duty){
	
	//Safety, make sure we don't set the duty cycle too high
	if( duty < 0){
		duty = 0;
	}
	else if( duty > 5){
		duty = 5;
	}
	
  float dutyCycle = (((MAX_POS - MIN_POS) / 5.0) * (duty)) + MIN_POS;
  
	if(channel == 0){
		//set duty cycle
		TIM2->CCR1 = 20000 * (dutyCycle / 100.0);
	}
  if(channel == 1){
    TIM2->CCR2 = 20000 * (dutyCycle / 100.0);
  }
}

void initTIM2(){
    //TODO Enable TIM2 Clock
    RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);
      
    //Disable Timer
    TIM2->CCER &= (~TIM_CCER_CC1E);
  
    //Load Prescaler
    TIM2->PSC = 79;
    //Set PWM Period
    TIM2->ARR = 20000;
    //Set duty cycle for both channels
    setDuty(0, 0);
    setDuty(1, 0);
    TIM2->RCR = 0;
  
    TIM2->CR1 |= TIM_CR1_ARPE;
    //Generate event
    TIM2->EGR |= TIM_EGR_UG;
    
    //Setup CC1
    //Enable output on CC1
    TIM2->CCMR1 &= (~TIM_CCMR1_CC1S);
    TIM2->CCMR1 &= (~TIM_CCMR1_OC1M);
    TIM2->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
    //Select edge of active transition
    TIM2->CCER &= (~TIM_CCER_CC1NP);
    TIM2->CCER &= (~TIM_CCER_CC1P);
    
    //Setup CC2
    //Enable output on CC2
    TIM2->CCMR1 &= (~TIM_CCMR1_CC2S);
    TIM2->CCMR1 &= (~TIM_CCMR1_OC2M);
    TIM2->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2);
    //Select edge of active transition
    TIM2->CCER &= (~TIM_CCER_CC2NP);
    TIM2->CCER &= (~TIM_CCER_CC2P);
        
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;  
    //enable timer
    TIM2->CCER |= (TIM_CCER_CC1E);
    TIM2->CCER |= (TIM_CCER_CC2E);

    
    TIM2->CR1 |= TIM_CR1_CEN;
 
}

void initTIM3(){
    //TODO Enable TIM2 Clock
    RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM3EN);
      
    //Disable Timer
    TIM3->CCER &= (~TIM_CCER_CC1E);
  
    //Load Prescaler
    TIM3->PSC = 65535;
    //Set PWM Period
    TIM3->ARR = 123;

    TIM3->RCR = 0;
    TIM3->SR &= (~TIM_SR_TIF);
  
    TIM3->CR1 |= TIM_CR1_ARPE;
    TIM3->DIER |= TIM_DIER_TIE;
    TIM3->DIER |= TIM_DIER_UIE;
    //Generate event
    TIM3->EGR |= TIM_EGR_UG;
  
    NVIC_EnableIRQ(TIM3_IRQn);
    
        
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;  
    //enable timer
    TIM3->CCER |= (TIM_CCER_CC1E);
    TIM3->CCER |= (TIM_CCER_CC2E);

    
    TIM3->CR1 |= TIM_CR1_CEN;
 
}


uint32_t getCCR1(){
    return TIM2->CCR1;
}

uint32_t getCNT(){
    TIM2->SR &= (~TIM_SR_CC1IF);
    return TIM2->CNT;
}

void TIM3_IRQHandler(void){
  TIM3_TIF = 1;
  TIM3->SR &= (~TIM_SR_UIF);
}
