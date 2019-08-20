/*
**********************************************************************
*                         ͷ�ļ�����
**********************************************************************
*/
#include "Uart.h"
#include "DS18B20.h"
/*
**********************************************************************
*                         ���غ궨��
**********************************************************************
*/
/*
**********************************************************************
*                         ����ȫ�ֱ���
**********************************************************************
*/
u16 temp = 0;	 				// �����ݴ�12λ��ADֵ
u8 tmh = 0, tml = 0;			// �����ݴ�2��8λ��ADֵ
u8 i = 0,j = 0; 
u8 N = 5; //�����������


u8 Temperature_Read = 0;
u8 Receive_Flag = 0;
u8 T_Single = 0;
u8 T_Decade = 0;
u8 Time_Cnt = 0; // Time_Cnt���������жϴ���
u8 Time_Threshold = 100; //Time_Threshold�жϴﵽÿ��100*10ms = 1s

u8 Temperature_Set = 10;
double Temperature_SetNew = 0;
double Temperature_Diff = 1.3;
double Temperature = 0;	//�¶ȼ���ֵ
double Temperature_Collect = 0; //�¶Ȳɼ�ֵ
double Temperature_Temp = 0; //�¶��ݴ�ֵ
double Temperature_Sum = 0; //�¶�ȥ�������С���ֵ
double Temperature_Array[5] = {0,0,0,0,0}; //�¶������ݴ�����
double Kp = 1,Ki = 0,Kd = 10;
double Err = 0,Err_Last = 0,Err_Integral = 0,Rerr = 0;
double Rerr_allow = 0; //PID������Χ

