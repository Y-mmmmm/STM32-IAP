#ifndef _GOKIT_H
#define _GOKIT_H

#include "stm32f10x.h"
#include "Hal_Flash\flash.h"
#include <string.h>
#include "stdio.h"

#define DEBUG
// #define PROTOCOL_DEBUG

/***************************************************************
*	UART1_TX   PA9 
*	UART1_RX   PA10
****************************************************************/
#define USART1_GPIO_Cmd		       RCC_APB2PeriphClockCmd
#define USART1_GPIO_CLK          RCC_APB2Periph_GPIOA

#define USART1_AFIO_Cmd		       RCC_APB2PeriphClockCmd
#define USART1_AFIO_CLK          RCC_APB2Periph_AFIO

#define USART1_CLK_Cmd		       RCC_APB2PeriphClockCmd
#define USART1_CLK               RCC_APB2Periph_USART1 

#define USART1_GPIO_PORT         GPIOA
#define USART1_RxPin             GPIO_Pin_10
#define USART1_TxPin             GPIO_Pin_9

/* 升级参数 */
#define PIECE_MAX 256
#define SSL_MAX_LEN 16
typedef struct 
{
    uint16_t rom_statue;
    uint32_t rom_size;
    uint8_t  ssl_data[SSL_MAX_LEN];
}update_param_def;

void HW_Init(void);
void Printf_SystemRccClocks(void);
void SW_Init(void);

#endif
