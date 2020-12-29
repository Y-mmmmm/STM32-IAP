#include "ymodem.h"
#include "main.h"
#include "stdio.h"



/* ����ָ�� */
void send_command(unsigned char command)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&command,1, 0xFFFF);
	HAL_Delay(10);
}



/**
 * @bieaf ����ҳ
 *
 * @param pageaddr  ҳ��ʼ��ַ	
 * @param num       ������ҳ��
 * @return 1
 */
static int Erase_page(uint32_t pageaddr, uint32_t num)
{
	HAL_FLASH_Unlock();
	
	/* ����FLASH*/
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = pageaddr;
	FlashSet.NbPages = num;
	
	/*����PageError�����ò�������*/
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	
	HAL_FLASH_Lock();
	return 1;
}



/**
 * @bieaf д���ɸ�����
 *
 * @param addr       д��ĵ�ַ
 * @param buff       д�����ݵ�����ָ��
 * @param word_size  ����
 * @return 
 */
static void WriteFlash(uint32_t addr, uint32_t * buff, int word_size)
{	
	/* 1/4����FLASH*/
	HAL_FLASH_Unlock();
	
	for(int i = 0; i < word_size; i++)	
	{
		/* 3/4��FLASH��д*/
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4 * i, buff[i]);	
	}

	/* 4/4��סFLASH*/
	HAL_FLASH_Lock();
}


/* ���������� */
void Set_Update_Down(void)
{
	unsigned int update_flag = 0xAAAAAAAA;				///< ��Ӧbootloader����������
	WriteFlash((Application_2_Addr + Application_Size - 4), &update_flag,1 );
}



/* ��ʱ�洢��buff */
unsigned char save_buf[128] = {0}; //���û�õ�����



/**
 * @bieaf CRC-16 У��
 *
 * @param addr ��ʼ��ַ
 * @param num   ����
 * @param crc   CRC
 * @return crc  ����CRC��ֵ
 */
#define POLY        0x1021  
uint16_t crc16(unsigned char *addr, int num, uint16_t crc)  
{  
    int i;  
    for (; num > 0; num--)					/* Step through bytes in memory */  
    {  
        crc = crc ^ (*addr++ << 8);			/* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)				/* Prepare to rotate 8 bits */  
        {
            if (crc & 0x8000)				/* b15 is set... */  
                crc = (crc << 1) ^ POLY;  	/* rotate and XOR with polynomic */  
            else                          	/* b15 is clear... */  
                crc <<= 1;					/* just rotate */  
        }									/* Loop for 8 bits */  
        crc &= 0xFFFF;						/* Ensure CRC remains 16-bit value */  
    }										/* Loop until num=0 */  
    return(crc);							/* Return updated CRC */  
}



/**
 * @bieaf ��ȡ���ݰ�������, ˳�����У��
 *
 * @param buf ��ʼ��ַ
 * @param len ����
 * @return 
 */
unsigned char Check_CRC(unsigned char* buf, int len)
{
	unsigned short crc = 0;
	
	/* ����CRCУ�� */
	if((buf[0]==0x00)&&(len >= 133)) //��λ����SOH��ʽ���͵Ļ������͵�һ֡���ȵ���133�ֽ�
	{
		crc = crc16(buf+3, 128, crc);    //����֡ȥ��ǰ�����ֽڵ�У��
		if(crc != (buf[131]<<8|buf[132]))
		{
			return 0;///< ûͨ��У��
		}
		
		/* ͨ��У�� */
		return 1;
	}
}



/* ���������Ĳ��� */
static enum UPDATE_STATE update_state = TO_START;
void Set_state(enum UPDATE_STATE state)
{
	update_state = state;
}


/* ��ѯ�����Ĳ��� */
unsigned char Get_state(void)
{
	return update_state;
}



unsigned char temp_buf[512] = {0};
unsigned char temp_len = 0;



/**
 * @bieaf YModem����
 *
 * @param none
 * @return none
 */
void ymodem_fun(void)
{
	int i;
	if(Get_state()==TO_START)
	{
		send_command(CCC);//֪ͨ���Ͷ˿��Է�������
		HAL_Delay(1000);
	}
	if(Rx_Flag)    	// Receive flag �����ڽ��յ���һ֡����
	{
		Rx_Flag=0;	// clean flag
				
		/* ���� */
		temp_len = Rx_Len; //��ȡ�������ݳ���
		for(i = 0; i < temp_len; i++)
		{
			temp_buf[i] = Rx_Buf[i]; //�������յ�������
		}
		
		switch(temp_buf[0]) //��ʼ���ݰ�����
		{
			case SOH://<���ݰ���ʼ
			{
				static unsigned char data_state = 0;
				static unsigned int app2_size = 0;
				if(Check_CRC(temp_buf, temp_len)==1)///< ͨ��CRC16У��
				{					
					if((Get_state()==TO_START)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< ����ʼ֡����ʼ
					{
						printf("> Receive start...\r\n");

						Set_state(TO_RECEIVE_DATA); //���õ�ǰ״̬Ϊ��������״̬
						data_state = 0x01;						
						send_command(ACK); //����Ӧ��
						send_command(CCC); //��������

						/* ����App2 */							
						Erase_page(Application_2_Addr, 40);
					}
					else if((Get_state()==TO_RECEIVE_END)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< �ǽ���֡������
					{
						printf("> Receive end...\r\n");

						Set_Update_Down();	//�����������					
						Set_state(TO_START);//���õ�ǰ״̬Ϊ����ʼ
						send_command(ACK);  //����Ӧ��
						HAL_NVIC_SystemReset(); //ϵͳ��λ
					}					
					else if((Get_state()==TO_RECEIVE_DATA)&&(temp_buf[1] == data_state)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< ��������
					{
						printf("> Receive data bag:%d byte\r\n",data_state * 128); //SOH��ʾ���δ�����128�ֽ�
						
						/* ��¼���� */
						WriteFlash((Application_2_Addr + (data_state-1) * 128), (uint32_t *)(&temp_buf[3]), 32);
						data_state++;
						
						send_command(ACK);	//Ӧ��һ��	
					}
				}
				else
				{
					printf("> Notpass crc\r\n");
				}
				
			}break;
			case EOT://���յ���λ������һ��EOT
			{
				if(Get_state()==TO_RECEIVE_DATA)
				{
					printf("> Receive EOT1...\r\n");
					
					Set_state(TO_RECEIVE_EOT2);	 //�յ�һ��EOT��׼�����յڶ���EOT				
					send_command(NACK);  //����NACK
				}
				else if(Get_state()==TO_RECEIVE_EOT2) //�յ��ڶ���EOT
				{
					printf("> Receive EOT2...\r\n");
					
					Set_state(TO_RECEIVE_END);	//���õ�ǰ״̬Ϊ��������״̬				
					send_command(ACK); //����Ӧ��
					send_command(CCC); //��������
				}
				else
				{
					printf("> Receive EOT, But error...\r\n");
				}
			}break;	
		}
	}
}



