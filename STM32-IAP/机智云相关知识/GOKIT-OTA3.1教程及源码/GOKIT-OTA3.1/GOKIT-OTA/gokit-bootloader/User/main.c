/**
********************************************************
*
* @file      main.c
* @author    Gizwtis
* @version   V2.3
* @date      2015-07-06
*
* @brief     机智云 只为智能硬件而生
*            Gizwits Smart Cloud  for Smart Products
*            链接|增值|开放|中立|安全|自有|自由|生态
*            www.gizwits.com
*
*********************************************************/

/* Includes ------------------------------------------------------------------*/


#include "gokit.h"
#include "../User/md5/gagent_md5.h"

/*Global Variable*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
update_param_def update_param;
#ifdef DEBUG
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (unsigned char) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    return (ch);
}
void usart1_init()
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);
    USART1_GPIO_Cmd(USART1_GPIO_CLK, ENABLE);
    USART1_AFIO_Cmd(USART1_AFIO_CLK, ENABLE);
    USART1_CLK_Cmd(USART1_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USART1_RxPin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART1_GPIO_PORT, &GPIO_InitStructure);
    
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
    USART_Cmd(USART1, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_TC);
}
#endif

void mcu_restart()
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

uint8_t md5_calc[SSL_MAX_LEN];
MD5_CTX ctx;
int8_t ROM_MD5_Check(uint32_t sys_size , uint32_t FlashAddr , uint8_t *ssl)
{
    uint8_t update_data_tmp[PIECE_MAX];
    uint32_t load_loop = 0;
    uint32_t remaind_data_len = sys_size;
    uint32_t valid_data_len = 0;
	
    GAgent_MD5Init(&ctx);
    if(0 == sys_size%PIECE_MAX)
    {
        load_loop = sys_size / PIECE_MAX;
    }
    else
    {
        load_loop = sys_size / PIECE_MAX + 1;
    }
#ifdef DEBUG
    printf("Check New Sys ...loop = %d\r\n",load_loop);
#endif
    for(uint32_t i = 0;i<load_loop;i++)
    {
        if(remaind_data_len > PIECE_MAX)
        {
            valid_data_len = PIECE_MAX;
        }
        else
        {
            valid_data_len = remaind_data_len;
        }
        memset(update_data_tmp,0,PIECE_MAX);
        rFlashData(update_data_tmp, valid_data_len, FlashAddr + i*PIECE_MAX);
        GAgent_MD5Update(&ctx, update_data_tmp, valid_data_len);
        remaind_data_len = remaind_data_len - valid_data_len;
#ifdef DEBUG
        printf("*");
#endif
    }
#ifdef DEBUG
    printf("\r\n");
#endif
    GAgent_MD5Final(&ctx, md5_calc);
#ifdef DEBUG
    printf("MD5 Calculate Success \r\n ");
#endif
    if(memcmp(ssl, md5_calc, SSL_MAX_LEN) != 0)
    {
#ifdef DEBUG
        printf("Md5_Cacl Check Faild ,MCU OTA Faild\r\n ");
#endif
#ifdef PROTOCOL_DEBUG
        printf("MD5: ");
        for(uint16_t i=0; i<SSL_MAX_LEN; i++)
        {
            printf("%02x ", md5_calc[i]);
        }
        printf("\r\n");
#endif
        return -1;
    }
    else
    {
#ifdef DEBUG
        printf("MD5 Check Success ,MCU OTA Success\r\n ");
#endif
        return 0;
    }
}

uint8_t update_new_system(uint32_t sys_size)
{
    uint8_t update_data_tmp[PIECE_MAX];
    uint32_t load_loop = 0;
    uint32_t remaind_data_len = sys_size;
    uint32_t valid_data_len = 0;
	
    if(0 == sys_size%PIECE_MAX)
    {
        load_loop = sys_size / PIECE_MAX;
    }
    else
    {
        load_loop = sys_size / PIECE_MAX + 1;
    }
#ifdef DEBUG
    printf("Copy New Sys ...loop = %d\r\n",load_loop);
#endif

    flash_erase(update_param.rom_size , SYS_APP_SAVE_ADDR_BASE);
		
#ifdef DEBUG
    printf("Copy New Sys\r\n");
#endif
    for(uint32_t i = 0;i<load_loop;i++)
    {
        if(remaind_data_len > PIECE_MAX)
        {
            valid_data_len = PIECE_MAX;
        }
        else
        {
            valid_data_len = remaind_data_len;
        }
        memset(update_data_tmp,0,PIECE_MAX);
        rFlashData(update_data_tmp, valid_data_len, SYS_APP_BAK_SAVE_ADDR_BASE + i*PIECE_MAX);
        
        wFlashData(update_data_tmp , valid_data_len , SYS_APP_SAVE_ADDR_BASE + i*PIECE_MAX);
        remaind_data_len = remaind_data_len - valid_data_len;
#ifdef DEBUG
        printf(".");
#endif
    }
#ifdef DEBUG
    printf("\r\n");
    printf("Copy Success , Wait to Check... \r\n");
#endif

    if(0 == ROM_MD5_Check(update_param.rom_size , SYS_APP_SAVE_ADDR_BASE , update_param.ssl_data))
    {
#ifdef DEBUG
        printf("New ROM Check Success , Wait to Load New Systerm \r\n");
#endif
        flash_erase(sizeof(update_param_def), UPDATE_PARAM_SAVE_ADDR_BASE);
        mcu_restart();
    }
    else
    {
#ifdef DEBUG
        printf("New ROM Check Faild , Update Faild , MCU Try To Update Again ,MCU Restart... \r\n");
#endif
        mcu_restart();
    }

    return 0;
}

int main(void)
{
	SystemInit();//初始化时钟和中断向量表等
#ifdef DEBUG
    usart1_init();
#endif
    memset((uint8_t *)&update_param, 0 , sizeof(update_param_def));
    rFlashData((uint8_t *)&update_param, sizeof(update_param_def), UPDATE_PARAM_SAVE_ADDR_BASE);
    if(0xEEEE == update_param.rom_statue)
    {
#ifdef DEBUG
        printf("Update Task ,Sys Will Load New Sys..Wait For A Moment \r\n");
        printf("Update Size [%d] \r\n",update_param.rom_size);
#endif
        if(0 == ROM_MD5_Check(update_param.rom_size , SYS_APP_BAK_SAVE_ADDR_BASE , update_param.ssl_data))
        {
            update_new_system(update_param.rom_size);
        }
        else
        {
#ifdef DEBUG
            printf("Check Faild , Go to Old Systerm\r\n");
#endif
            flash_erase(sizeof(update_param_def), UPDATE_PARAM_SAVE_ADDR_BASE);
            if(((*(vu32*)(SYS_APP_SAVE_ADDR_BASE + 4)) & 0xFF000000) == 0x08000000)
            {
#ifdef DEBUG
                printf("Sys Will Load APP.....\r\n");
#endif
                iap_load_app(SYS_APP_SAVE_ADDR_BASE);
            }
            else 
            {
#ifdef DEBUG
                printf("Start APP Failed!\r\n");
#endif
            }
        }
        
    }
    else
    {
#ifdef DEBUG
		printf("No Update Task , Go To APP ....%04X\r\n",update_param.rom_statue);
#endif
		if(((*(vu32*)(SYS_APP_SAVE_ADDR_BASE + 4)) & 0xFF000000) == 0x08000000)
		{
#ifdef DEBUG
            printf("Sys Will Load APP.....\r\n");
#endif
			iap_load_app(SYS_APP_SAVE_ADDR_BASE);
		}
		else 
		{
#ifdef DEBUG
			printf("Start APP Failed!\r\n");
#endif
		}
    }
	while(1)
	{

	}
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
