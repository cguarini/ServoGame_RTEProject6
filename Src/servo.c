#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "servo.h"
#include "Timer.h"
#include "recipeProcessor.h"
#include "rng.h"
#include "main.h"
#include "usart.h"

#define POSITION_FACTOR (1)
#define MOVEMENT_WAIT_FACTOR (2)

int servoState[2] = {0,0};//1 for recipe running, 0 for paused
int servoPosition[2] = {0,0};
SERVO_PARAMS_t servo_params[2];
extern int32_t gyro_angle[3] ;
uint32_t score = 0;



int getServoState(int servo){
	//default servo to servo 1 if out of bounds
	if(servo > 1 || servo < 0){
		servo = 0;
	}
	
	return servoState[servo];
}

/*
*Set the given servo to the given position
*@param servo, servo to move
*@param position, position to move the servo to (0-5)
*/
void moveServo(int servo, int position){
  
  //Keep the position in bounds (0 to 5)
  if(position < 0){
    position = 0;
  }
  if(position > 5){
    position = 5;
  }
  
  //Allow time for servo to move before next instruction commences
  if(servoPosition[servo] > position){
    addToWaitCounter(servo, MOVEMENT_WAIT_FACTOR * (servoPosition[servo] - position));
  }
  else{
    addToWaitCounter(servo, MOVEMENT_WAIT_FACTOR * (position - servoPosition[servo]));
  }
  
  //Set position
  servoPosition[servo] = position;
  setDuty(servo, position * POSITION_FACTOR);
  
}

/*
* Moves the given servo right by the given amount
*@param servo, servo to move
*@param amount, amount to move right
*/
void moveServoRight(int servo, int amount){
  
  //Move the servo right, maxing out at position 5
  servoPosition[servo] += amount;
  if(servoPosition[servo] > 5){
    servoPosition[servo] = 5;
  }
  
  //Set servo to position
  setDuty(servo, servoPosition[servo] * POSITION_FACTOR);
}

/*
* Moves the given servo left by the given amount
*@param servo, servo to move
*@param amount, amount to move left
*/
void moveServoLeft(int servo, int amount){
  
  //Move the servo right, maxing out at position 5
  servoPosition[servo] -= amount;
  if(servoPosition[servo] < 0){
    servoPosition[servo] = 0;
  }
  
  //Set servo to position
  setDuty(servo, servoPosition[servo] * POSITION_FACTOR);
}


/*
* Handles the commands input from the user. First character controls Servo0,
* second character controls Servo1
*/
void handleCommand(char * command){
  
  //loop through twice, once for each servo
	for(int i = 0; i < 2; i++){
    
    //Parse the character into the separate commands
    //Pause Recipe Execution
    if(command[i] == 'P' || command[i] == 'p'){
      servoState[i] = 0;
    }
    //Continue Recipie Execution
    if(command[i] == 'C' || command[i] == 'c'){
      servoState[i] = 1;
    }
    //Begin or restart recipe
    if(command[i] == 'B' || command[i] == 'b'){
      servoState[i] = 1;
      restartRecipe(i);
    }
    //Check if the servo is paused before allowing l or r commands
    if(!servoState[i]){
      //Move 1 position to the right
      if(command[i] == 'R' || command[i] == 'r'){
        moveServoRight(i, 1);
      }
      //Move 1 position to the left
      if(command[i] == 'L' || command[i] == 'l'){
        moveServoLeft(i, 1);
      }
    }
    //Otherwise, no-op
  }
}

uint32_t getGyroPostion(){
	
}


void servo_task(void *parameters){
	
	uint32_t servo_pos;
	SERVO_PARAMS_t * p = (SERVO_PARAMS_t *) parameters;
	
	
	//Generate random servo position
    HAL_RNG_GenerateRandomNumber(&hrng, &servo_pos);
	
	servo_pos = servo_pos % 6;
	USART_Printf("Servo is now at position %d\r\n", servo_pos);
	moveServo(p->id, servo_pos);
  uint32_t roundStartTime = xTaskGetTickCount();
	
	for(;;){
    
    //Get Duty Cycle of each servo
    uint32_t servoDutyCycle = getDutyCycle(0);
    uint32_t playerDutyCycle = getDutyCycle(1);
    
    float dutyCycleFactor = (float) servoDutyCycle / playerDutyCycle;
		
		if((dutyCycleFactor >= 1 && dutyCycleFactor < 1.05) || (dutyCycleFactor <= .05) ){
			HAL_RNG_GenerateRandomNumber(&hrng, &servo_pos);
			servo_pos = servo_pos % 6;
      
      //Check for repeat positions
      while(servo_pos == servoPosition[0]){
        HAL_RNG_GenerateRandomNumber(&hrng, &servo_pos);
        servo_pos = servo_pos % 6;
      }
      
      moveServo(p->id, servo_pos);
			score++;
			USART_Printf("Servo Hit! Score is now %d\r\n", score);
			USART_Printf("Servo is now at position %d\r\n", servo_pos);
      roundStartTime = xTaskGetTickCount();
		}
    else if(xTaskGetTickCount() > roundStartTime + FIVE_SECONDS){
      USART_Printf("Out of time! Score is still %d.\r\n", score);
      HAL_RNG_GenerateRandomNumber(&hrng, &servo_pos);
			servo_pos = servo_pos % 6;
      
      //Check for repeat positions
      while(servo_pos == servoPosition[0]){
       HAL_RNG_GenerateRandomNumber(&hrng, &servo_pos);
			servo_pos = servo_pos % 6;
      }
      
			moveServo(p->id, servo_pos);
      USART_Printf("Servo is now at position %d\r\n", servo_pos);
      roundStartTime = xTaskGetTickCount();
    }
		
		
		
		
		vTaskDelay(50);
	}
}

void player_servo_task(void * parameters){
	
	SERVO_PARAMS_t * p = (SERVO_PARAMS_t *) parameters;
	
	for(;;){
	
		int32_t angle = gyro_angle[1];
    
    float pos = (float) angle/POS_1;
    setDuty(1, pos);

		vTaskDelay(20);
	}
}

void player_servo_task_init(int id, char *task_name){
	SERVO_PARAMS_t *p = &servo_params[id];   // get pointer to THIS instance of parameters (one for each task)
	p->id = id;                           // initialize members of this structure for this task
	strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
	xTaskCreate( player_servo_task, task_name, 256, (void *)p, 2, &p->handle); // go ahead and create the task
}

void servo_task_init(int id, char *task_name){
	SERVO_PARAMS_t *p = &servo_params[id];   // get pointer to THIS instance of parameters (one for each task)
	p->id = id;                           // initialize members of this structure for this task
	strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
	xTaskCreate( servo_task, task_name, 256, (void *)p, 2, &p->handle); // go ahead and create the task
}

