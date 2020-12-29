#include "flash.h"

volatile FLASH_Status FLASHStatus;

/* MCU OTA */
void flash_erase_page(uint8_t flashPage , uint32_t addr_base)
{
	FLASHStatus = FLASH_COMPLETE;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(addr_base + flashPage*FLASH_PAGE_SIZE);
	FLASH_Lock();
}

void flash_erase(uint32_t size , uint32_t addr_base)
{
    uint32_t flashPageSum;
		uint32_t i;
    /*如果小于1024做处理*/
    if(size < FLASH_PAGE_SIZE)
        size = FLASH_PAGE_SIZE;
		
    /* 计算需要擦写的Flash页 */
    if((size % FLASH_PAGE_SIZE) == 0)
    {
        flashPageSum = size / FLASH_PAGE_SIZE;
    }
    else
    {
        flashPageSum = (size / FLASH_PAGE_SIZE) + 1;
    }
#ifdef DEBUG
    printf("Need Erase %d Pages \r\n",flashPageSum);
#endif
    FLASHStatus = FLASH_COMPLETE;
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
		
    for(i = 0;i<flashPageSum;i++)
    {
        FLASH_ErasePage(addr_base + i*FLASH_PAGE_SIZE);
    }
		
    FLASH_Lock();
#ifdef DEBUG
    printf("Erase Success  \r\n");
#endif
}

void writeFlash(uint16_t * buf_to_save , uint16_t len , uint32_t wFlashAddr)
{
    uint16_t count=0;
		FLASHStatus=FLASH_COMPLETE;
    if(wFlashAddr >= 0x08010000)
    {
#ifdef DEBUG
        printf("Waring:Flash Write Addr Error\r\n");
#endif
        return;
    }
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);/////ȥԽFlashәطքҪ־λ	 
	while(count < len && FLASHStatus == FLASH_COMPLETE)
	{
		FLASH_ProgramHalfWord((wFlashAddr + count*2),buf_to_save[count]); //вflashһٶַ֘дɫѫؖè16λé	
		count ++;     
	}
	FLASH_Lock();
}

void readFlash(uint16_t * buf_to_get,uint16_t len , uint32_t readFlashAddr)
{
	uint16_t count=0;
	while(count<len)
	{
	 	buf_to_get[count]=*(uint16_t *)(readFlashAddr + count*2);
		count++;
	}
}

/*写Flash,控制写长度,Flash地址偏移*/
#define PIECEMAXLEN 256
void wFlashData(uint8_t * buf_to_save , uint16_t len , uint32_t wFlashAddr)
{
    uint8_t WriteFlashTempBuf[PIECEMAXLEN];//写Flash临时缓冲区
    uint16_t WriteFlashTempLen = 0;//写Flash长度
	
    memset(WriteFlashTempBuf,0xEE,sizeof(WriteFlashTempBuf));//写Flash临时缓冲区首先全部填充0xEE
    memcpy(WriteFlashTempBuf,buf_to_save,len);//临时缓冲区
	
    WriteFlashTempLen = len;
    if(len%2 != 0)
        WriteFlashTempLen += 1;//因为Flash只能写半字
		
    writeFlash((uint16_t *)&WriteFlashTempBuf ,  WriteFlashTempLen/2 , wFlashAddr);
}

void rFlashData(uint8_t * buf_to_get , uint16_t len , uint32_t rFlashAddr)
{
    uint8_t ReadFlashTempBuf[PIECEMAXLEN];//写Flash临时缓冲区
    uint16_t ReadFlashTempLen = 0;//写Flash长度
    
    if(len%2 == 0)
    {
        ReadFlashTempLen = len;
        readFlash((uint16_t *)&ReadFlashTempBuf,ReadFlashTempLen/2 , rFlashAddr);
        memcpy(buf_to_get,ReadFlashTempBuf,len);
    }
    else
    {
        ReadFlashTempLen = len + 1;//因为Flash只能读半字
        readFlash((uint16_t *)&ReadFlashTempBuf,ReadFlashTempLen/2 , rFlashAddr);
        memcpy(buf_to_get,ReadFlashTempBuf,len);
    }
}

/****IAP*****/
typedef  void (*iapfun)(void); //函数指针
iapfun jump2app; //定义一个函数指针
u16 iapbuf[1024];   

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 

//设置栈顶指针
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value，传入的addr参数就存在r0中
    BX r14
}

void iap_load_app(u32 appxaddr)
{
	//判断加载APP地址是否正确
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)
	{
#ifdef DEBUG
		printf("Stack Success!\r\n");
#endif
		jump2app = (iapfun)*(vu32*)(appxaddr+4);//将地址转换成指针再取值，再被定义为函数指针，指向APP程序运行入口
		MSR_MSP(*(vu32*)appxaddr);//设置栈顶指针
		jump2app(); //跳转至APP
	}
	else
	{
#ifdef DEBUG
		printf("Stack Failed!\r\n");
#endif
	}
}
