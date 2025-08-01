#include "pico/stdlib.h"
#include "pico/types.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "unistd.h"
#include "pico_uart_transports.h"

/* microROS header files */
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rcl/error_handling.h>
#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/int32.h>

/* I/O pins binding */
#define LED_PIN 16
#define BUZZER_PIN 19
#define BUTTON_PIN 15

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

/* Global variables */
uint32_t test_start_time = 0; /* Gets the time at which the test is started (expressed as microseconds from boot), which is the time in which the led is turned on */
uint32_t led_activation_time = 0; /* This variable represents the time that the led will wait before turning on, its value will be generated randomly each execution (expressed as microseconds from boot) */
uint32_t reaction_time = 0; /* This variable represents the time taken by the user to press the button after the led has been turned on (expressed as microseconds from boot) */
uint32_t best_reaction_time = UINT32_MAX; /* This variable represents the best time, it's needed mainly to pilot the buzzer */

/* Concurrency semaphores */
SemaphoreHandle_t led_semaphore; /* Semaphore that handles the led status' toggle */
uint32_t led_blocked = 0;
SemaphoreHandle_t buzzer_semaphore; /* Semaphore that makes the buzzer wait for the click over the button to perform its checks */
uint32_t buzzer_blocked = 1;
SemaphoreHandle_t publisher_semaphore; /* Semaphore that makes the publisher wait for the button to be clicked */
uint32_t publisher_blocked = 1;

/* Task handlers */
TaskHandle_t ledTaskHandler = NULL;
TaskHandle_t buzzerTaskHandler = NULL;
TaskHandle_t rosPubTaskHandler = NULL;
TaskHandle_t rosSubTaskHandler = NULL;

/* microROS entities and message formats */
rcl_publisher_t time_publisher;
rcl_publisher_t best_time_publisher;
rcl_subscription_t subscriber;
std_msgs__msg__Int32 reaction_time_to_upload; /* This will hold the reaction time measured each time */
std_msgs__msg__Int32 best_reaction_time_to_upload; /* This will hold the best reaction time measured if beaten */
std_msgs__msg__Int32 best_reaction_time_to_read; /* This will hold the best reaction time to be read from microROS */

/*
---------- Function Prototypes ----------
*/
void led_task(void *pvParameters);
void buzzer_task(void *pvParameters);
void ros_publisher_task(void *pvParameters);
void micro_ros_task(void *arg);
void subscription_callback(const void *msgin);
void button_callback(uint gpio, uint32_t events);
void sleep_ms_rt(uint32_t ms);
void play_buzzer(uint duration);
void pwm_buzzer_init();

/*
---------- FreeRTOS TASKS DEFINITION ----------
*/
/* Task that handles the led status */
void led_task(void *pvParameters){
    while(1){
        /* Generating the random interval time between 1 and 3 seconds */
        uint32_t led_activation_time = (rand() % 3000) + 1000;
        sleep_ms_rt(led_activation_time);
        /* Enable the interrupts over the button to detect the future click */
        gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true);
        /* Here the led is turned on and the start time is saved in the respective variable */
        gpio_put(LED_PIN, 1);
        test_start_time = to_us_since_boot(get_absolute_time());
        led_blocked = 1;
        /* Now wait over the semaphore that the user presses the button */
        xSemaphoreTake(led_semaphore, portMAX_DELAY);
    }
}

/* Task that manages the buzzer, making it sound each time the best record is beaten */
void buzzer_task(void *pvParameters){
    while(1){
        /* Wait for a "new record" event to occur */
        buzzer_blocked = 1;
        xSemaphoreTake(buzzer_semaphore, portMAX_DELAY);
        /* Make the buzzer sound for 1,5 seconds */
        for(int i = 0; i < 1500; i++){
            play_buzzer(50);
            sleep_ms_rt(25);
        }
        /* Re-awaken the led task */
        gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, false);
        if(led_blocked){
            led_blocked = 0;
            xSemaphoreGive(led_semaphore);
        }
    }
}

/* Task that handles the publishing over microROS topics of both reaction time and best time */
void ros_publisher_task(void *pvParameters){
    while(1){
        publisher_blocked = 1;
        xSemaphoreTake(publisher_semaphore, portMAX_DELAY);
        /* Elaborate the value and send it to the respective microROS topic */
        reaction_time_to_upload.data = reaction_time;
        RCSOFTCHECK(rcl_publish(&time_publisher, &reaction_time_to_upload, NULL));
        /* Once the result has been published check if the result is the best one */
        gpio_set_irq_enabled(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, false);
        if(reaction_time_to_upload.data < best_reaction_time){
            best_reaction_time = reaction_time_to_upload.data;
            best_reaction_time_to_upload.data = best_reaction_time;
            RCSOFTCHECK(rcl_publish(&best_time_publisher, &best_reaction_time_to_upload, NULL));
            /* Awake the sleeping buzzer that will need to sound */
            if(buzzer_blocked){
                buzzer_blocked = 0;
                xSemaphoreGive(buzzer_semaphore);
            }
            /* The re-awakening of the led task will be performed in the buzzer function if it's called or in this task otherwise */
        } else{
            if(led_blocked){
                led_blocked = 0;
                xSemaphoreGive(led_semaphore);
            }
        }
    }
}

