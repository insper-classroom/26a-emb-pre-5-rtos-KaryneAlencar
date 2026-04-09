#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_g;

QueueHandle_t xQueueBtn;

void btn_callback(uint gpio, uint32_t events) {
  int btn;
  if(events == 0x4){
    if(gpio == BTN_PIN_G){
      btn = 1;
    }
    if(gpio == BTN_PIN_R){
      btn = 2;
    }
  }
  xQueueSendFromISR(xQueueBtn, &btn, 0);
}

void led_1_task(void *p) {
  gpio_init(LED_PIN_R);
  gpio_set_dir(LED_PIN_R, GPIO_OUT);

  int delay = 250;

  while (true) {

    if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_R, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_R, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

void btn_1_task(void *p) {
  gpio_init(BTN_PIN_R);
  gpio_set_dir(BTN_PIN_R, GPIO_IN);
  gpio_pull_up(BTN_PIN_R);

  int btn;
  while (true) {
    if (xQueueReceive( xQueueBtn, &btn, pdMS_TO_TICKS(100))){
      if(btn == 2){
        xSemaphoreGive(xSemaphore_r);
      }
    }
  }
}

void led_2_task(void *p) {
  gpio_init(LED_PIN_G);
  gpio_set_dir(LED_PIN_G, GPIO_OUT);

  int delay = 250;

  while (true) {

    if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
      gpio_put(LED_PIN_G, 1);
      vTaskDelay(pdMS_TO_TICKS(delay));
      gpio_put(LED_PIN_G, 0);
      vTaskDelay(pdMS_TO_TICKS(delay));
    }
  }
}

void btn_2_task(void *p) {
  gpio_init(BTN_PIN_G);
  gpio_set_dir(BTN_PIN_G, GPIO_IN);
  gpio_pull_up(BTN_PIN_G);

  int btn;
  while (true) {
    if (xQueueReceive( xQueueBtn, &btn, pdMS_TO_TICKS(100))){
      if(btn == 1){
        xSemaphoreGive(xSemaphore_g);
      }
    }
  }
}

int main() {
  stdio_init_all();
  printf("Start RTOS \n");

  xSemaphore_r = xSemaphoreCreateBinary();
  xSemaphore_g = xSemaphoreCreateBinary();

  xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
  xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

  xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
  xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

  xQueueBtn = xQueueCreate(32, sizeof(int) );

  vTaskStartScheduler();

  while (true)
    ;
}