/*
**********************************************************************
*                         ����ԭ������
**********************************************************************
*/
void delay10ms(void);
void Int0Init(void);
void Int1Init(void);
void TemperatureRead(void);
void PID(void);
void Timer0Init(void);
/*********************************************************************
* �� �� ��       : main
* ��������		 : ������
* �����б�       : ��
* �������    	 : ��
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
	   for(i = 0;i < 5;i++) //�¶Ȳɼ���ֵԤ���� 5��ֵ���д������һ��ֵ��ƽ��1s���һ��ֵ
	   {
			TemperatureRead();
			Temperature_Array[i] = Temperature_Collect;
	   }
	   
	   for(i = 0;i < N - 1;i++) //���ѭ��������������С��������
	   {
	   		for(j = 0;j < N - i - 1;j++) //�ڲ�ѭ������ÿ�ֵıȽϴ���
			{
				if(Temperature_Array[j] > Temperature_Array[j + 1]) //�����ǰֵ���ں�һ��ֵ���ͽ���
				{
					Temperature_Temp = 	Temperature_Array[j];
					Temperature_Array[j] = Temperature_Array[j + 1];
					Temperature_Array[j + 1] = Temperature_Temp;
				}	
			}
	   }
		    for(i = 1;i < N - 1;i++) //ȥ�������Сֵ   
    {       
         Temperature_Sum += Temperature_Array[i];    
    }    
     Temperature = Temperature_Sum/(N - 2); //ȡƽ�� 
	   Temperature_Read = (u8)(Temperature);
	   PID();
	   if(Rerr > Rerr_allow)
	   CT_Led2 = 0; //����
	   else
	   CT_Led2 = 1; //ֹͣ����


//	   }
	}
}

/*********************************************************************
* �� �� ��       : Timer0Init
* ��������		 : ��ʱ��0��ʼ������
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Timer0Init(void)
{
	TMOD &= 0xf0;		// T0��Ϊ��ʱ��ģʽ������TR0���ƴ�
	TMOD |= 0x01;		// T0ѡ��Ϊ����ģʽ1��16λ��ʱ����TH0 TL0ȫʹ��

	TH0 = 0xd8;			// ʹ�õ�Ƭ��С�������10ms��ʱ��TH0��TL0��ֵ
	TL0 = 0xf0;	
    ET0 = 0;			// �ȹض�ʱ��0�ж�����
	EA = 1;				// �����ж�
	TR0 = 1;			// ��ʼ��ʱ			
}



/*********************************************************************
* �� �� ��       : Timer0Isr
* ��������		 : ��ʱ��0�жϴ������
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Timer0Isr(void) interrupt 1
{
	TH0 = 0xd8;				// ʹ�õ�Ƭ��С�������10ms��ʱ��TH0��TL0��ֵ
	TL0 = 0xf0;	
	
	if (Time_Cnt++ >= 100)   // ÿ��100*10ms = 1s������LED����״̬ת��һ��
	{
		Time_Cnt = 0;		// gCnt��Ϊ��ֵ0�Կ�����һ���жϴ�������
		Led5 = !Led5;	   	// LED����ת��
		UartSendByte(Temperature_Read/10 + '0');
		UartSendByte(Temperature_Read%10 + '0');
		UartSendByte(' ');
			
	}
}
/*********************************************************************
* �� �� ��       : Int0Init
* ��������		 : ��ʼ��INT0Ϊ�½��ش��������򿪸����жϿ���
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Int0Init(void)
{
	IT0 = 1;			// ����Ϊ���ش���ģʽ���½��أ�
	EX0 = 1;			// ��INT0���ж�����	
	EA = 1;				// �����ж�	
}


/*********************************************************************
* �� �� ��       : Int1Init
* ��������		 : ��ʼ��INT1Ϊ�½��ش��������򿪸����жϿ���
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Int1Init(void)
{
	IT1 = 1;			// ����Ϊ���ش���ģʽ���½��أ�
	EX1 = 1;			// ��INT1���ж�����	
	EA = 1;				// �����ж�	
}
/*********************************************************************
* �� �� ��       : Int0Isr
* ��������		 : INT0���жϴ������(interrupt service routine)
				   ע�⣺INT0��Ӧ���жϺ�����0
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Int0Isr(void)	interrupt 0		
{
	delay10ms();	 		// �����ʱ����
	if (CT_Time == 0)			// ��ʱ10ms��S15��Ϊ�͵�ƽ֤������ȷʵ������
	{
		Led3 ^= 1;		// LED1ȡ��
		ET0 ^= 1;
		if(ET0 == 1)
		{
			UartSendByte('T');
			UartSendByte(':');
		}
	}
}
/*********************************************************************
* �� �� ��       : Int1Isr
* ��������		 : INT1���жϴ������(interrupt service routine)
				   ע�⣺INT1��Ӧ���жϺ�����2������1
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void Int1Isr(void)	interrupt 2		
{
	delay10ms();	 		    // �����ʱ����
	delay10ms();
	delay10ms();
	if (send_T == 0)			// ��ʱ10ms��S16��Ϊ�͵�ƽ֤������ȷʵ������
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
* �� �� ��       : delay10ms
* ��������		 : �ɵ�Ƭ��С����V1.3���ɵ���ʱ10ms�ľ�ȷ��ʱ����
* �����б�       : ��
* �������    	 : ��
*********************************************************************/
void delay10ms(void)   //��� 0us
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=38;b>0;b--)
            for(a=130;a>0;a--);
}

void TemperatureRead(void)
{
	Ds18b20TempConvertCmd();		// ��д��ת������
	Ds18b20TempReadCmd();			// Ȼ��ȴ�ת������Ͷ�ȡ�¶�����
	tml = Ds18b20ReadByte();		// ��ȡ�¶�ֵ��16λ���ȶ����ֽ�
	tmh = Ds18b20ReadByte();		// �ٶ����ֽ�

	temp = tml | (tmh << 8);		// Ĭ����12λ�ֱ��ʣ�ǰ��4��Sλ�Ƿ���λ
	
	// ���¶�ʱ����λΪ0������������û�п��Ǹ��¶��������Ϊ����ʵ������
	// ���������ģ����Ҫ���ǵ����¶ȵ������������Ҫ���ж�Sλ����SλΪ1��
	// �����ȥ��S��1�ټ��㣬������ֵ�Ӹ��ż��ɡ�
	Temperature_Collect = temp * 0.0625;

}

void PID(void)
{
	Err = Temperature_Set - Temperature;
	Err_Integral += Err;
	Rerr = Kp*Err + Ki*Err_Integral + Kd*(Err - Err_Last);
	Err_Last = Err;
	 
}