/*
---------- microROS TASKS DEFINITION ----------
*/

void micro_ros_task(void *arg){
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;

    /* Create init options */
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

    /* Create microROS node */
    rcl_node_t node;
    RCCHECK(rclc_node_init_default(&node, "reaction_tester_node", "", &support));

    /* Create subscriber */
    RCCHECK(rclc_subscription_init_default(
        &subscriber,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "best_reaction_time"
    ));

    /* Create reaction time publisher */
    RCCHECK(rclc_publisher_init_default(
        &time_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "reaction_time"
    ));

    /* Create best time publisher */
    RCCHECK(rclc_publisher_init_default(
        &best_time_publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "best_reaction_time"
    ));

    /* Create executor */
    rclc_executor_t executor;
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &best_reaction_time_to_read, &subscription_callback, ON_NEW_DATA));

    /* Spawn timings publisher thread */
    xTaskCreate(ros_publisher_task, "ROS Pub Task", 512, NULL, 1, &rosPubTaskHandler);

    /* Initialize published message (check if it's actually needed) */
    reaction_time_to_upload.data = 0;

    while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        sleep_ms_rt(10);
	}

    /* Free-ing resources after execution */
    RCCHECK(rcl_subscription_fini(&subscriber, &node));
    RCCHECK(rcl_publisher_fini(&time_publisher, &node));
    RCCHECK(rcl_publisher_fini(&best_time_publisher, &node));
    RCCHECK(rcl_node_fini(&node));
    vTaskDelete(NULL);
}

/*
---------- Callback functions definition ----------
*/

/* Task that handles the readings from the "best times" microROS topic */
void subscription_callback(const void *msgin){ 
    const std_msgs__msg__Int32 *msg = (const std_msgs__msg__Int32 *)msgin;
    best_reaction_time = msg->data;
}


/* Callback function that manages the click over the button */
void button_callback(uint gpio, uint32_t events){
    play_buzzer(100);
    if(gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)){
        /* Fetch the time of click and use it to calculate reaction time */
        uint32_t click_time = to_us_since_boot(get_absolute_time());
        reaction_time = click_time - test_start_time;
        /* Unlock the microROS publisher task (which will eventually unlock the led task) */
        if(publisher_blocked){
            publisher_blocked = 0;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(publisher_semaphore, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        /* As soon as the button is pressed turn the led off and disable interrupts on the button to avoid detecting other clicks */
        gpio_put(LED_PIN, 0);
    }
}

/*
---------- Auxiliary functions ----------
*/
void sleep_ms_rt(uint32_t ms) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    } else {
        sleep_ms(ms);
    }
}

void play_buzzer(uint duration){
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_gpio_level(BUZZER_PIN, 500);
    sleep_ms_rt(duration);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void pwm_buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);
    pwm_config_set_wrap(&config, 1000);
    pwm_init(slice, &config, true);
}

/*
---------- MAIN ----------
*/
void main(void){
    stdio_init_all();

    /* Initializing the led (which will initially be off) */
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    /* Initializing the button (using the internal pull-up resistor) */
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    /* Initializing the buzzer (which, like the led, will initially be off) */
    pwm_buzzer_init();
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    /* Initializing the semaphores (ensuring they will be set to 0 so that they will block the task until further notice) */
    led_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(led_semaphore, portMAX_DELAY);
    buzzer_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(buzzer_semaphore, portMAX_DELAY);
    publisher_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(publisher_semaphore, portMAX_DELAY);

    rmw_uros_set_custom_transport(
		true,
		NULL,
		pico_serial_transport_open,
		pico_serial_transport_close,
		pico_serial_transport_write,
		pico_serial_transport_read
	);

    /* FreeRTOS tasks creation */
    xTaskCreate(led_task, "LED Task", 256, NULL, 1, &ledTaskHandler);
    xTaskCreate(buzzer_task, "Buzzer Task", 256, NULL, 1, &buzzerTaskHandler);
    xTaskCreate(micro_ros_task, "microROS Task", 5000, NULL, 1, &rosSubTaskHandler);
    
    vTaskStartScheduler();
    while(1){}
}