/*
**********************************************************************
*                         头文件包含
**********************************************************************
*/
#include "Uart.h"
#include "DS18B20.h"
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
u16 temp = 0;	 				// 用来暂存12位的AD值
u8 tmh = 0, tml = 0;			// 用来暂存2个8位的AD值
u8 i = 0,j = 0; 
u8 N = 5; //数组排序参数


u8 Temperature_Read = 0;
u8 Receive_Flag = 0;
u8 T_Single = 0;
u8 T_Decade = 0;
u8 Time_Cnt = 0; // Time_Cnt用来计算中断次数
u8 Time_Threshold = 100; //Time_Threshold判断达到每隔100*10ms = 1s

u8 Temperature_Set = 10;
double Temperature_SetNew = 0;
double Temperature_Diff = 1.3;
double Temperature = 0;	//温度计算值
double Temperature_Collect = 0; //温度采集值
double Temperature_Temp = 0; //温度暂存值
double Temperature_Sum = 0; //温度去掉最大最小求和值
double Temperature_Array[5] = {0,0,0,0,0}; //温度数据暂存数组
double Kp = 1,Ki = 0,Kd = 10;
double Err = 0,Err_Last = 0,Err_Integral = 0,Rerr = 0;
double Rerr_allow = 0; //PID允许误差范围

