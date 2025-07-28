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
uint32_t test_start_time = 0; /* Gets the time at which the test is started (expressed as microseconds from boot), which is the time in which the led is turned on */
uint32_t led_activation_time = 0; /* This variable represents the time that the led will wait before turning on, its value will be generated randomly each execution (expressed as microseconds from boot) */
uint32_t reaction_time = 0; /* This variable represents the time taken by the user to press the button after the led has been turned on (expressed as microseconds from boot) */
uint32_t best_reaction_time = UINT32_MAX; /* This variable represents the best time, it's needed mainly to pilot the buzzer */

/* Concurrency semaphores */
SemaphoreHandle_t led_semaphore; /* Semaphore that handles the led status' toggle */
SemaphoreHandle_t buzzer_semaphore; /* Semaphore that makes the buzzer wait for the click over the button to perform its checks */
SemaphoreHandle_t publisher_semaphore; /* Semaphore that makes the publisher wait for the button to be clicked */

/* Task handlers */
TaskHandle_t ledTaskHandler = NULL;
TaskHandle_t buzzerTaskHandler = NULL;
TaskHandle_t rosPubTaskHandler = NULL;
TaskHandle_t rosSubTaskHandler = NULL;

/*
---------- Function Prototypes ----------
*/
void led_task(void *pvParameters);
void buzzer_task(void *pvParameters);
void ros_publisher_task(void *pvParameters);
void ros_subscription_task(void *pvParameters);
void button_callback(void *pvParameters);

/*
---------- FreeRTOS TASKS DEFINITION + Callback functions definition ----------
*/
/* Task that handles the led status */
void led_task(void *pvParameters){
    while(1){
        /* Generating the random interval time between 1 and 3 seconds */
        led_activation_time = (rand() % 3000) + 1000;
        while(led_activation_time > 0){
            vTaskDelay(pdMS_TO_TICKS(1));
            led_activation_time--;
        }
        /* Enable the interrupts over the button to detect the future click */
        gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
        /* Here the led is turned on and the start time is saved in the respective variable */
        gpio_put(LED_PIN, 1);
        test_start_time = to_us_since_boot(get_absolute_time());
        /* Now wait over the semaphore that the user presses the button */
        xSemaphoreTake(led_semaphore, portMAX_DELAY);
        /* As soon as the button is pressed turn the led off and notify the microros publisher by signaling its semaphore */
        gpio_put(LED_PIN, 0);
        xSemaphoreGive(publisher_semaphore);
    }
}

/* Task that manages the buzzer, making it sound each time the best record is beaten */
void buzzer_task(void *pvParameters){

}

/* Task that handles the publishing over microROS topics of both reaction time and best time */
void ros_publisher_task(void *pvParameters){

}

/* Task that handles the reading from the "best times" microROS topic, it is executed just once to get the best score during the first execution */
void ros_subscription_task(void *pvParameters){ 

}


/* Callback function that manages the click over the button */
void button_callback(void *pvParameters){

}


/*
---------- MAIN ----------
*/
int main(){
    stdio_init_all();

    /* Initializing the led (which will initially be off) */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    /* Initializing the button (using the internal pull-up resistor) */
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    /* Initializing the buzzer (which, like the led, will initially be off) */
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    /* Initializing the semaphores (ensuring they will be set to 0 so that they will block the task until further notice) */
    led_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(led_semaphore, portMAX_DELAY);
    buzzer_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(buzzer_semaphore, portMAX_DELAY);
    publisher_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(publisher_semaphore, portMAX_DELAY);

    /* FreeRTOS tasks creation */
    xTaskCreate(led_task, "LED Task", 256, NULL, 1, &ledTaskHandler);
    xTaskCreate(buzzer_task, "Buzzer Task", 256, NULL, 1, &buzzerTaskHandler);
    xTaskCreate(ros_publisher_task, "ROS Pub Task", 256, NULL, 1, &rosPubTaskHandler);
    xTaskCreate(ros_subscription_task, "ROS Sub Task", 256, NULL, 1, &rosSubTaskHandler);

    vTaskStartScheduler();
    while(1){}
    return 0;
}