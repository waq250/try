/*
**********************************************************************
*                         头文件包含
**********************************************************************
*/
#include "DS18B20.h"


/*
**********************************************************************
*                         本地全局变量
**********************************************************************
*/
sbit gIO = P3^7;			 			// DS18B20的单总线IO接在P3.7上

/*
**********************************************************************
*                         内部函数原型声明
**********************************************************************
*/
static void delay5ms(void);
static void delay15us(void);
static void delay45us(void);
static void delay70us(void);
static void delay750us(void);
static void delay1ms(void);
static void delay750ms(void);
static void delay500ms(void);
static void delay200ms(void);
static u8 Ds18b20Init(void);
static void Ds18b20WriteByte(u8 dat);




/************ 以下为低层时序函数 ************************************/ 

/*********************************************************************
* 函 数 名       : Ds18b20Init
* 函数功能		 : 按照DS18B20底层时序要求进行传感器初始化
* 参数列表       : 无
* 函数输出    	 : 若初始化成功则返回0，否则返回1
*********************************************************************/
static u8 Ds18b20Init(void)
{
	u8 i = 0;

	gIO = 0;			// 时序要求将总线拉低480us~960us
	delay750us();		// 实际延时750us，符合480-960之间的条件
	gIO = 1;			// 然后拉高总线，如果DS18B20做出反应会将在15us~60us后总线被拉低

	i = 0;
	while (gIO)			// 等待DS18B20拉低总线
	{
		i++;
		if(i>5)			// 等待 15*5=75us，如果还没拉低则可以认为初始化失败了
		{
			return 1;	// 初始化失败
		}
		delay15us();	// 隔15us查看一下是否收到DS18B20的回应
	}
	return 0;			//初始化成功
}


/*********************************************************************
* 函 数 名       : Ds18b20WriteByte
* 函数功能		 : 按照DS18B20底层时序要求向DS18B20写入1字节数据
* 参数列表       : dat - 待写入的1字节数据
* 函数输出    	 : 无
*********************************************************************/
static void Ds18b20WriteByte(u8 dat)
{
	u16 i = 0, j = 0;

	for (j=0; j<8; j++)
	{
		gIO = 0;	     	  	// 每写入一位数据之前先把总线拉低1us
		i++;
		gIO = dat & 0x01;  		// 然后写入一个数据，从最低位开始
		delay70us();			// 时序要求最少60us
		gIO = 1;				// 然后释放总线，至少1us给总线恢复时间才能接着写入第二个数值
		dat >>= 1;
	}
}


/*********************************************************************
* 函 数 名       : Ds18b20ReadByte
* 函数功能		 : 按照DS18B20底层时序要求从DS18B20中读取1字节数据
* 参数列表       : 无
* 函数输出    	 : 返回读取到的1字节数据
*********************************************************************/
u8 Ds18b20ReadByte(void)
{
	u8 byte = 0, bi = 0;
	u16 i = 0, j = 0;
		
	for (j=8; j>0; j--)
	{
		gIO = 0;		// 先将总线拉低1us
		i++;
		gIO = 1;		// 然后释放总线
		i++;
		i++;			// 延时6us等待数据稳定
		bi = gIO;	 	// 读取数据，从最低位开始读取
		/*将byte左移一位，然后与上右移7位后的bi，注意移动之后移掉那位补0。*/
		byte = (byte >> 1) | (bi << 7);	
		//byte |= (bi << (8-j));
		delay45us();
	}				
	return byte;
}


/*************** 高层时序 *************************************/

void Ds18b20TempConvertCmd(void)
{
	Ds18b20Init();
	delay1ms();
	Ds18b20WriteByte(0xcc);		// 跳过ROM操作命令		 
	Ds18b20WriteByte(0x44);	    // 温度转换命令
	delay200ms();				// 等待转换成功
}

void Ds18b20TempReadCmd(void)
{	
	Ds18b20Init();
	delay1ms();
	Ds18b20WriteByte(0xcc);	 	// 跳过ROM操作命令
	Ds18b20WriteByte(0xbe);	 	// 发送读取温度命令
}



/*************** 注意下面的延时函数都是12M晶振模式下的 ******************/
static void delay15us(void)   //误差 0us
{
    unsigned char a;
    for(a=6;a>0;a--);
}

static void delay45us(void)   //误差 0us
{
    unsigned char a;
    for(a=21;a>0;a--);
}

static void delay70us(void)   //误差 0us
{
    unsigned char a,b;
    for(b=1;b>0;b--)
        for(a=32;a>0;a--);
}


static void delay750us(void)   //误差 0us
{
    unsigned char a,b;
    for(b=83;b>0;b--)
        for(a=3;a>0;a--);
}

static void delay1ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=142;b>0;b--)
            for(a=2;a>0;a--);
}

static void delay750ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=167;c>0;c--)
        for(b=136;b>0;b--)
            for(a=15;a>0;a--);
}
static void delay500ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=23;c>0;c--)
        for(b=152;b>0;b--)
            for(a=70;a>0;a--);
}
static void delay200ms(void)   //误差 0us
{
    unsigned char a,b,c;
    for(c=4;c>0;c--)
        for(b=116;b>0;b--)
            for(a=214;a>0;a--);
    _nop_();  //if Keil,require use intrins.h
}