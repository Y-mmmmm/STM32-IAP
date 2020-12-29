# STM32CubeMx开发之路—移植easyflash

---

## 运行环境

* `Windows10`
* `STM32CubeMX` `Version 5.4.0`
* `Keil5(MDK5)` `Version 5.28.0.0`
* `硬件开发板` `F103RB-NUCLEO`

---

## 简介

本例程主要讲解如何移植easyflash.

---

## STM32CubeMx基本配置

  基础配置过程请参考 [STM32CubeMx开发之路—配置第一个项目](https://blog.csdn.net/weixin_41294615/article/details/85235905)

---

## 使用`printf`功能

  重定向`printf`的过程请参考 [STM32CubeMx开发之路—3发送USART数据和printf重定向](https://blog.csdn.net/weixin_41294615/article/details/86154538)

  本例程不需要其他什么外设资源, 配置`printf()`主要是方便我们调试.

## 下载easyflash源码

[下载EasyFlash源码](https://github.com/armink/EasyFlash)

会得到一个源码的仓库, 如下图所示

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/sourcecode.png"/></div>

**注意**: 源码的说明文档里面已经说明了如何使用这个库, 大家也可以参考这个说明进行移植, 我这儿根据自己的移植思路针对STM32使用HAL库做移植说明.

---

## 在自己的代码中添加easyflash所需要的文件

* 将EasyFlash源码源码中的`easyflash`文件夹复制到自己的工程中, 如下图:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/copy.png"/></div>

## 代码修改

* 添加`easyflash`的资源文件夹, 如下图:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addfile.png"/></div>

* 添加下面4个文件文件
`easyflash/src/easyflash.c`
`easyflash/src/ef_env.c`
`easyflash/src/ef_utils.c`
`easyflash/port/ef_port.c`
* 如图:
<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addfile1.png"/></div>

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addfile2.png"/></div>

* 添加路径, 如图

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addpath.png"/></div>

* 添加好后, 如图:
<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addpath1.png"/></div>

---

* 编译

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/error.png"/></div>

* 会发现有很多错误, 先不要慌, 出现错误是因为我们还没有进行配置的
* 打开`ef_cfg.h`文件进行配置, 具体怎么修改见下图:

* 随便写个版本本号, 我写的是`0`

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_cfg1.png"/></div>

* 根据实际情况填写最小擦除粒度, 我是用的是`F103rb`所以我写的是`1024`, 如果你使用的是`F103ze`就应该填`2048`,根据实际情况进行填写
* `EF_WRITE_GRAN`填`32`(上面有注释,如果是stm32f4就要填8)

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_cfg2.png"/></div>

* 填写`easyflash`的开始地址,我划分的分区是后面64K, 所以我填的是`(64 * EF_ERASE_MIN_SIZE + 0x08000000)`
* `ENV_AREA_SIZE`填写`(EF_ERASE_MIN_SIZE * 2)`
* `#define LOG_AREA_SIZE`因为我没有使用到, 所以我就把它注释掉了
* 如下图:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_cfg3.png"/></div>

---

* 打开`ef_port.c`文件,就下来配置接口
* 主要实现一下几个函数的接口
* `ef_port_init`(如果使用FAL, 则需要配置,本例程不需要配置)
* `ef_port_read`
* `ef_port_erase`
* `ef_port_write`
* 下面我已经写好了, 大家可以直接复制使用

---

* 1.添加头文件, 在最上面添加`#include "main.h"`

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_port1.png"/></div>

---

* 2.添加读函数
```
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* You can add your code under here. */
    uint8_t *buf_8 = (uint8_t *)buf;
    size_t i;
    for (i = 0; i < size; i++, addr ++, buf_8++)
    {
      *buf_8 = *(uint8_t *) addr;
    }
    return result;
}
```

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_port2.png"/></div>

---

* 3.添加擦除函数
```
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode result = EF_NO_ERR;

    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
    HAL_FLASH_Unlock();
  
    /* 擦除FLASH*/
    FLASH_EraseInitTypeDef FlashSet;
    FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
    FlashSet.PageAddress = addr;
    FlashSet.NbPages = (size + EF_ERASE_MIN_SIZE -1)/ EF_ERASE_MIN_SIZE;

    /*设置PageError，调用擦除函数*/
    uint32_t PageError = 0;
    HAL_FLASHEx_Erase(&FlashSet, &PageError);

    HAL_FLASH_Lock();
    return result;
}
```
<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_port3.png"/></div>

---

* 4.添加写函数
```
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
    
    /* You can add your code under here. */
    size_t i;
    uint32_t read_data;

    HAL_FLASH_Unlock();
    for (i = 0; i < size; i += 4, buf++, addr += 4)
    {
      /* write data */
      HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *buf);	
      read_data = *(uint32_t *)addr;
      /* check data */
      if (read_data != *buf)
      {
        result = EF_WRITE_ERR;
        break;
      }
    }
   HAL_FLASH_Lock();
    return result;
}
```
<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/ef_port4.png"/></div>

---

* 修改`ef_port.c`的向量表, 添加要使用的变量
* 如下图:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/addtable.png"/></div>

---

## 写测试程序

* 在主函数中添加`#include "easyflash.h"`头文件
* 在主函数添加如下测试代码:
```
  /* USER CODE BEGIN 2 */
  unsigned int Reboot_Time = 0;
  if(easyflash_init() == EF_NO_ERR)                         // 初始化成功
  {
    ef_get_env_blob("reboot_time", &Reboot_Time, 8, NULL);  // 读出reboot_time的值
  }
  Reboot_Time++;                                            // reboot_time的值加1

  ef_set_env_blob("reboot_time",&Reboot_Time,8);            // 保存reboot_time的值
  printf("Reboot_Time is %d\n",Reboot_Time);                // 打印reboot_time的值
  /* USER CODE END 2 */
```

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/main.png"/></div>

---

## 测试输出

* 打开串口调试助手
* 按复位按键, 会发现每次复位之后咱们存的值会增加1, 说明测试成功!

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/image/out.png"/></div>

---

## 源码

[STM32F103rb_easyflash.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_easyflash/STM32F103rb_easyflash.zip)

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
