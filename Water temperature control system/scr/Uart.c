/*
**********************************************************************
*                         头文件包含
**********************************************************************
*/
#include "Uart.h"

/*
**********************************************************************
*                         本地宏定义
**********************************************************************
*/


/*
**********************************************************************
*                         本地全局变量
**********************************************************************
*/
extern u8 Temperature_Set;
extern u8 Receive_Flag;
extern u8 T_Single;
extern u8 T_Decade;
extern double Err_Last,Err_Integral;
/*
**********************************************************************
*                         函数原型声明
**********************************************************************
*/
void UartInit(void);
void UartSendByte(u8 c);


/*********************************************************************
* 函 数 名       : UartInit
* 函数功能		 : 串口初始化为9600波特率，8位数据位
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void UartInit(void)
{
    TMOD = 0x20;		// T1设置为8位自动重装载定时器			
    SCON = 0x50;		// 串口工作在模式1：8位UART波特率可变，且允许接收
    TH1 = 0xFA;			// 单片机小精灵V1.3算出的2400波特率且波特率
    TL1 = TH1;			// 加倍时的定时器设置值。
    PCON = 0x80;		// 设置为波特率加倍
	EA = 1;				// 开总中断
	ES = 1;			    // 开串口中断
	TR1 = 1;		    // 定时器1开启计数
}


/*********************************************************************
* 函 数 名       : UartSendByte
* 函数功能		 : 通过串口发送1字节数据给外部设备
* 参数列表       : c - 待发送的1字节数据
* 函数输出    	 : 无
*********************************************************************/
void UartSendByte(u8 c)
{
    SBUF = c;
    while (!TI);
    TI = 0;
}


/*********************************************************************
* 函 数 名       : UartIsr
* 函数功能		 : 串口的中断处理程序
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void UartIsr(void) interrupt 4	// 注意串口中断的编号是4
{
	u8 c = 0;				    // 临时变量用于存储串口中断接收的字符

    if (RI)						// 串口接收中断
    {
        RI = 0;
        //add your code here!
		c = SBUF;				// 通过串口接收外部发送的字符
//		UartSendByte(c);
		if(Receive_Flag == 2)
		{
			T_Decade = c - '0';
			Receive_Flag = 1;	
		}
        else if(Receive_Flag == 1)
		{
			T_Single = c - '0';
			Receive_Flag = 0;
			Err_Last = 0;
			Err_Integral = 0;
			Temperature_Set = T_Decade*10+T_Single;
				
		}
		else if(c == 't')
		{
			Receive_Flag = 2;
		}
		else
		{}

    }
    else 						// 串口发送中断
        TI = 0;					// if else中如果只有1句代码则可不加大括号
}

