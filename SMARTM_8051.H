#ifndef __SMARTM_8051_H__
#define __SMARTM_8051_H__
#include <stdio.h>
#include <stdlib.h>
#include "8051.h"
#include <intrins.h>
#include "Typedef.h"
#include "Common.h"
#include "Key.h"
#include "EEPROM.h"
#include "UART.h"
//#include "delay.h"
//#include "LCD.h"
#include "PWM.h"


//#include "spi.h"
//#include "flash.h"

//�ܽŶ���  
//�ٶȱ����� P1.0
//PWM���    P1.1


#define KEY_PORT            P1     //����IO
#define KEY_PORT_MASK       0xFC   //�������룬ʹ��2.3.4.5.6.7�����ܽ�

//sbit	 LED1=P5^5;//P2^6;
sbit	 LED2=P5^4;
sbit	 BELL=P2^6;							//������
sbit	 MOTO_KEY=P2^7;			//���ʹ�ܣ����˵�Ƭ���ϵ�ʱ�ܽŸߵ�ѹ���µ�����
sbit	 LID_UP=P3^3;							//�ż�⿪��  �͵�ƽ����
//sbit   TIME_UP=P1^2;
//sbit   TIME_DOWN=P1^3;


sbit PSB=P2^0;//����ʱ��PSB=1;����ʱ��PSB=0
sbit E2=P2^1; 		//����Ϊʱ��SCLK
sbit E1=P2^2;  		//����Ϊʱ��SCLK
sbit RW=P2^3;		 	//����ΪSID
sbit RS=P2^4;			//����ʱΪCS
sbit REST=P2^5;

#define MOTOR_ENABLE  MOTO_KEY=1;
#define MOTOR_DISABLE MOTO_KEY=0;
#define Bell_Run 			BELL=1;
#define Bell_Stop			BELL=0;

//EEPROM��ַ
#define IAP_ADDRESS 0xF000
#define TIME_HIGH_ADDRESS  IAP_ADDRESS
#define TIME_LOW_ADDRESS  IAP_ADDRESS+2
//#define SPEED_HIGH_ADDRESS  IAP_ADDRESS+0x200   //0xF200
//#define SPEED_LOW_ADDRESS   IAP_ADDRESS+0x202  	//SPEED_HIGH_ADDRESS+2
#define SPEED_HIGH_ADDRESS  IAP_ADDRESS+4   //0xF200
#define SPEED_LOW_ADDRESS   IAP_ADDRESS+6  	//SPEED_HIGH_ADDRESS+2

//�豸����״̬��
#define MOTO_STOP   0
#define MOTO_RUN    1
#define MOTO_PASUE  2
#define LID_UP      3
#define MOTO_START  4
#define MOTO_ERROR  5
//static UINT8 MOTO_STOP=0;
//static UINT8 MOTO_RUN=1;
//static UINT8 MOTO_PASUE=2;
//static UINT8 MOTO_START=3;
//static UINT8 LID_UP=4;

#define INIT      0                   /* task number of task:  init           */
#define DISPLAY   1                   /* task number of task:  command        */
#define CLOCK     2                   /* task number of task:  clock          */
#define COUNTER   3                   /* task number of task:  counter       */
#define MOTO    4                   /* task number of task:  signal         */
#define KEYREAD   5                   /* task number of task:  keyread        */
#define GET_ESC   6                   /* task number of task:  get_escape     */

//PWM��ر���

#define SYSCLK_DIV_1  0x09     				//����CFλ���ж�
#define SYSCLK_DIV_2  0x03
#define SYSCLK_DIV_4  0x0A
#define SYSCLK_DIV_6  0x0D
#define SYSCLK_DIV_8  0x0E
#define SYSCLK_DIV_12 0x00



struct time  {                        /* structure of the time record         */
  UINT8 hour;                 /* hour                                 */
  UINT8 min;                  /* minute                               */
  UINT8 sec;                  /* second                               */
};


#endif