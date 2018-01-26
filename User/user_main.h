#include "stm32f7xx_hal.h"
#include "cmsis_os.h"

#define LEDS_TOGGLE_ALL		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3)
#define LEDS_OFF_ALL			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_PIN_RESET)
#define LEDS_ON_1					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET)
#define LEDS_ON_2					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET)
#define LEDS_ON_3					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET)

#define TRIAC_ON					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET)
#define TRIAC_OFF					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET)

#define POWER_ON					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_SET);	osDelay(33); HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_RESET)
#define POWER_OFF					TRIAC_OFF; HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_SET); osDelay(33); HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_RESET)

#define SIGNAL_SD_SET			HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_SET)
#define SIGNAL_SD_RESET		HAL_GPIO_WritePin(GPIOE,GPIO_PIN_6,GPIO_PIN_RESET)
#define SIGNAL_SD_READ		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)

#define SIGNAL_PWM_READ		HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)
