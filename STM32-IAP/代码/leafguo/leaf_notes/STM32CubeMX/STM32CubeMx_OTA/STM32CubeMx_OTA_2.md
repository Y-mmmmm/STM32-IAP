# STM32CubeMx开发之路—在线升级OTA(2/4)—BootLoader

* [STM32CubeMx开发之路—在线升级OTA(1/4)—基础知识](https://blog.csdn.net/weixin_41294615/article/details/104669663)
* [STM32CubeMx开发之路—在线升级OTA(2/4)—BootLoader](https://blog.csdn.net/weixin_41294615/article/details/104669701)
* [STM32CubeMx开发之路—在线升级OTA(3/4)—App](https://blog.csdn.net/weixin_41294615/article/details/104669732)
* [STM32CubeMx开发之路—在线升级OTA(4/4)—测试](https://blog.csdn.net/weixin_41294615/article/details/104669766)
* [STM32CubeMx开发之路—在线升级OTA—(另一种思路)](https://blog.csdn.net/weixin_41294615/article/details/105122644)

---

## 运行环境

* `Windows10`
* `STM32CubeMX` `Version 5.4.0`
* `Keil5(MDK5)` `Version 5.28.0.0`
* `硬件开发板` `F103RB-NUCLEO`

---

## 简介

本例程主要讲解在线升级(OTA)的`BooLoader`的编写.我将以我例程的BootLoader为例, 讲解`BootLoader`(后面会提供免费的代码下载链接), 其他的大体上原理都差不多.

---

## 流程图分析

以我历程的BootLoader为例:
* 我将`App2区`的最后一个字节(`0x0801FFFC`)用来表示`App2区`是否有升级程序, STM32在擦除之后Flash的数据存放的都是`0xFFFFFFFF`, 如果有, 我们将这个地址存放`0xAAAAAAAA`. 具体的流程图见下图所示

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/BootLoader.jpg"/></div>
---

## 程序编写和分析

所需STM32的资源有:
* printf的使用, 可以参考以前的博文[STM32CubeMx开发之路—3发送USART数据和printf重定向](https://blog.csdn.net/weixin_41294615/article/details/86154538)
* Flash的读写, 可以参考以前的博文[STM32CubeMX开发之路—8Flash读写](https://blog.csdn.net/weixin_41294615/article/details/87610039)
* 程序跳转指令:可以参考如下代码:
```
/* 采用汇编设置栈的值 */
__asm void MSR_MSP (uint32_t ulAddr) 
{
    MSR MSP, r0   //设置Main Stack的值
    BX r14
}


/* 程序跳转函数 */
typedef void (*Jump_Fun)(void);
void IAP_ExecuteApp (uint32_t App_Addr)
{
  Jump_Fun JumpToApp;

  if ( ( ( * ( __IO uint32_t * ) App_Addr ) & 0x2FFE0000 ) == 0x20000000 )  //检查栈顶地址是否合法.
  {
    JumpToApp = (Jump_Fun) * ( __IO uint32_t *)(App_Addr + 4);  //用户代码区第二个字为程序开始地址(复位地址)  
    MSR_MSP( * ( __IO uint32_t * ) App_Addr );                  //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
    JumpToApp();                                                //跳转到APP.
  }
}
```

* 在需要跳转的地方执行这个函数就可以了`IAP_ExecuteApp(Application_1_Addr);`
* 其他的代码请参考`BootLoader源代码

## 源码

`BootLoader源代码` [STM32F103rb_delay_us.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/STM32F103rb_bootloader.zip)

---

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
