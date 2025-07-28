#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* microROS header files */
#include "rcl/rcl.h"
#include "rclc/rclc.h"
#include "rclc/executor.h"
#include "std_msgs/msg/int32.h"

/* I/O pins binding */
#define LED_PIN 16
#define BUZZER_PIN 19
#define BUTTON_PIN 15

/* Global variables */
uint32_t led_activation_time = 0; /* This variable represents the time that the led will wait before turning on, its value will be generated randomly each execution */
uint32_t reaction_time = 0; /* This variable represents the time taken by the user to press the button after the led has been turned on */
uint32_t best_reaction_time = UINT32_MAX; /* This variable represents the best time, it's needed mainly to pilot the buzzer */

/* Concurrency semaphores */
SemaphoreHandle_t led_semaphore; /* Semaphore that handles the led status' toggle */
SemaphoreHandle_t buzzer_semaphore; /* Semaphore that makes the buzzer wait for the click over the button to perform its checks */
SemaphoreHandle_t publisher_semaphore; /* Semaphore that makes the publisher wait for the button to be clicked */

/*
---------- FreeRTOS TASKS DEFINITION ----------
*/
/* Task that handles the led status */
void led_task(void *pvParameters){

}

/* Task that manages the click over the button */
void button_task(void *pvParameters){

}

/* Task that manages the buzzer, making it sound each time the best record is beaten */
void buzzer_task(void *pvParameters){

}

/* Task that handles the publishing over microROS topics of both reaction time and best time */
void ros_publisher_task(void *pvParameters){

}

/* Task that handles the reading from the "best times" microROS topic */
void ros_subscription_task(void *pvParameters){ 

}

/*
---------- MAIN ----------
*/
int main(){
    return 0;
}