#ifndef __Uart_H__
#define __Uart_H__

/*
**********************************************************************
*                         ͷ�ļ�����
**********************************************************************
*/
#include "common.h"


/*
**********************************************************************
*                         ���غ궨��
**********************************************************************
*/
//#define LCD1602_DATA_PORT	 P0			// LCD1602��8λ���ݶ˿�


/*
**********************************************************************
*                         �ⲿ����ԭ������
**********************************************************************
*/

void UartInit(void);
void UartSendByte(u8 c);

#endif