# STM32CubeMx开发之路—在线升级OTA(3/4)—App

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

本例程主要讲解在线升级(OTA)的`App1`的编写以及整个流程的说明.我将以我例程的App为例, 采用Ymodem协议进行串口传输,讲解`App`的编写(后面会提供免费的代码下载链接), 其他的协议原理大体上都差不多, 都是通过某种协议拿到升级的代码.

---

## 流程图分析

以我历程的App1为例:

* 先修改向量表, 因为本程序是由BootLoader跳转过来的, 不修改向量表后面会出现问题.
* 打印版本信息, 方便查看不同的App版本
* 本例程的升级程序采用串口的Ymoderm协议进行传输bin文件. 具体的流程图见下图所示

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/App.jpg"/></div>

---

## 程序编写和分析

所需STM32的资源有:
* printf的使用, 详情可参考 [STM32CubeMx开发之路—3发送USART数据和printf重定向](https://blog.csdn.net/weixin_41294615/article/details/86154538)
* Flash的读写, 详情可参考 [STM32CubeMX开发之路—8Flash读写](https://blog.csdn.net/weixin_41294615/article/details/87610039)
* 串口的DMA收发, 详情可参考 [STM32CubeMx开发之路—4采用DMA方式收发数据](https://blog.csdn.net/weixin_41294615/article/details/86218556)
* YModem协议相关的, 详情可参考 [YModem介绍](https://blog.csdn.net/weixin_41294615/article/details/104652105)

---

## Ymodem协议

* 百度百科[Ymodem协议](https://baike.baidu.com/item/Ymodem)
* 具体流程可自行查找相关文档, 这儿提供一个我找到的 [XYmodem.pdf](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/xymodem.pdf).
* Ymodem协议相关介绍可参考我的这篇教程 [YModem介绍](https://blog.csdn.net/weixin_41294615/article/details/104652105).

## 代码分析

* 代码大多数都是通过串口实现Ymodem协议的接收, 这儿就不详细说明
* 后面放了我的源代码, 详情请参考我的源代码.

* 主函数添加修改向量表的指令

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/xlb.png"/></div>

---

* 打印版本信息以及跳转指令

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/ver.jpg"/></div>

---

* YModem相关的文件接收部分

```

/**
 * @bieaf YModem升级
 *
 * @param none
 * @return none
 */
void ymodem_fun(void)
{
	int i;
	if(Get_state()==TO_START)
	{
		send_command(CCC);
		HAL_Delay(1000);
	}
	if(Rx_Flag)    	// Receive flag
	{
		Rx_Flag=0;	// clean flag
				
		/* 拷贝 */
		temp_len = Rx_Len;
		for(i = 0; i < temp_len; i++)
		{
			temp_buf[i] = Rx_Buf[i];
		}
		
		switch(temp_buf[0])
		{
			case SOH:///<数据包开始
			{
				static unsigned char data_state = 0;
				static unsigned int app2_size = 0;
				if(Check_CRC(temp_buf, temp_len)==1)///< 通过CRC16校验
				{					
					if((Get_state()==TO_START)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< 开始
					{
						printf("> Receive start...\r\n");

						Set_state(TO_RECEIVE_DATA);
						data_state = 0x01;						
						send_command(ACK);
						send_command(CCC);

						/* 擦除App2 */							
						Erase_page(Application_2_Addr, 40);
					}
					else if((Get_state()==TO_RECEIVE_END)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< 结束
					{
						printf("> Receive end...\r\n");

						Set_Update_Down();						
						Set_state(TO_START);
						send_command(ACK);
						HAL_NVIC_SystemReset();
					}					
					else if((Get_state()==TO_RECEIVE_DATA)&&(temp_buf[1] == data_state)&&(temp_buf[2] == (unsigned char)(~temp_buf[1])))///< 接收数据
					{
						printf("> Receive data bag:%d byte\r\n",data_state * 128);
						
						/* 烧录程序 */
						WriteFlash((Application_2_Addr + (data_state-1) * 128), (uint32_t *)(&temp_buf[3]), 32);
						data_state++;
						
						send_command(ACK);		
					}
				}
				else
				{
					printf("> Notpass crc\r\n");
				}
				
			}break;
			case EOT://数据包开始
			{
				if(Get_state()==TO_RECEIVE_DATA)
				{
					printf("> Receive EOT1...\r\n");
					
					Set_state(TO_RECEIVE_EOT2);					
					send_command(NACK);
				}
				else if(Get_state()==TO_RECEIVE_EOT2)
				{
					printf("> Receive EOT2...\r\n");
					
					Set_state(TO_RECEIVE_END);					
					send_command(ACK);
					send_command(CCC);
				}
				else
				{
					printf("> Receive EOT, But error...\r\n");
				}
			}break;	
		}
	}
}
```
* 其中部分函数未在以上代码中展现, 详情请参看下面的源代码(免费).


## 源码

`App1源代码` [STM32F103rb_App1.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/STM32F103rb_App1.zip)

---

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
