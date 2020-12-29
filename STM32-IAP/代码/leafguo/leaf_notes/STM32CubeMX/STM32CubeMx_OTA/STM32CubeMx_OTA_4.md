# STM32CubeMx开发之路—在线升级OTA(4/4)—测试

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

本例程主要对前三章的教程做测试验证 `BootLoader` + `App`的升级功能.

---

## 源代码

* `BootLoader源代码` [STM32F103rb_delay_us.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/STM32F103rb_bootloader.zip)
* `App1源代码` [STM32F103rb_App1.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/STM32F103rb_App1.zip)


## 代码的下载

* 由下图可知两份代码的下载区域是不一样的
* 所以他们下载的区域也不一样

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/fenqu.jpg"/></div>

### BootLoader的下载

* BootLoader的代码默认是最开始的所以不需要特别设置代码的下载位置
* 按照下图, 修改擦除方式为`Erase Sectors`, 大小限制在`0X5000`(20K)

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/BootLoader_cfg.png"/></div>

---

* 烧录代码
* 运行, 通过串口1打印输出, 会看到以下打印消息
* 说明BootLoader已经成功运行

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/BootLoader_result.png"/></div>

---

### App1的下载

* App1稍微复杂一点, 需要将代码的起始位置设置为`0x08005000`
* 同时也要修改擦除方式为`Erase Sectors`, 见下图

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/App_cfg1.png"/></div>

---

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/App_cfg2.png"/></div>

---

* 烧录代码
* 运行, 通过串口1打印输出, 会看到以下打印消息
* 说明`BootLoader`已经成功跳转到版本号为0.0.1的`App1`

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/App1_result.png"/></div>

---

## 生成App2的.bin文件

* Keil如何生成.bin文件, 请参考这篇博文 [Keil如何生成.bin文件](https://blog.csdn.net/weixin_41294615/article/details/104656577)
* 修改代码, 把版本号改为0.0.2, 并且编译并且生成.bin文件


---
* 生成好之后你会得到一个.bin结尾的文件, 这就是我们待会儿YModem要传输的文件

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/bin_file.png"/></div>

---

## 使用Xshell进行文件传输

* 打开Xshell
* 代码中, 串口1进行调试信息的打印, 串口2进行YModem升级的
* 所以使用Xshell打开串口2进行文件传输, 串口1则可以通过串口调试助手查看调试消息, 具体过程如下


<div align=center><img width="1000" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/download.gif"/></div>

* 你会看到App的版本成功升级到0.0.2了.
* 如果你到了这一步.
* 那么恭喜你! 你已经能够使用在线升级了!

---

## 总结

通过本几节的教程, 想必你已经会使用在线升级了, 只要原理知道了其他的问题都可以迎刃而解了, 除了使用YModem协议传输.bin文件, 你还可以通过蓝牙, WIFI,等其他协议传输, 只要能够将.bin文件传输过去, 那其他的部分原理都差不多.

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
