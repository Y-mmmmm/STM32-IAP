# STM32CubeMx开发之路—在线升级OTA(1/4)—基础知识

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

本例程主要讲解在线升级(OTA)的基础知识, 主要是针对`IAP OTA`从`原理分析`, `分区划分`, 到`代码编写`和`实验验证`等过程阐述这一过程. 帮助大家加深对OTA的认识.

---

## 什么是BootLoader?

`BootLoader`可以理解成是引导程序, 它的作用是启动正式的`App应用程序`.
换言之, `BootLoader`是一个程序, App也是一个程序,  `BootLoader程序`是用于启动`App程序`的.

---

## STM32中的程序在哪儿?

正常情况下, 我们写的程序都是放在STM32片内Flash中(暂不考虑外扩Flash).
我们写的代码最终会变成二进制文件, 放进Flash中
感兴趣的话可以在`Keil`>>>`Debug`>>>`Memory`中查看, 右边Memory窗口存储的就是代码

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/bin.jpg"/></div>

接下来就可以进入正题了.

---

## 进行分区

既然我们写的程序都会变成二进制文件存放到Flash中, 那么我们就可以进一步对我们程序进行分区.
我使用的是`F103RB-NUCLEO开发板`,他的Flash一共128页, 每页1K.见下图:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/flash.jpg"/></div>

---

以它为例, 我将它分为三个区.`BootLoader区`、 `App1区`、 `App2区(备份区)`具体划分如下图:
* `BootLoader区`存放启动代码
* `App1区`存放应用代码
* `App2区`存放暂存的升级代码

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/fenqu.jpg"/></div>

---

## 总体流程图

* 先执行`BootLoader`程序, 先去检查`APP2`区有没有程序, 如果有就将App2区(备份区)的程序拷贝到`App1区`, 然后再跳转去执行`App1`的程序.
* 然后执行`App1`程序, 因为`BootLoader`和`App1`这两个程序的向量表不一样, 所以跳转到`App1`之后第一步是先去更改程序的向量表. 然后再去执行其他的应用程序.
* 在应用程序里面会加入程序升级的部分, 这部分主要工作是拿到升级程序, 然后将他们放到`App2区(备份区)`, 以便下次启动的时候通过`BootLoader`更新`App1`的程序.
流程图如下图所示:

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_OTA/image/Boot_App.png"/></div>

---

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
