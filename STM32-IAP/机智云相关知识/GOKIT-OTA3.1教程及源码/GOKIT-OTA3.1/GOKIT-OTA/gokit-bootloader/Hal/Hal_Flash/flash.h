#ifndef _FLASH_
#define _FLASH_
#include "gokit.h"

/* BootLoader Flash首地址 */
#define SYS_Bootloader_SAVE_ADDR_BASE       0x08000000//Bootloader首地址

/* 升级参数存储 */
#define UPDATE_PARAM_SAVE_ADDR_BASE         0x08002C00
#define UPDATE_PARAM_MAX_SIZE               (1*1024)//支持参数大小1KB

/* APP Flash首地址 */
#define SYS_APP_SAVE_ADDR_BASE              0x08003000
#define APP_DATA_MAX_SIZE                   (26*1024)//支持APP大小26KB

/* APP BAK Flash首地址 */
#define SYS_APP_BAK_SAVE_ADDR_BASE          0x08009800
#define APP_BAK_DATA_MAX_SIZE               (26*1024)//支持APP_BAK大小26KB


/* FLASH页大小 */
#define FLASH_PAGE_SIZE           0x400   //1KB

/* 大数据分片大小 */
//#define BIGDATA_PIECE_MAX 200 //支持的最大数据分片即大数据最大传输单元,200B


void save_param_to_flash(uint16_t * buf_to_save,uint16_t len );
void read_param_from_flash(uint16_t * buf_to_get,uint16_t len);
void set_flash_flag_to_updata(uint16_t crc_code);

void rFlashData(uint8_t * buf_to_get , uint16_t len , uint32_t rFlashAddr);
void wFlashData(uint8_t * buf_to_save , uint16_t len , uint32_t wFlashAddr);
void iap_load_app(u32 appxaddr);
void flash_erase(uint32_t size , uint32_t addr_base);
#endif
