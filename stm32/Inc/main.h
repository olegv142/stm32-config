#pragma once

#include "stm32f4xx_hal.h"

extern IWDG_HandleTypeDef hiwdg;

#define LED_PORT GPIOC
#define LED_PIN GPIO_PIN_12

void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);
