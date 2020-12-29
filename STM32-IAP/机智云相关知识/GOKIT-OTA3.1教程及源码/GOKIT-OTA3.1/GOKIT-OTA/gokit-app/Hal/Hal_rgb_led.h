#ifndef _HAL_RGB_LED_H
#define _HAL_RGB_LED_H

#include <stdio.h>
#include <stm32f1xx.h>
#include <stdio.h>

/*兼容V2.2,RGB开关IO*/
#define GPIO_RGB_CLK    RCC_APB2Periph_GPIOA
#define GPIO_RGB_PORT   GPIOA
#define GPIO_RGB_PIN    GPIO_PIN_0

#define  R_MAX  255
#define  G_MAX  255
#define  B_MAX  255
									
#define SCL_LOW 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define SCL_HIGH 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)

#define SDA_LOW		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)
#define SDA_HIGH	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)

void rgbLedInit(void);
void ledRgbControl(uint8_t R,uint8_t B,uint8_t G);
void rgbKeyGpioInit(void);

//void LED_R_Control(uint8_t R);
//void LED_G_Control(uint8_t G);
//void LED_B_Control(uint8_t B);

#endif /*_HAL_RGB_LED_H*/

