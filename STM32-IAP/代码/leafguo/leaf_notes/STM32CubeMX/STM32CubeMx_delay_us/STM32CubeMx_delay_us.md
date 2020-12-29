# STM32CubeMx开发之路—微妙延时函数delay_us()

---

## 运行环境

* `Windows10`
* `STM32CubeMX` `Version 5.4.0`
* `Keil5(MDK5)` `Version 5.28.0.0`
* `硬件开发板` `F103RB-NUCLEO`

---

## 简介

本例程主要讲解如何使用STM32在不用定时器的情况下使用微秒延时函数.

---

## STM32CubeMx基本配置

  基础配置过程请参考 [STM32CubeMx开发之路—配置第一个项目](https://blog.csdn.net/weixin_41294615/article/details/85235905)

---

## 使用`printf`功能

  重定向`printf`的过程请参考 [STM32CubeMx开发之路—3发送USART数据和printf重定向](https://blog.csdn.net/weixin_41294615/article/details/86154538)

## CubeMX配置

### 查看时钟树

* CubeMX不需要什么特别的配置, 只需要查看一下时钟树就行了, 而且在代码中也有专门的接口调用, 这儿大家了解一下, 由下图可知, 系统的时钟为`72M HZ`.

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_delay_us/image/clocktree.png"/></div>

---

## 代码修改

* 本节的微秒延时主要是通过不断获取系统时间来实现的
* 在`main.c`文件中添加如下函数代码

```
/**
 * @bieaf 微秒延时函数
 * @detail 注意:由于指令执行需要时间, 实际的延时精度约为1微秒
 *
 * @param unsigned int Delay 延时的微秒
 */
void delay_us(unsigned int Delay)
{
    uint32_t tickstart = SysTick->VAL;                            ///<获取当前tick
    uint32_t tickNum = 0;
    uint32_t tickMax = SysTick->LOAD + 1;
    uint32_t delay_usvalue = (tickMax / 1000) * Delay;            ///<计算一共需要延时的tick
    while(1)
    {
        uint32_t cur_tick = SysTick->VAL;
        if (cur_tick > tickstart)                                 ///<进行了一次重载
        {
            tickNum = tickstart + (tickMax - cur_tick);
        }
        else                                                      ///<未进行过重载
        {
            tickNum = tickstart - cur_tick;
        }

        if (tickNum > delay_usvalue)                              ///<达到延时的tick数
        {
            return;
        }
    }
}
```

---

* 在主函数`main()`中添加如下测试函数

```
while (1)
{
  static unsigned int start_time = 0;
  static unsigned int end_time = 0;
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */


  start_time = SysTick->VAL;    ///<延时前获取tick
  delay_us(10);                 ///<延时
  end_time = SysTick->VAL;      ///<延时后获取tick
  
  printf("delay time tick is %d\r\n", start_time - end_time); ///打印输出
  HAL_Delay(1000);
}
/* USER CODE END 3 */
```

---

* 结果分析, 如下图所示:
* **我们延时的时间为10us, 会发现tick相差的是756,是不是我们的延时函数错了呢?**
* 进一步分析, 系统时钟是`72M HZ`的, 也就是说1s系统时钟走72M次, 也就是说1us走72次
* 10us的时间应该走`72 * 10 = 720`次,而我们测试的结果为756次, 多了36次约为0.5us
* **这些时间是哪儿多出来的呢?**
* 如果你使用过汇编, 你可能就知道, 进出函数的跳转指令, 取指令, 读指令, ...等各种指令需要时间的,多出来的时间就是那些时间造成的误差.
* **有没有办法消除这些误差?**
* 办法是有的, 不过这和系统相关, 可以经过多次测试, 在延时函数里面减去多的这些时间误差, 这儿不详细说明, 一般对于要用的微秒延时的像软件IIC这样的功能, 这些误差是没有影响的,大可放心使用.

<div align=center><img width="700" src="https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_delay_us/image/out.png"/></div>

---

## 源码

[STM32F103rb_delay_us.zip](https://gitee.com/leafguo/leafguo/raw/master/leaf_notes/STM32CubeMX/STM32CubeMx_delay_us/STM32F103rb_delay_us.zip)

**备注：提供一下个人微信号**<font color=#ff0000> Hleafleafleaf</font>**，欢迎加好友，共同学习！共同进步！**

---
