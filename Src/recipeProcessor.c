#include "recipeProcessor.h"
#include "servo.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "string.h"



//Initilaizing the values
int waitArg[] = {0,0}; //Stores the wait value for each servo recipe
int waitCounter[] = {0,0};//How many more "cycles" the instruction must wait before execution
int programCounter[] = {0,0};//Stores the instruction index each servo is on
int loopAddress[] = {0,0};//Stores the address of the loop instruction
int loopCounter[] = {0,0};//Stores the loop count of each servo
int endFlag[] = {0,0};//Marks the end of recipe execution
//Marks that the program encountered an error
//0 - no error
//1 - command error
//2 - nested loop error
int errorFlag[] = {0,0};
//Task parameters
RECIPE_PARAMS_t recipe_params[2];


uint32_t recipe1[RECIPE_LIMIT] = {
MOV+0,
MOV+1,
MOV+5,
MOV+1,
MOV+3,
LOOP+1,
MOV+1 ,
MOV+4,
END_LOOP,
MOV+0,
MOV+2,
WAIT+0,
MOV+3,
WAIT+0,
MOV+2,
MOV+3,
WAIT+31,
WAIT+31,
WAIT+31,
MOV+1, 
RECIPE_END 
};


uint32_t recipe2[RECIPE_LIMIT] = {
MOV+3,
MOV+5,
MOV+2,
MOV+3,
LOOP+3,
MOV+1 ,
MOV+4,
END_LOOP,
MOV+0,
MOV+2,
WAIT+0,
MOV+3,
WAIT+0,
MOV+2,
MOV+3,
WAIT+31,
WAIT+31,
WAIT+31,
MOV+5, 
RECIPE_END 
};


uint32_t * recipePrograms[] = { recipe1, recipe2};

int getWait(int servo){
	
	//Default to 0 if out of bounds
	if(servo > 1 || servo < 0){
		servo = 0;
	}
  
  return waitArg[servo];
}

//Reinitialize the given servo
void restartRecipe(int servo){
  programCounter[servo] = 0;
  endFlag[servo] = 0;
  errorFlag[servo] = 0;
  waitCounter[servo] = 0;
  waitArg[servo] = 0;
  loopCounter[servo] = 0;
  loopAddress[servo] = 0;
}



int addToWaitCounter(int servo, int amount){
  waitCounter[servo] += amount;
  waitCounter[servo] += getWait(servo);
}

void executeLoop(int servo, int count, int address){
	//Set the loop address to branch back to
	loopAddress[servo] = address;
	//Set the loop counter
	loopCounter[servo] = count;
}

// To check the recipe command at each iteration 
void recipe_task(void *parameters)
{
  
  RECIPE_PARAMS_t * p = (RECIPE_PARAMS_t *) parameters;
  //Grab i from parameters, will be replacement of i from project 2 for loop
  int i = p->id;
	//Infinite task loop, function should never return
	for(;;) {
		
		//Check if the recipe is paused or not
		//AND check if the instruction has waited long enough before execution
		//AND check that the program is not in an error state
		if(getServoState(i) && !waitCounter[i] && !errorFlag[i]){
			//No errors, execute the program
			
			//Fetch the instruction
			int PC = programCounter[i];
			uint32_t * recipe = recipePrograms[i];
			uint32_t instruction = recipe[PC];
			
			//Decode and execute the instruction
			uint32_t parameter = instruction & PARAMETER_MASK;
			switch(instruction  & OPCODE_MASK){
				
				//Move the servo to the desired position
				case MOV:		
          if(parameter > 5){
            errorFlag[i] = 1;
          }
          else{
            moveServo(i, parameter);	
          }
					break;
          
         //DMOV moves the servo position twice

          case DM:		
          if(parameter > 3){
            errorFlag[i] = 1;
          }
          else{
            moveServo(i, parameter * 2);
          }
					break;
          
				//Set time between instructions, parameter * 100ms
				case DELAY: 
					waitArg[i] = parameter; // for getting the count in wait command
					break;
        
        //Wait the given amount of time before the next instruction
        case WAIT:
          waitCounter[i] += parameter;
          break;
					
				//Loop through the following instructions until END_LOOP the given amount
				case LOOP:
					//Check for nested loops
					if(loopCounter[i]){
						errorFlag[i] = 2;
					}
					executeLoop(i, parameter, PC);
					break;	

				//Marks the end of a loop function, either return to LOOP or exit loop
				case END_LOOP:
          loopCounter[i]--;
					if(loopCounter[i] > 0){
						programCounter[i] = loopAddress[i];
					}
					break;

				//End of recipe, do nothing
				case RECIPE_END:
					endFlag[i] = 1;
					break;
			}
			
			//If the recipe has not ended, increment the PC
			if(!endFlag[i]){
				programCounter[i]++;
			}
		}

		
		//Decrement or reset the wait cycles for this servo
    vTaskDelay(100);
		waitCounter[i]--;
		if(waitCounter[i] < 0){
			waitCounter[i] = getWait(i);
		}
	}
}


/*****************************************
teller_task_init() initializes the teller_params control block and creates a task.
inputs
  int id - 0 or 1, used to differentiate task instance
  char *name - a unique human readable name for the task
  int base_ms - a base amount of time to blink an teller
  int max_jitter_ms - max variability in the blink time
outputs
  none
*******************************************/
void recipe_task_init(int id, char *task_name)
{
  RECIPE_PARAMS_t *p = &recipe_params[id];   // get pointer to THIS instance of parameters (one for each task)
  p->id = id;                           // initialize members of this structure for this task
  strncpy(p->task_name, task_name, configMAX_TASK_NAME_LEN);
  xTaskCreate( recipe_task, task_name, 256, (void *)p, 2, &p->handle); // go ahead and create the task 
}




	
