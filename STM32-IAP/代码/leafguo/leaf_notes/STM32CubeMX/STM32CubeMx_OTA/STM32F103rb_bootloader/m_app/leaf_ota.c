#include "leaf_ota.h"
#include "main.h"
#include "stdio.h"



/**
 * @bieaf ����ҳ,F1ϵ��Flash��ҳ���֣���F4�����������֣�����������һ��
 *
 * @param pageaddr  ��ʼ��ַ	
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
 * @param buff       д�����ݵ���ʼ��ַ
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



/**
 * @bieaf �����ɸ�����
 *
 * @param addr       �����ݵĵ�ַ
 * @param buff       �������ݵ�����ָ��
 * @param word_size  ����
 * @return 
 */
static void ReadFlash(uint32_t addr, uint32_t * buff, uint16_t word_size)
{
	for(int i =0; i < word_size; i++)
	{
		buff[i] = *(__IO uint32_t*)(addr + 4 * i);
	}
	return;
}


/* ��ȡ����ģʽ */
unsigned int Read_Start_Mode(void)
{
	unsigned int mode = 0;
	//��ȡAPP2(���������һ���ֽڣ���дAPP2ʱ��Ϊ����Ϊ0xAAAAAAAA)
	ReadFlash((Application_2_Addr + Application_Size - 4), &mode, 1);//0x0800F000U + 0xA000U -4 = 0x08018FFC 
	return mode; //���ض�ȡ��ַ��ֵ
}



/**
 * @bieaf ���г���ĸ���
 * @detail 1.����Ŀ�ĵ�ַ
 *         2.Դ��ַ�Ĵ��뿽����Ŀ�ĵ�ַ
 *         3.����Դ��ַ
 *
 * @param  ���˵�Դ��ַ
 * @param  ���˵�Ŀ�ĵ�ַ
 * @return ���˵ĳ����С
 */
void MoveCode(unsigned int src_addr, unsigned int des_addr, unsigned int byte_size)
{
	/*1.����Ŀ�ĵ�ַ*/
	printf("> Start erase des flash......\r\n");
	Erase_page(des_addr, (byte_size/PageSize));
	printf("> Erase des flash down......\r\n");
	
	/*2.��ʼ����*/	
	unsigned int temp[256];
	
	printf("> Start copy......\r\n");
	for(int i = 0; i < byte_size/1024; i++)
	{
		ReadFlash((src_addr + i*1024), temp, 256);
		WriteFlash((des_addr + i*1024), temp, 256);
	}
	printf("> Copy down......\r\n");
	
	/*3.����Դ��ַ*/
	printf("> Start erase src flash......\r\n");
	Erase_page(src_addr, (byte_size/PageSize));
	printf("> Erase src flash down......\r\n");
}



/* ���û������ջ��ֵ */
__asm void MSR_MSP (uint32_t ulAddr) 
{
    MSR MSP, r0 			//set Main Stack value���βε�ֵ����r0�Ĵ����ģ����Դ˴�ֱ�ӽ�r0��ֵ����MSP
    BX r14						//�������� = ��ת��LR���ӼĴ�����LR�Ĵ����洢���Ǻ������ӳ������ʱ�ķ��ص�ַ
}



/* ������ת���� */
typedef void (*Jump_Fun)(void); //����һ������ָ�����
//��ת��App_AddrӦ�ó����ַ
void IAP_ExecuteApp (uint32_t App_Addr)
{
	Jump_Fun JumpToApp;      //���庯��ָ��JumpToApp = void (*JumpToApp)(void)                                                                                                 
    
	if ( ( ( * ( __IO uint32_t * ) App_Addr ) & 0x2FFE0000 ) == 0x20000000 )	//���ջ����ַ�Ƿ�Ϸ�.__IO=��дȨ�ޣ��׵�ַ�洢����MSP�ĳ�ʼֵ
	{ 
		JumpToApp = (Jump_Fun) * ( __IO uint32_t *)(App_Addr + 4);				//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP( * ( __IO uint32_t * ) App_Addr );								//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		JumpToApp();															//��ת��APP.
	}
}



/**
 * @bieaf ����BootLoader������
 *
 * @param none
 * @return none
 */
void Start_BootLoader(void)
{
	/*==========��ӡ��Ϣ==========*/  
//	 printf("\r\n");
//	 printf("******************************************\r\n");
//	 printf("*                                        *\r\n");
//	 printf("*    *****     ****     ****   *******   *\r\n");
//	 printf("*    *   **   *    *   *    *     *      *\r\n");
//	 printf("*    *    *   *    *   *    *     *      *\r\n");
//	 printf("*    *   **   *    *   *    *     *      *\r\n");
//	 printf("*    *****    *    *   *    *     *      *\r\n");
//	 printf("*    *   **   *    *   *    *     *      *\r\n");
//	 printf("*    *    *   *    *   *    *     *      *\r\n");
//	 printf("*    *   **   *    *   *    *     *      *\r\n");
//	 printf("*    *****     ****    ****       *      *\r\n");
//	 printf("*                                        *\r\n");
//	 printf("*********************** by Leaf_Fruit ****\r\n");
	
	 printf("\r\n");
	 printf("***********************************\r\n");
	 printf("*                                 *\r\n");
	 printf("*    Leaf_Fruit's BootLoader      *\r\n");
	 printf("*                                 *\r\n");
	 printf("***********************************\r\n");
	
	printf("> Choose a startup method......\r\n");	
	switch(Read_Start_Mode())									///< ��ȡ�Ƿ�����Ӧ�ó��� */
	{
		case Startup_Normol:										///< ��������,û����д��������FLASH��������0xFFFFFFFF */
		{
			printf("> Normal start......\r\n");
			break;
		}
		case Startup_Update:										///< ��������������⵽��¼��APP2����APP2���һ���ֽ���Ϊ����Ϊ0xAAAAAAAA */
		{
			printf("> Start update......\r\n");		
			MoveCode(Application_2_Addr, Application_1_Addr, Application_Size); //��Application_Size��С��Դ�����Application_2_Addr��ֵ��Application_1_Addr��ַ��������������Դ����
			printf("> Update down......\r\n");
			break;
		}
		case Startup_Reset:										///< �ָ��������� Ŀǰûʹ�� */
		{
			printf("> Restore to factory program......\r\n");
			break;			
		}
		default:														///< ����ʧ��
		{
			printf("> Error:%X!!!......\r\n", Read_Start_Mode()); //��ӡʧ�ܵ�״̬
			return;			
		}
	}
	
	/* ��ת��Ӧ�ó��� */
	printf("> Start up......\r\n\r\n");	
	IAP_ExecuteApp(Application_1_Addr); //��ת��APP1�ĵ�ַ
}




