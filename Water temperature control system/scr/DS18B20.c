/*
**********************************************************************
*                         ͷ�ļ�����
**********************************************************************
*/
#include "DS18B20.h"


/*
**********************************************************************
*                         ����ȫ�ֱ���
**********************************************************************
*/
sbit gIO = P3^7;			 			// DS18B20�ĵ�����IO����P3.7��

/*
**********************************************************************
*                         �ڲ�����ԭ������
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




/************ ����Ϊ�Ͳ�ʱ���� ************************************/ 

/*********************************************************************
* �� �� ��       : Ds18b20Init
* ��������		 : ����DS18B20�ײ�ʱ��Ҫ����д�������ʼ��
* �����б�       : ��
* �������    	 : ����ʼ���ɹ��򷵻�0�����򷵻�1
*********************************************************************/
static u8 Ds18b20Init(void)
{
	u8 i = 0;

	gIO = 0;			// ʱ��Ҫ����������480us~960us
	delay750us();		// ʵ����ʱ750us������480-960֮�������
	gIO = 1;			// Ȼ���������ߣ����DS18B20������Ӧ�Ὣ��15us~60us�����߱�����

	i = 0;
	while (gIO)			// �ȴ�DS18B20��������
	{
		i++;
		if(i>5)			// �ȴ� 15*5=75us�������û�����������Ϊ��ʼ��ʧ����
		{
			return 1;	// ��ʼ��ʧ��
		}
		delay15us();	// ��15us�鿴һ���Ƿ��յ�DS18B20�Ļ�Ӧ
	}
	return 0;			//��ʼ���ɹ�
}


/*********************************************************************
* �� �� ��       : Ds18b20WriteByte
* ��������		 : ����DS18B20�ײ�ʱ��Ҫ����DS18B20д��1�ֽ�����
* �����б�       : dat - ��д���1�ֽ�����
* �������    	 : ��
*********************************************************************/
static void Ds18b20WriteByte(u8 dat)
{
	u16 i = 0, j = 0;

	for (j=0; j<8; j++)
	{
		gIO = 0;	     	  	// ÿд��һλ����֮ǰ�Ȱ���������1us
		i++;
		gIO = dat & 0x01;  		// Ȼ��д��һ�����ݣ������λ��ʼ
		delay70us();			// ʱ��Ҫ������60us
		gIO = 1;				// Ȼ���ͷ����ߣ�����1us�����߻ָ�ʱ����ܽ���д��ڶ�����ֵ
		dat >>= 1;
	}
}


/*********************************************************************
* �� �� ��       : Ds18b20ReadByte
* ��������		 : ����DS18B20�ײ�ʱ��Ҫ���DS18B20�ж�ȡ1�ֽ�����
* �����б�       : ��
* �������    	 : ���ض�ȡ����1�ֽ�����
*********************************************************************/
u8 Ds18b20ReadByte(void)
{
	u8 byte = 0, bi = 0;
	u16 i = 0, j = 0;
		
	for (j=8; j>0; j--)
	{
		gIO = 0;		// �Ƚ���������1us
		i++;
		gIO = 1;		// Ȼ���ͷ�����
		i++;
		i++;			// ��ʱ6us�ȴ������ȶ�
		bi = gIO;	 	// ��ȡ���ݣ������λ��ʼ��ȡ
		/*��byte����һλ��Ȼ����������7λ���bi��ע���ƶ�֮���Ƶ���λ��0��*/
		byte = (byte >> 1) | (bi << 7);	
		//byte |= (bi << (8-j));
		delay45us();
	}				
	return byte;
}


/*************** �߲�ʱ�� *************************************/

void Ds18b20TempConvertCmd(void)
{
	Ds18b20Init();
	delay1ms();
	Ds18b20WriteByte(0xcc);		// ����ROM��������		 
	Ds18b20WriteByte(0x44);	    // �¶�ת������
	delay200ms();				// �ȴ�ת���ɹ�
}

void Ds18b20TempReadCmd(void)
{	
	Ds18b20Init();
	delay1ms();
	Ds18b20WriteByte(0xcc);	 	// ����ROM��������
	Ds18b20WriteByte(0xbe);	 	// ���Ͷ�ȡ�¶�����
}



/*************** ע���������ʱ��������12M����ģʽ�µ� ******************/
static void delay15us(void)   //��� 0us
{
    unsigned char a;
    for(a=6;a>0;a--);
}

static void delay45us(void)   //��� 0us
{
    unsigned char a;
    for(a=21;a>0;a--);
}

static void delay70us(void)   //��� 0us
{
    unsigned char a,b;
    for(b=1;b>0;b--)
        for(a=32;a>0;a--);
}


static void delay750us(void)   //��� 0us
{
    unsigned char a,b;
    for(b=83;b>0;b--)
        for(a=3;a>0;a--);
}

static void delay1ms(void)   //��� 0us
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=142;b>0;b--)
            for(a=2;a>0;a--);
}

static void delay750ms(void)   //��� 0us
{
    unsigned char a,b,c;
    for(c=167;c>0;c--)
        for(b=136;b>0;b--)
            for(a=15;a>0;a--);
}
static void delay500ms(void)   //��� 0us
{
    unsigned char a,b,c;
    for(c=23;c>0;c--)
        for(b=152;b>0;b--)
            for(a=70;a>0;a--);
}
static void delay200ms(void)   //��� 0us
{
    unsigned char a,b,c;
    for(c=4;c>0;c--)
        for(b=116;b>0;b--)
            for(a=214;a>0;a--);
    _nop_();  //if Keil,require use intrins.h
}