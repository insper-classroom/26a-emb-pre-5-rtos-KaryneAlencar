/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueBtn;

SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;
//y = 2 r = 1
void btn_callback(uint gpio, uint32_t events) {
  int btn = 0;
  if(events == GPIO_IRQ_EDGE_FALL){
    if(gpio == BTN_PIN_Y){
      btn = 2;
    }
    if(gpio == BTN_PIN_R){
      btn = 1;
    }
  }
  if(btn != 0){
    xQueueSendFromISR(xQueueBtn, &btn, 0);
  }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int ligado = 0;
    
    while (true) {
        int btn;
        if(xSemaphoreTake(xSemaphoreLedR, pdMS_TO_TICKS(500)) == pdTRUE){
            ligado = !ligado;
        }
        if(!ligado){
            gpio_put(LED_PIN_R, 0);
        }
        if(ligado){
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    int ligado = 0;
    while (true) {
        int btn;
        if(xSemaphoreTake(xSemaphoreLedY, pdMS_TO_TICKS(500)) == pdTRUE){
            ligado = !ligado;
        }
        if(!ligado){
            gpio_put(LED_PIN_Y, 0);
        }
        if(ligado){
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void btn_task(void *p) {
    int btn;
    while (true) {
        if (xQueueReceive( xQueueBtn, &btn, pdMS_TO_TICKS(100))) {

            if(btn == 2){
                xSemaphoreGive(xSemaphoreLedY);
            }

            if(btn == 1){
                xSemaphoreGive(xSemaphoreLedR);
            }
            
        }
    }
}

int main() {
    stdio_init_all();

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    //printf("Start RTOS \n");
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xQueueBtn = xQueueCreate(32, sizeof(int));

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED2_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