/*
**********************************************************************
*                         函数原型声明
**********************************************************************
*/
void delay10ms(void);
void Int0Init(void);
void Int1Init(void);
void TemperatureRead(void);
void PID(void);
void Timer0Init(void);
/*********************************************************************
* 函 数 名       : main
* 函数功能		 : 主函数
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void main(void)
{	
	UartInit();
	Int0Init();
	Int1Init();
	Timer0Init();
	while (1)
	{
	   Temperature_Sum = 0;
	   for(i = 0;i < 5;i++) //温度采集数值预处理， 5个值进行处理输出一个值，平均1s输出一个值
	   {
			TemperatureRead();
			Temperature_Array[i] = Temperature_Collect;
	   }
	   
	   for(i = 0;i < N - 1;i++) //外层循环控制轮数（从小到大排序）
	   {
	   		for(j = 0;j < N - i - 1;j++) //内层循环控制每轮的比较次数
			{
				if(Temperature_Array[j] > Temperature_Array[j + 1]) //如果当前值大于后一个值，就交换
				{
					Temperature_Temp = 	Temperature_Array[j];
					Temperature_Array[j] = Temperature_Array[j + 1];
					Temperature_Array[j + 1] = Temperature_Temp;
				}	
			}
	   }
		    for(i = 1;i < N - 1;i++) //去掉最大最小值   
    {       
         Temperature_Sum += Temperature_Array[i];    
    }    
     Temperature = Temperature_Sum/(N - 2); //取平均 
	   Temperature_Read = (u8)(Temperature);
	   PID();
	   if(Rerr > Rerr_allow)
	   CT_Led2 = 0; //加热
	   else
	   CT_Led2 = 1; //停止加热


//	   }
	}
}

/*********************************************************************
* 函 数 名       : Timer0Init
* 函数功能		 : 定时器0初始化函数
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Timer0Init(void)
{
	TMOD &= 0xf0;		// T0设为定时器模式，仅用TR0控制打开
	TMOD |= 0x01;		// T0选择为工作模式1：16位定时器、TH0 TL0全使用

	TH0 = 0xd8;			// 使用单片机小精灵计算10ms定时的TH0、TL0初值
	TL0 = 0xf0;	
    ET0 = 0;			// 先关定时器0中断允许
	EA = 1;				// 打开总中断
	TR0 = 1;			// 开始计时			
}



/*********************************************************************
* 函 数 名       : Timer0Isr
* 函数功能		 : 定时器0中断处理程序
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Timer0Isr(void) interrupt 1
{
	TH0 = 0xd8;				// 使用单片机小精灵计算10ms定时的TH0、TL0初值
	TL0 = 0xf0;	
	
	if (Time_Cnt++ >= 100)   // 每隔100*10ms = 1s，则让LED亮灭状态转换一次
	{
		Time_Cnt = 0;		// gCnt置为初值0以开启下一次中断次数计数
		Led5 = !Led5;	   	// LED亮灭转换
		UartSendByte(Temperature_Read/10 + '0');
		UartSendByte(Temperature_Read%10 + '0');
		UartSendByte(' ');
			
	}
}
/*********************************************************************
* 函 数 名       : Int0Init
* 函数功能		 : 初始化INT0为下降沿触发、并打开各级中断开关
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Int0Init(void)
{
	IT0 = 1;			// 设置为边沿触发模式（下降沿）
	EX0 = 1;			// 打开INT0的中断允许。	
	EA = 1;				// 打开总中断	
}


/*********************************************************************
* 函 数 名       : Int1Init
* 函数功能		 : 初始化INT1为下降沿触发、并打开各级中断开关
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Int1Init(void)
{
	IT1 = 1;			// 设置为边沿触发模式（下降沿）
	EX1 = 1;			// 打开INT1的中断允许。	
	EA = 1;				// 打开总中断	
}
/*********************************************************************
* 函 数 名       : Int0Isr
* 函数功能		 : INT0的中断处理程序(interrupt service routine)
				   注意：INT0对应的中断号码是0
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Int0Isr(void)	interrupt 0		
{
	delay10ms();	 		// 软件延时消抖
	if (CT_Time == 0)			// 延时10ms后S15仍为低电平证明按键确实被按下
	{
		Led3 ^= 1;		// LED1取反
		ET0 ^= 1;
		if(ET0 == 1)
		{
			UartSendByte('T');
			UartSendByte(':');
		}
	}
}
/*********************************************************************
* 函 数 名       : Int1Isr
* 函数功能		 : INT1的中断处理程序(interrupt service routine)
				   注意：INT1对应的中断号码是2而不是1
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void Int1Isr(void)	interrupt 2		
{
	delay10ms();	 		    // 软件延时消抖
	delay10ms();
	delay10ms();
	if (send_T == 0)			// 延时10ms后S16仍为低电平证明按键确实被按下
	{
		Led4 ^= 1;
		UartSendByte('T');
		UartSendByte('_');
		UartSendByte('s');
		UartSendByte('e');
		UartSendByte('t');
		UartSendByte(Temperature_Set/10 + '0');
		UartSendByte(Temperature_Set%10 + '0');
		UartSendByte(' ');
		UartSendByte('T');
		UartSendByte(Temperature_Read/10 + '0');
		UartSendByte(Temperature_Read%10 + '0');
		UartSendByte(' ');

	}
}



/*********************************************************************
* 函 数 名       : delay10ms
* 函数功能		 : 由单片机小精灵V1.3生成的延时10ms的精确延时函数
* 参数列表       : 无
* 函数输出    	 : 无
*********************************************************************/
void delay10ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=38;b>0;b--)
            for(a=130;a>0;a--);
}

void TemperatureRead(void)
{
	Ds18b20TempConvertCmd();		// 先写入转换命令
	Ds18b20TempReadCmd();			// 然后等待转换完后发送读取温度命令
	tml = Ds18b20ReadByte();		// 读取温度值共16位，先读低字节
	tmh = Ds18b20ReadByte();		// 再读高字节

	temp = tml | (tmh << 8);		// 默认是12位分辨率，前面4个S位是符号位
	
	// 正温度时符号位为0，下面代码计算没有考虑负温度情况，因为我们实验是在
	// 室温下做的，如果要考虑到负温度的情况，代码中要先判断S位，若S位为1则
	// 必须点去掉S的1再计算，计算后的值加负号即可。
	Temperature_Collect = temp * 0.0625;

}

void PID(void)
{
	Err = Temperature_Set - Temperature;
	Err_Integral += Err;
	Rerr = Kp*Err + Ki*Err_Integral + Kd*(Err - Err_Last);
	Err_Last = Err;
	 
}


