#include "GPIOINP.h"


void initGPIOA(){
  // Enable the  clock for GPIO(Port A )
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);

  //init PA0
  // Configure Pin 0 port A as input mode (Pg:278) 00: Alternate Function mode
  GPIOA->MODER &= ~(GPIO_MODER_MODER0);

  GPIOA->MODER |= (2);


  // Alternate function register low due to Pin 0:
  GPIOA->AFR[0] &= (~GPIO_AFRL_AFRL0);
  GPIOA->AFR[0] |= (GPIO_AFRL_AFRL0 & 1);

  //init PA1
  // Configure Pin 1 port A as input mode (Pg:278) 00: Alternate Function mode
  GPIOA->MODER &= ~(GPIO_MODER_MODER1);

  GPIOA->MODER |= (2 << 2);


  // Alternate function register low due to Pin 0:
  GPIOA->AFR[0] &= (~GPIO_AFRL_AFRL1);
  GPIOA->AFR[0] |= (GPIO_AFRL_AFRL1 & (1 << 4));
  
    
}