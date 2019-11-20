#include "stm32l476xx.h"
#include "cmsis_os.h"

#define POS_0 (0)
#define POS_1 (45)
#define FIVE_SECONDS (5000)


void moveServoRight(int servo, int amount);
void moveServoLeft(int servo, int amount);
void handleCommand(char * command);
void moveServo(int servo, int position);
int getServoState(int servo);

void servo_task_init(int id, char *task_name);
void player_servo_task_init(int id, char *task_name);

typedef struct {
  int id;
  TaskHandle_t handle;  // generated by FreeRTOS, saved for later use (task synchronization, etc.)
  char task_name[16];
} SERVO_PARAMS_t;
