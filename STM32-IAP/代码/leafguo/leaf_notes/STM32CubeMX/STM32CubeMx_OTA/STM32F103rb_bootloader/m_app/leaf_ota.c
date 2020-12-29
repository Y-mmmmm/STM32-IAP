#include "leaf_ota.h"
#include "main.h"
#include "stdio.h"



/**
 * @bieaf 擦除页,F1系列Flash以页划分，而F4是以扇区划分，擦除的区域不一样
 *
 * @param pageaddr  起始地址	
 * @param num       擦除的页数
 * @return 1
 */
static int Erase_page(uint32_t pageaddr, uint32_t num)
{
	HAL_FLASH_Unlock();
	
	/* 擦除FLASH*/
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = pageaddr;
	FlashSet.NbPages = num;
	
	/*设置PageError，调用擦除函数*/
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	
	HAL_FLASH_Lock();
	return 1;
}


/**
 * @bieaf 写若干个数据
 *
 * @param addr       写入的地址
 * @param buff       写入数据的起始地址
 * @param word_size  长度
 * @return 
 */
static void WriteFlash(uint32_t addr, uint32_t * buff, int word_size)
{	
	/* 1/4解锁FLASH*/
	HAL_FLASH_Unlock();
	
	for(int i = 0; i < word_size; i++)	
	{
		/* 3/4对FLASH烧写*/
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4 * i, buff[i]);	
	}

	/* 4/4锁住FLASH*/
	HAL_FLASH_Lock();
}



/**
 * @bieaf 读若干个数据
 *
 * @param addr       读数据的地址
 * @param buff       读出数据的数组指针
 * @param word_size  长度
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


/* 读取启动模式 */
unsigned int Read_Start_Mode(void)
{
	unsigned int mode = 0;
	//读取APP2(备份区最后一个字节，编写APP2时人为设置为0xAAAAAAAA)
	ReadFlash((Application_2_Addr + Application_Size - 4), &mode, 1);//0x0800F000U + 0xA000U -4 = 0x08018FFC 
	return mode; //返回读取地址的值
}



/**
 * @bieaf 进行程序的覆盖
 * @detail 1.擦除目的地址
 *         2.源地址的代码拷贝到目的地址
 *         3.擦除源地址
 *
 * @param  搬运的源地址
 * @param  搬运的目的地址
 * @return 搬运的程序大小
 */
void MoveCode(unsigned int src_addr, unsigned int des_addr, unsigned int byte_size)
{
	/*1.擦除目的地址*/
	printf("> Start erase des flash......\r\n");
	Erase_page(des_addr, (byte_size/PageSize));
	printf("> Erase des flash down......\r\n");
	
	/*2.开始拷贝*/	
	unsigned int temp[256];
	
	printf("> Start copy......\r\n");
	for(int i = 0; i < byte_size/1024; i++)
	{
		ReadFlash((src_addr + i*1024), temp, 256);
		WriteFlash((des_addr + i*1024), temp, 256);
	}
	printf("> Copy down......\r\n");
	
	/*3.擦除源地址*/
	printf("> Start erase src flash......\r\n");
	Erase_page(src_addr, (byte_size/PageSize));
	printf("> Erase src flash down......\r\n");
}



/* 采用汇编设置栈的值 */
__asm void MSR_MSP (uint32_t ulAddr) 
{
    MSR MSP, r0 			//set Main Stack value，形参的值传给r0寄存器的，所以此处直接将r0的值传给MSP
    BX r14						//函数返回 = 跳转到LR链接寄存器，LR寄存器存储的是函数或子程序调用时的返回地址
}



/* 程序跳转函数 */
typedef void (*Jump_Fun)(void); //定义一个函数指针别名
//跳转到App_Addr应用程序地址
void IAP_ExecuteApp (uint32_t App_Addr)
{
	Jump_Fun JumpToApp;      //定义函数指针JumpToApp = void (*JumpToApp)(void)                                                                                                 
    
	if ( ( ( * ( __IO uint32_t * ) App_Addr ) & 0x2FFE0000 ) == 0x20000000 )	//检查栈顶地址是否合法.__IO=读写权限，首地址存储的是MSP的初始值
	{ 
		JumpToApp = (Jump_Fun) * ( __IO uint32_t *)(App_Addr + 4);				//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP( * ( __IO uint32_t * ) App_Addr );								//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		JumpToApp();															//跳转到APP.
	}
}



/**
 * @bieaf 进行BootLoader的启动
 *
 * @param none
 * @return none
 */
void Start_BootLoader(void)
{
	/*==========打印消息==========*/  
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
	switch(Read_Start_Mode())									///< 读取是否启动应用程序 */
	{
		case Startup_Normol:										///< 正常启动,没有烧写升级程序，FLASH擦除后是0xFFFFFFFF */
		{
			printf("> Normal start......\r\n");
			break;
		}
		case Startup_Update:										///< 升级再启动，检测到烧录的APP2程序，APP2最后一个字节人为设置为0xAAAAAAAA */
		{
			printf("> Start update......\r\n");		
			MoveCode(Application_2_Addr, Application_1_Addr, Application_Size); //将Application_Size大小的源代码从Application_2_Addr赋值到Application_1_Addr地址，并擦除升级的源代码
			printf("> Update down......\r\n");
			break;
		}
		case Startup_Reset:										///< 恢复出厂设置 目前没使用 */
		{
			printf("> Restore to factory program......\r\n");
			break;			
		}
		default:														///< 启动失败
		{
			printf("> Error:%X!!!......\r\n", Read_Start_Mode()); //打印失败的状态
			return;			
		}
	}
	
	/* 跳转到应用程序 */
	printf("> Start up......\r\n\r\n");	
	IAP_ExecuteApp(Application_1_Addr); //跳转到APP1的地址
}




