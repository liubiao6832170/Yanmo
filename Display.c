

#include "SmartM_8051.h"
#include <rtx51tny.h>                 /* RTX-51 tiny functions & defines      */

extern UINT8  MOTO_STATE;
extern UINT16 RUN_TIME,MOTOR_SPEED, RUN_TIME_TEMP;

void Diaplay_Init();
void Display_Time1();
void Diaplay_SUB();
void WaitNms(UINT16 n);

//sbit PSB=P2^0;//并口时，PSB=1;串口时，PSB=0
//sbit REST=P2^1;
//sbit E2=P2^2; 		//串口为时钟SCLK
//sbit E1=P2^3;  		//串口为时钟SCLK
//sbit RS=P2^4;			//串口时为CS
//sbit RW=P2^5;		 	//串口为SID
//sbit stop=P3^2;


typedef unsigned int Uint;
typedef unsigned char Uchar;


UINT8 Dis_Speed_Data[]="1750";
UINT8 Dis_Time_Data_H[2]="15";
UINT8 Dis_Time_Data_L[3]="32";


Uchar m1,num,ii,z,z1,d,d1,s,s1,s10,s100;
Uchar code  num_8x16[11][16];//宋体12
Uchar code  num_16x32[11][62];//DS-Digital24
Uchar code  num_24x48[11][144];//宋体36

//这个是在串口时指令和数据之间的延时

void delay10US(Uchar x)
{
    Uchar k;
		for(k=0;k<x;k++);
 }
/**/
const Uchar delay=250;  //延时时间常数
static void Wait1ms(void)//延迟1 ms
{
		Uchar cnt=0;
		while (cnt<delay) cnt++;
}
//延迟n ms 
void WaitNms(Uint n)
{
    Uint i;
    for(i=1;i<=n;i++)
   	Wait1ms();
}

void time_nms(Uint x)//0.5ms
{
	Uchar j;
 while(x--)
  {for(j=0;j<50;j++)
    {;}
  }
}


/////////////////////////////////////////////////////////////////////////////////
//以下是串口时开的读写时序


void SendByteLCDH(Uchar WLCDData)
{
	 Uchar i;
	 for(i=0;i<8;i++)
	 {
			 if((WLCDData<<i)&0x80)RW=1;
			 else RW=0;
			 E1=0; 
			 E1=1 ;
	 }
}


 SPIWH(Uchar Wdata,Uchar WRS)
 {
		 SendByteLCDH(0xf8+(WRS<<1));//寄存器选择WRS
		 SendByteLCDH(Wdata&0xf0);
		 SendByteLCDH((Wdata<<4)&0xf0);
 }
 

void WRCommandH(Uchar CMD)
{
			RS=0;
			RS=1;
			SPIWH(CMD,0);
			delay10US(90);//89S52来模拟串行通信,所以,加上89S52的延时,
}



void WRDataH(Uchar Data)
{ 
  RS=0;
  RS=1;
  SPIWH(Data,1);

}


void SendByteLCDL(Uchar WLCDData)
{
	 Uchar i;
	 for(i=0;i<8;i++)
	 {
			 if((WLCDData<<i)&0x80)RW=1;
			 else RW=0;
			 E2=0; 
			 E2=1 ;
	 }
}

 
 SPIWL(Uchar Wdata,Uchar WRS)
 {
			 SendByteLCDL(0xf8+(WRS<<1));
			 SendByteLCDL(Wdata&0xf0);
			 SendByteLCDL((Wdata<<4)&0xf0);
 }

 
void WRCommandL(Uchar CMD)
{
			RS=0;
			RS=1;
			SPIWL(CMD,0);
			delay10US(90);//89S52来模拟串行通信,所以,加上89S52的延时,
}




void WRDataL(Uchar Data)
{ 
  RS=0;
  RS=1;
  SPIWL(Data,1);

}



void ShowNUMCharH(Uchar addr,Uchar i,Uchar count)
{
     Uchar j;
			for(j=0;j<count;)
			{	
					WRCommandH(addr);	//设定DDRAM地址
					WRDataH(i+j);//必为两个16*8位字符拼成一个16*16才能显示
					j++;
					WRDataH(i+j);
					addr++;
					j++;
			}		
}
//下半屏显示连续字串(半宽字符)
void ShowNUMCharL(Uchar addr,Uchar i,Uchar count)
{
     Uchar j;
			for(j=0;j<count;)
			{	
					WRCommandL(addr);	//设定DDRAM地址
					WRDataL(i+j);
					j++;
					WRDataL(i+j);
					addr++;
					j++;
			}		
}

void WRCGRAM(Uchar data1,Uchar data2,Uchar addr)
{     
      Uchar i;
      for(i=0;i<16;)
			{
					WRCommandH(addr+i); WRCommandL(addr+i);  	//设定CGRAM地址
					WRDataH(data1); WRDataL(data1);
					WRDataH(data1); WRDataL(data1);
					i++;
					WRCommandH(addr+i);  WRCommandL(addr+i);  	//设定CGRAM地址
					WRDataH(data2); WRDataL(data2);
					WRDataH(data2); WRDataL(data2);
					i++;
			}  
}


void ShowCGChar(Uchar addr,Uchar i)
{
     Uchar j;
	for(j=0;j<0x20;)
	{	
	    WRCommandH(addr+j);	//设定DDRAM地址
	    WRCommandL(addr+j);	//设定DDRAM地址
			WRDataL(0x00);
	    WRDataL(i);		
			WRDataH(0x00);//字符地址低八位
	    WRDataH(i);//字符地址高八位
			j++;
	}		
}

/****************************************
*函数名称:reg_h
*输    入:x：起始地址：80 81 82...9E
					y:纵向向下偏移点数
					x2:横向向向重复次数（隔一个反白一次）
					y2:y轴方向上点数 16 ，32
					d1,d2:数据内容，0x00 0x00洗白，0xff,0x00 反白，0xff 0xff全黑
*输    出:无
*功    能:上半屏刷点阵
******************************************/

void reg_h(Uchar x,Uchar y,Uchar x2,Uchar y2,Uchar d1,Uchar d2 )
{    
	   Uchar j,i;	    
	   WRCommandH(0x34);			//去扩展指令寄存器
	   WRCommandH(0x36);			//打开绘图功能
	   for(j=0;j<y2;j++)				//2行   画两横上边框
		  {   
			   WRCommandH(0x80+y+j);  //Y总坐标,即第几行
			   WRCommandH(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节
			   for(i=0;i<x2;i++)	//写入一行
			  {
					 WRDataH(d1);
					 WRDataH(d2);
			  }
		  }		  
}

void reg_l(Uchar x,Uchar y,Uchar x2,Uchar y2,Uchar d1,Uchar d2 )
{    
	   Uchar j,i;	    
	   WRCommandL(0x34);			//去扩展指令寄存器
	   WRCommandL(0x36);			//打开绘图功能
	   for(j=0;j<y2;j++)				//2行   画两横上边框
		  {   
					 WRCommandL(0x80+y+j);  //Y总坐标,即第几行
					 WRCommandL(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节
			   for(i=0;i<x2;i++)	//写入一行
			  {
					 WRDataL(d1);
					 WRDataL(d2);
			  }
		  }	  
}
void display_rom_8x16(bit y,Uchar column,Uchar *text)
{
			Uint i=0,j;

			if(y==0)
			{
				WRCommandH(column);	
				while(text[i]>0x00)
				{
				if((text[i]>=0x20)&&(text[i]<0x7e))	
				{
						j=text[i];
						WRDataH(j);
						i++;
						j=text[i];		
						WRDataH(j);
						WaitNms(10);////延时 x ms
						i++;
				}	
				else
				i++;
				}
			}
			else
			{	
				WRCommandL(column);	
				while(text[i]>0x00)
				{
				if((text[i]>=0x20)&&(text[i]<0x7e))	
				{
						j=text[i];
						WRDataL(j);
						i++;		
						j=text[i];
						WRDataL(j);
						WaitNms(10);////延时 x ms
					i++;
				}	
				else
				i++;
				}
}	
		
}

void ShowText(bit y,Uchar column,Uchar *text)
{
		if(y==0)
			{
				WRCommandH(column);	
				while(*text>0) 
					{ 
					 WRDataH(*text); 
					 text++; 
					} 
			}
		else{
				WRCommandL(column);	
				while(*text>0) 
					{ 
					 WRDataL(*text); 
					 text++; 
					} 
			}
}	 
		
void fb1234(Uchar xn)
{
			reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
			reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
		if(xn==1)
			{	
			reg_h(0x80,0,12,16,0xff,0xff);			//单行反白	
			}
		else if (xn==2)
			{
			reg_h(0x80,16,12,16,0xff,0xff);			//单行反白	
			}
		else if(xn==3)
			{
			reg_l(0x80,0,12,16,0xff,0xff);			//单行反白		
			}
		else if(xn==4)
			{
			reg_l(0x80,16,12,16,0xff,0xff);			//单行反白	
			}	
		else
			_nop_();
}		
/*	
void time_rom_8x16(Uchar column,Uchar z,Uchar z1,Uchar d,Uchar d1,Uchar s,Uchar s1,Uchar s10,Uchar s100)
{
//	Uchar j;			
	WRCommandH(column);		
	WRDataH(z+48);	
	WRDataH(z1+48);				
	WRDataH(':');	
	WRDataH(d+48);	
	WRDataH(d1+48);				
	WRDataH(':');	
	WRDataH(s+48);	
	WRDataH(s1+48);				
	WRDataH(':');	
	WRDataH(s10+48);	
	WRDataH(s100+48);										
}
*/
void time_ram_8x16(Uchar x,Uchar y,Uchar z,Uchar z1,Uchar d,Uchar d1,Uchar s,Uchar s1,Uchar s10,s100)
{
			Uchar k;
			WRCommandH(0x34);			//去扩展指令寄存器
			WRCommandH(0x36);			//打开绘图功能
			for(k=0;k<16;k++)
			{						
					WRCommandH(0x80+y+k);  //Y总坐标,即第几行
					WRCommandH(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
					WRDataH(num_8x16[z][k]);
					WRDataH(~(num_8x16[z1][k]));
					WRDataH(num_8x16[10][k]);
					WRDataH(num_8x16[d][k]);
					WRDataH(~(num_8x16[d1][k]));
					WRDataH(num_8x16[10][k]);
					WRDataH(num_8x16[s][k]);
					WRDataH(~(num_8x16[s1][k]));
					WRDataH(num_8x16[10][k]);
					WRDataH(num_8x16[s10][k]);	
					WRDataH(~(num_8x16[s100][k]));	
	}							
}

/******************************************************************************/	
//void time_ram_16x32(Uchar x,Uchar y,Uchar d,Uchar d1,Uchar s,Uchar s1)
//{
//			Uchar k;
//			WRCommandH(0x34);			//去扩展指令寄存器
//			WRCommandH(0x36);			//打开绘图功能
//			for(k=0;k<31;k++)
//			{						
//						WRCommandH(0x80+y+k);  //Y总坐标,即第几行
//						WRCommandH(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
//						WRDataH(num_16x32[d][k*2]);		WRDataH(num_16x32[d][k*2+1]);
//						WRDataH(num_16x32[d1][k*2]);	WRDataH(num_16x32[d1][k*2+1]);
//						WRDataH(num_16x32[10][k*2]);	WRDataH(num_16x32[10][k*2+1]);
//						WRDataH(num_16x32[s][k*2]);		WRDataH(num_16x32[s][k*2+1]);
//						WRDataH(num_16x32[s1][k*2]);	WRDataH(num_16x32[s1][k*2+1]);	
//			}						
//}	
/******************************************************************************/	
void time_ram_16x32(Uchar x,Uchar y,Uchar d,Uchar d1,Uchar s,Uchar s1)
{
			Uchar k;
			WRCommandL(0x34);			//去扩展指令寄存器
			WRCommandL(0x36);			//打开绘图功能
			for(k=0;k<31;k++)
			{						
						WRCommandL(0x80+y+k);  //Y总坐标,即第几行
						WRCommandL(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
						WRDataL(num_16x32[d][k*2]);		WRDataL(num_16x32[d][k*2+1]);
						WRDataL(num_16x32[d1][k*2]);	WRDataL(num_16x32[d1][k*2+1]);
						WRDataL(num_16x32[10][k*2]);	WRDataL(num_16x32[10][k*2+1]);
						WRDataL(num_16x32[s][k*2]);		WRDataL(num_16x32[s][k*2+1]);
						WRDataL(num_16x32[s1][k*2]);	WRDataL(num_16x32[s1][k*2+1]);	
			}						
}	
	
/******************************************************************************/
//void time_ram_24x48(Uchar x,Uchar y,Uchar z,Uchar z1,Uchar d,Uchar d1)
//{
//	Uchar k,j;
//	WRCommandH(0x34);	WRCommandL(0x34);			//去扩展指令寄存器
//	WRCommandH(0x36);	WRCommandL(0x36);			//打开绘图功能
//	for(k=0;k<48;k++)
//	{	
//		if(k<32)
//		{
//		WRCommandH(0x80+y+k);  //Y总坐标,即第几行
//		WRCommandH(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
//		for(j=0;j<3;j++) 	{WRDataH(num_24x48[z][k+k*2+j]);}
//		for(j=0;j<3;j++) 	{WRDataH(~(num_24x48[z1][k+k*2+j]));}			
//		for(j=0;j<3;j++) 	{WRDataH(num_24x48[d][k+k*2+j]);}
//		for(j=0;j<3;j++) 	{WRDataH(~(num_24x48[d1][k+k*2+j]));}
//		}
//		else
//		{
//		WRCommandL(0x80+y+k-32);  //Y总坐标,即第几行
//		WRCommandL(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
//		for(j=0;j<3;j++) 	{WRDataL(num_24x48[z][k+k*2+j]);}
//		for(j=0;j<3;j++) 	{WRDataL(~(num_24x48[z1][k+k*2+j]));}				
//		for(j=0;j<3;j++) 	{WRDataL(num_24x48[d][k+k*2+j]);}
//		for(j=0;j<3;j++) 	{WRDataL(~(num_24x48[d1][k+k*2+j]));}
//		}			
//	}						
//}

/******************************************************************************/
void time_ram_24x48(Uchar x,Uchar y,Uchar z,Uchar z1,Uchar z2,Uchar d,Uchar d1)
{
			Uchar k,j;
			WRCommandH(0x34);	WRCommandL(0x34);			//去扩展指令寄存器
			WRCommandH(0x36);	WRCommandL(0x36);			//打开绘图功能
			for(k=0;k<48;k++)
			{	
				if(k<32)
				{
						WRCommandH(0x80+y+k);  //Y总坐标,即第几行
						WRCommandH(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
						for(j=0;j<3;j++) 	{WRDataH(num_24x48[z][k+k*2+j]);}
						for(j=0;j<3;j++) 	{WRDataH((num_24x48[z1][k+k*2+j]));}		
						for(j=0;j<3;j++) 	{WRDataH((num_24x48[z2][k+k*2+j]));}			
						for(j=0;j<3;j++) 	{WRDataH(num_24x48[d][k+k*2+j]);}
						for(j=0;j<3;j++) 	{WRDataH((num_24x48[d1][k+k*2+j]));}
				}
				else
				{
						WRCommandL(0x80+y+k-32);  //Y总坐标,即第几行
						WRCommandL(x);	//X坐标，即横数第几个字节开始写起,80H为第一个字节				
						for(j=0;j<3;j++) 	{WRDataL(num_24x48[z][k+k*2+j]);}
						for(j=0;j<3;j++) 	{WRDataL((num_24x48[z1][k+k*2+j]));}		
						for(j=0;j<3;j++) 	{WRDataL((num_24x48[z2][k+k*2+j]));}				
						for(j=0;j<3;j++) 	{WRDataL(num_24x48[d][k+k*2+j]);}
						for(j=0;j<3;j++) 	{WRDataL((num_24x48[d1][k+k*2+j]));}
				}			
			}						
}			


//初始化LCD-8位接口
void LCDInit(void)
{ 	
	  PSB=0; //串口
//    PSB=1;//并口时选这个,上一行取消
    REST=1;
    REST=0;
    REST=1;

  	WRCommandH(0x30);	//基本指令集,8位并行
  	WRCommandL(0x30);	//基本指令集,8位并行

		WRCommandH(0x06);	//启始点设定：光标右移
		WRCommandL(0x06);	//启始点设定：光标右移

		WRCommandH(0x01);	//清除显示DDRAM
		WRCommandL(0x01);	//清除显示DDRAM

		WRCommandH(0x0C);	//显示状态开关：整体显示开，光标显示关，光标显示反白关
		WRCommandL(0x0C);	//显示状态开关：整体显示开，光标显示关，光标显示反白关

		WRCommandH(0x02);	//地址归零	
		WRCommandL(0x02);	//地址归零	
	
//	WaitNms(250);
//	reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
//	reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 	
//	WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
//	
//	ShowText(1,0x95,"您好! 欢迎光临");//引用汉字串显示
//	
}
/*******************************************************************/
void Display_Time1(void)
{	
							os_delete_task(DISPLAY);
			WaitNms(50);
			reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
			reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
			WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
			LCDInit();//初始化		
			if(MOTO_STATE==MOTO_RUN)
				{
							ShowText(0,0x91,"Run  ");
				}else if(MOTO_STATE==MOTO_START)
				{
							ShowText(0,0x91,"Run  ");
				}	else	if(MOTO_STATE==MOTO_PASUE)
				{
//							LCDInit();//初始化		
//							ShowText(0,0x91,"Pause");
							ShowText(0,0x91,"Pause");
				}else	if(MOTO_STATE==LID_UP)
				{
//							LCDInit();//初始化		
//							ShowText(0,0x91,"LID_UP");
							ShowText(0,0x91,"LID_UP");
				}else	if(MOTO_STATE==MOTO_STOP)
				{
//							LCDInit();//初始化		
//							ShowText(0,0x91,"STOP ");
							ShowText(0,0x91,"STOP ");
				}else	if(MOTO_STATE==MOTO_ERROR)
				{
							ShowText(0,0x91,"Error");
				}else	
				{
//							LCDInit();//初始化		
//							ShowText(0,0x91,"STOP ");
							ShowText(0,0x91,"STOP ");
				} 
					ShowText(0,0x80,"2458:");//引用汉字串显示
					ShowText(0,0x80,"STATE:");//引用汉字串显示
				sprintf(Dis_Speed_Data,"%4d",MOTOR_SPEED);
				sprintf(Dis_Time_Data_H,"%02d",RUN_TIME/60);
				sprintf(Dis_Time_Data_L,":%02d",RUN_TIME%60);
			
				ShowText(1,0x92,"rpm");//引用汉字串显示
				ShowText(1,0x99," min");//引用汉字串显示
				ShowText(1,0x90,Dis_Speed_Data);
				sprintf(Dis_Time_Data_H,"%02d",RUN_TIME_TEMP/60);
				sprintf(Dis_Time_Data_L,":%02d",RUN_TIME_TEMP%60);
				ShowText(1,0x97,Dis_Time_Data_H);
				ShowText(1,0x98,Dis_Time_Data_L);
				os_create_task (DISPLAY); 
//				time_ram_16x32(0x80,0,d,d1,s,s1);
////				ShowText(1,0x95,"您好! 欢迎光临");//引用汉字串显示
//				time_ram_16x32(0x84,0,RUN_TIME/60/10,RUN_TIME/60%10,RUN_TIME%60/10,RUN_TIME%60%10);
//				while(RUN_TIME>0)
//				{
//							time_ram_24x48(0x84,0,RUN_TIME/60/10,RUN_TIME/60%10,10,RUN_TIME%60/10,RUN_TIME%60%10);
//				}
//				Diaplay_SUB();
}
/*******************************************************************/
void Diaplay_SUB(void)
{
				time_ram_24x48(0x84,0,RUN_TIME/60/10,RUN_TIME/60%10,10,RUN_TIME%60/10,RUN_TIME%60%10);
}
/*******************************************************************/
void Diaplay_Init(void)
{	
//				WaitNms(50);		//等待时间		
//////			reg_h(0x80,0,4,32,0x00,0x00);			//绘图演示-洗屏	
//////			reg_l(0x90,0,8,32,0x00,0x00);			//绘图演示-洗屏 	
//			reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
//			reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 	
//			WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
//			LCDInit();//初始化		
//					ShowText(0,0x80,"2458:");//引用汉字串显示
//					ShowText(0,0x80,"STATE:");//引用汉字串显示
//					if(MOTO_STATE==MOTO_RUN)
//						{
//									ShowText(0,0x91,"Run");
//						}	else	if(MOTO_STATE==MOTO_PASUE)
//						{
//									ShowText(0,0x91,"Pause");
//						}else	if(MOTO_STATE==LID_UP)
//						{
//									ShowText(0,0x91,"LID_UP");
//						}else	if(MOTO_STATE==MOTO_STOP)
//						{
//									ShowText(0,0x91,"STOP");
//						}else	
//						{
//									ShowText(0,0x91,"STOP");
//						}
//						sprintf(Dis_Speed_Data,"%4d",MOTOR_SPEED);

////					
////						ShowText(1,0x80,"ABCDEFGHIJKLMNOPQR");//引用汉字串显示
////						
//						ShowText(1,0x92,"rpm");//引用汉字串显示
//						ShowText(1,0x99," min");//引用汉字串显示
//						ShowText(1,0x90,Dis_Speed_Data);
//						ShowText(1,0x97,Dis_Time_Data_H);
//						ShowText(1,0x98,Dis_Time_Data_L);
//		//				ShowText(1,0x95,"您好! 欢迎光临");//引用汉字串显示
//						
//						time_ram_24x48(0x84,0,RUN_TIME/60/10,RUN_TIME/60%10,10,RUN_TIME%60/10,RUN_TIME%60%10);

//		ini_int1();//开中断


	WaitNms(250);
//	LCDInit();//初始化

//for(m1=0;m1<3;m1++)
	{	
				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
				reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
//				LCDInit();//初始化		
//				ShowText(0,0x94,"绘晶科技");//引用汉字串显示	
//				ShowText(0,0x80,"绘晶科技");//引用汉字串显示	
//				ShowText(0,0x80,"       ");//引用汉字串显示	
//				ShowText(0,0x94,"莱普艾克");//引用汉字串显示	
//				ShowText(1,0x84," 192x64");//引用汉字串显示		
//				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	(x,y,x2,y2,d1,d2)
//				reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 (x,y,x2,y2,d1,d2)			
//				reg_h(0x80,0,12,2,0xff,0xff);			//绘图演示-画上边框 (x,y,x2,y2,d1,d2)
//				reg_h(0x80,2,1,30,0xc0,0x00);			//绘图演示-画上左边框(x,y,x2,y2,d1,d2) 
//				reg_h(0x80+11,2,1,30,0x00,0x03);		//绘图演示-画上右边框 (x,y,x2,y2,d1,d2)		
//				reg_l(0x80,29,12,2,0xff,0xff);			//绘图演示-画下边框 (x,y,x2,y2,d1,d2)
//				reg_l(0x80,0,1,29,0xc0,0x00);			//绘图演示-画下左边框 (x,y,x2,y2,d1,d2)
//				reg_l(0x80+11,0,1,29,0x00,0x03);		//绘图演示-画下右边框 (x,y,x2,y2,d1,d2)			
//				WaitNms(1250);		//等待时间
//				WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
//				WRCommandL(0x34);	WRCommandL(0x34);	//关绘图	
//			//-----------------------------------------------------------------------汉字加图形	
//				WRCommandH(0x01);	WRCommandL(0x01);		
//				ShowText(0,0x94,"绘晶科技");//引用汉字串显示	
//				ShowText(1,0x84," 192X64");//引用汉字串显示	
//				reg_h(0x80,0,12,32,0xff,0xff);			//绘图演示-全屏反白	
//				reg_l(0x80,0,12,32,0xff,0xff);			//绘图演示-全屏反白 
//				WaitNms(1250);		//等待时间	
//			//-----------------------------------------------------------------------全屏反白	
//					
//				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
//				reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
//				LCDInit();//初始化	
//					ShowNUMCharH(0x80,0x01,24);//显示半宽特殊符号（地址，首字符，一行个数）
//				ShowNUMCharH(0x90,0x30,24);//显示半宽0~?数字标点
//				ShowNUMCharL(0x80,0x41,24);//显示半宽A~P大写
//				ShowNUMCharL(0x90,0x61,24);//显示半宽a~p小写
//				WaitNms(1250);		//等待时间						
//			//-----------------------------------------------------------------------调ASCII-ROM，	
//				LCDInit();//初始化	
//				WRCGRAM(0xff,0x00,0x40);//写入横（2个8位，2个8位，自编地址）
//				WRCGRAM(0x00,0xff,0x50);//写入横2
//				WRCGRAM(0xaa,0xaa,0x60);//写入竖
//				WRCGRAM(0x55,0x55,0x70);//写入竖2------------自编符号
//					ShowCGChar(0x80,0x00);//显示横并填满（显示地址，自编数据地址）
//				WaitNms(1350);		//等待时间-----------------横1
//				WRCommandH(0x01);	WRCommandL(0x01);
//					ShowCGChar(0x80,02);//显示横2并填满
//				WaitNms(1350);		//等待时间-----------------横
//				WRCommandH(0x01);	WRCommandL(0x01);
//				ShowCGChar(0x80,04);//显示竖并填满
//				WaitNms(1250);		//等待时间-----------------竖1
//				WRCommandH(0x01);	WRCommandL(0x01);
//				ShowCGChar(0x80,06);//显示竖2并填满
//				WaitNms(1250);		//等待时间-----------------竖

//				WRCommandH(0x01);	WRCommandL(0x01);
//				WRCGRAM(0x00,0x00,0x40);//清CGRAM1
//				WRCGRAM(0x00,0x00,0x50);//清CGRAM2------------清自编符号
//				WRCGRAM(0xaa,0x55,0x40);//写入点
//				WRCGRAM(0x55,0xaa,0x50);//写入点2
//				ShowCGChar(0x80,00);//显示点并填满
//				WaitNms(1250);		//等待时间-----------------点1
//				WRCommandH(0x01);	WRCommandL(0x01);
//				ShowCGChar(0x80,02);//显示点2并填满
//				WaitNms(1250);		//等待时间-----------------点
//			//-----------------------------------------------------------------------扫描横竖点
//				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
//				reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
//				LCDInit();//初始化	
//				display_rom_8x16(0,0x81,"HUIJINGKEJI:19264ZW\0");//(0为上半屏,地址,字符串）
//				display_rom_8x16(0,0x91,"PHONE:18923422341\0");	
//				display_rom_8x16(1,0x81,"TEL: 0755-23146001\0");
//				display_rom_8x16(1,0x91,"www.huijinglcm.com\0 ");
//				reg_h(0x85,0,1,32,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)
//				reg_l(0x85,0,1,16,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)
//				reg_h(0x86,17,1,16,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)
//				reg_l(0x86,0,1,32,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)	
//				reg_h(0x87,17,1,16,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)	
//				reg_l(0x87,0,1,16,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)	
//				reg_h(0x88,0,1,32,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)
//				reg_l(0x88,0,1,32,0xff,0x00);			//绘图演示-单字反白 (x,y,x2,y2,d1,d2)	
//				WaitNms(1250);		//等待时间
//				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏		
//				WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
			//-----------------------------------------------------------------------引用字符串演示	
				reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
				reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 
				LCDInit();//初始化	
				ShowText(0,0x91,"Tissue/Grinder 2020");//引用汉字串显示
//				ShowText(0,0x95,"2020");//引用汉字串显示
				WaitNms(2000);	
////				ShowText(1,0x80,"19264 带中文字库显示器");//引用汉字串显示
				ShowText(1,0x93,"Initializing..");//引用汉字串显示	
				fb1234(4);
				WaitNms(5000);	
				fb1234(4);
				WaitNms(5000);	
//				fb1234(4);
//				WaitNms(5000);	
//				ShowText(0,0x80,"深圳绘晶科技有限公司");//引用汉字串显示
//				ShowText(1,0x80,"青岛莱普艾克电子有限公司");//引用汉字串显示
////				fb1234(3);
//				WaitNms(2000);	
//				ShowText(1,0x80,"                      ");//引用汉字串显示
//				ShowText(1,0x82,"实验室前端处理专家");//引用汉字串显示
////				fb1234(3);
//				WaitNms(200);	
//				ShowText(0,0x80,"青岛莱普艾克电子有限公司");//引用汉字串显示
//				ShowText(0,0x92,"实验室前端处理专家");//引用汉字串显示
////				fb1234(3);
//				fb1234(1);	WaitNms(200);		//等待时间	
//				ShowText(1,0x80,"青岛莱普艾克电子有限公司");//引用汉字串显示
//				ShowText(1,0x92,"实验室前端处理专家");//引用汉字串显示
//				fb1234(1);	WaitNms(200);		//等待时间	
//				ShowText(0,0x90,"青岛莱普艾克电子有限公司");//引用汉字串显示
//				ShowText(1,0x82,"实验室前端处理专家");//引用汉字串显示
//				fb1234(2);	WaitNms(200);		//等待时间	
//				fb1234(3);	WaitNms(200);		//等待时间	
//				fb1234(4);	WaitNms(250);		//等待时间		
			//-----------------------------------------------------------------------调用汉字/单行反白						
//						LED1=!LED1;
//						LED2=!LED2;
	}

//				WaitNms(50);		//等待时间		
//		reg_h(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏	
//		reg_l(0x80,0,12,32,0x00,0x00);			//绘图演示-洗屏 	
//		WRCommandH(0x34);	WRCommandH(0x34);	//关绘图	
//		LCDInit();//初始化		
//	ShowText(0,0x80,"深圳绘晶科技");//引用汉字串显示
//	ShowText(0,0x80,"深圳绘晶科技");//引用汉字串显示
//	ShowText(1,0x95,"您好! 欢迎光临");//引用汉字串显示	
//			
//						
//   	for(z=0;z<10;z++)
//	{		
//		for(z1=0;z1<10;z1++)
//		{		
//			for(d=0;d<6;d++)
//			{	
//				
//				for(d1=0;d1<10;d1++)
//				{	
//					time_ram_24x48(0x86,0,z,z1,d,d1);
////				time_ram_24x48(0x84,0,RUN_TIME/60/10,RUN_TIME/60%10,10,RUN_TIME%60/10,RUN_TIME%60%10);
//					for(s=0;s<6;s++)
//					{								
//						for(s1=0;s1<10;s1++)
//						{		
//							time_ram_16x32(0x80,0,d,d1,s,s1);	
//							time_nms(10);////延时 x ms	
//						LED1=!LED1;
//						LED2=!LED2;	
//							for(s10=0;s10<10;s10++)
//							{			
//								time_nms(10);////延时 x ms
//								
//								for(s100=0;s100<10;s100++)
//								{																
//								time_nms(1);////延时 x ms	
//								time_ram_8x16(0x80,16,z,z1,d,d1,s,s1,s10,s100);	
////								time_rom_8x16(0x80,z,z1,d,d1,s,s1,s10,s100);									
//								}
//							}
//						}
//					}
//				}
//			}	
//		}	
//	}						
	

}


Uchar code  num_8x16[11][16]={
/*--  文字:  0  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00,0x00,
/*--  文字:  1  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x10,0x70,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x7C,0x00,0x00,
/*--  文字:  2  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x04,0x04,0x08,0x10,0x20,0x42,0x7E,0x00,0x00,
/*--  文字:  3  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x04,0x18,0x04,0x02,0x02,0x42,0x44,0x38,0x00,0x00,
/*--  文字:  4  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x04,0x0C,0x14,0x24,0x24,0x44,0x44,0x7E,0x04,0x04,0x1E,0x00,0x00,
/*--  文字:  5  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7E,0x40,0x40,0x40,0x58,0x64,0x02,0x02,0x42,0x44,0x38,0x00,0x00,
/*--  文字:  6  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x1C,0x24,0x40,0x40,0x58,0x64,0x42,0x42,0x42,0x24,0x18,0x00,0x00,
/*--  文字:  7  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x7E,0x44,0x44,0x08,0x08,0x10,0x10,0x10,0x10,0x10,0x10,0x00,0x00,
/*--  文字:  8  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x24,0x18,0x24,0x42,0x42,0x42,0x3C,0x00,0x00,
/*--  文字:  9  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x18,0x24,0x42,0x42,0x42,0x26,0x1A,0x02,0x02,0x24,0x38,0x00,0x00,
/*--  文字:  :  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=8x16   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,
};

Uchar code  num_16x32[11][62]={
/*--  文字:  0  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0x1F,0xFF,
0x1F,0xFF,0x1E,0x0F,0x1E,0x0F,0x3E,0x0F,0x3E,0x0F,0x3E,0x0F,0x1C,0x07,0x00,0x00,
0x18,0x02,0x3C,0x0F,0x3C,0x0F,0x3C,0x0F,0x3C,0x1F,0x3C,0x1F,0x7C,0x0F,0x7F,0xFF,
0x7F,0xFF,0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  1  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=9x31   --*/
/*--  宽度不是8的倍数，现调整为：宽度x高度=16x31  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x07,0x00,
0x0F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x0E,0x00,0x00,0x00,
0x04,0x00,0x1E,0x00,0x1E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x1E,0x00,
0x0E,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  2  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0x07,0xFF,
0x03,0xFF,0x00,0x07,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x07,0xFF,0x0F,0xFE,
0x1F,0xFC,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x7F,0xF8,
0x7F,0xFC,0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  3  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0x0F,0xFF,
0x07,0xFF,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x1F,0x00,0x0F,0x0F,0xFF,0x1F,0xFC,
0x0F,0xFE,0x00,0x0F,0x00,0x1F,0x00,0x1F,0x00,0x1E,0x00,0x1E,0x00,0x1E,0x0F,0xFE,
0x1F,0xFE,0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  4  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x03,0x38,0x07,
0x38,0x0F,0x3C,0x1F,0x3C,0x1F,0x7C,0x1F,0x7C,0x1F,0x7C,0x1F,0x3F,0xFE,0x1F,0xFC,
0x0F,0xFC,0x00,0x1E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x1E,
0x00,0x0E,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  5  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0x3F,0xFE,
0x3F,0xFC,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3C,0x00,0x3F,0xF8,0x1F,0xFC,
0x0F,0xFE,0x00,0x0F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1E,0x00,0x1E,0x0F,0xFE,
0x1F,0xFE,0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  6  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0x1F,0xFE,
0x1F,0xFC,0x1E,0x00,0x1E,0x00,0x3E,0x00,0x3E,0x00,0x3E,0x00,0x1F,0xFC,0x0F,0xFE,
0x1F,0xFE,0x3C,0x07,0x3C,0x0F,0x3C,0x0F,0x3C,0x0F,0x3C,0x0F,0x7C,0x0F,0x7F,0xFF,
0x7F,0xFF,0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  7  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0x07,0xFF,
0x03,0xFF,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x07,0x00,0x00,
0x00,0x06,0x00,0x0F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x1F,0x00,0x0F,
0x00,0x0E,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  8  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0x1F,0xFF,
0x1F,0xFF,0x1E,0x0F,0x1E,0x0F,0x3E,0x0F,0x3E,0x0F,0x3E,0x0F,0x1F,0xFF,0x0F,0xFE,
0x1F,0xFE,0x3C,0x0F,0x3C,0x0F,0x3C,0x0F,0x3C,0x1F,0x3C,0x1F,0x7C,0x0F,0x7F,0xFF,
0x7F,0xFF,0x3F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  9  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=16x31   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0x3F,0xFF,
0x3F,0xFF,0x3C,0x0F,0x3C,0x0F,0x3C,0x0F,0x3C,0x1F,0x3C,0x0F,0x3F,0xFF,0x1F,0xFC,
0x0F,0xFE,0x00,0x0F,0x00,0x1F,0x00,0x1F,0x00,0x1E,0x00,0x1E,0x00,0x1E,0x0F,0xFE,
0x1F,0xFE,0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  -  --*/
/*--  DS-Digital24;  此字体下对应的点阵为：宽x高=15x31   --*/
/*--  宽度不是8的倍数，现调整为：宽度x高度=16x31  --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFC,0x7F,0xFE,
0x3F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


Uchar code  num_24x48[11][144]={
/*--  文字:  0  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x01,0xE7,0x80,0x03,0xC3,
0xC0,0x07,0x81,0xE0,0x0F,0x80,0xF0,0x0F,0x00,0xF0,0x1F,0x00,0xF8,0x1E,0x00,0xF8,
0x3E,0x00,0x78,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,
0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,
0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x78,
0x1E,0x00,0xF8,0x1F,0x00,0xF8,0x0F,0x00,0xF0,0x0F,0x81,0xF0,0x07,0x81,0xE0,0x03,
0xC3,0xC0,0x01,0xE7,0x80,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  1  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x1C,0x00,0x00,0x7C,
0x00,0x07,0xFC,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,
0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,
0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,
0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,
0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,0x3C,0x00,0x00,
0x3E,0x00,0x00,0x7F,0x00,0x07,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  2  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x03,0xC7,0xC0,0x07,0x01,
0xE0,0x0E,0x00,0xF0,0x1E,0x00,0xF8,0x1E,0x00,0xF8,0x3E,0x00,0x78,0x3E,0x00,0x78,
0x3F,0x00,0x78,0x3F,0x00,0x78,0x1F,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF0,0x00,
0x01,0xF0,0x00,0x03,0xE0,0x00,0x03,0xC0,0x00,0x07,0x80,0x00,0x0F,0x00,0x00,0x1E,
0x00,0x00,0x3C,0x00,0x00,0x78,0x00,0x00,0xF0,0x00,0x01,0xE0,0x00,0x03,0xC0,0x00,
0x07,0x80,0x1C,0x07,0x00,0x1C,0x0E,0x00,0x38,0x1C,0x00,0x38,0x3C,0x00,0x78,0x3F,
0xFF,0xF8,0x3F,0xFF,0xF8,0x3F,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  3  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x00,0x07,0x87,0x80,0x0F,0x03,
0xC0,0x1E,0x01,0xE0,0x1E,0x01,0xF0,0x1E,0x01,0xF0,0x1F,0x00,0xF0,0x1F,0x00,0xF0,
0x1E,0x00,0xF0,0x00,0x00,0xF0,0x00,0x01,0xF0,0x00,0x01,0xF0,0x00,0x03,0xE0,0x00,
0x03,0xC0,0x00,0x0F,0x00,0x00,0xFE,0x00,0x00,0x07,0x80,0x00,0x01,0xE0,0x00,0x00,
0xF0,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0x78,0x00,0x00,0x7C,0x1E,0x00,0x7C,
0x3F,0x00,0x7C,0x3F,0x00,0x7C,0x3F,0x00,0x78,0x3E,0x00,0xF8,0x1E,0x00,0xF0,0x0F,
0x01,0xE0,0x07,0x87,0xC0,0x01,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  4  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xC0,0x00,0x03,0xC0,0x00,0x07,
0xC0,0x00,0x0F,0xC0,0x00,0x0F,0xC0,0x00,0x1F,0xC0,0x00,0x3F,0xC0,0x00,0x3F,0xC0,
0x00,0x77,0xC0,0x00,0x77,0xC0,0x00,0xE7,0xC0,0x01,0xC7,0xC0,0x01,0xC7,0xC0,0x03,
0x87,0xC0,0x07,0x07,0xC0,0x07,0x07,0xC0,0x0E,0x07,0xC0,0x1E,0x07,0xC0,0x1C,0x07,
0xC0,0x38,0x07,0xC0,0x38,0x07,0xC0,0x7F,0xFF,0xFE,0x7F,0xFF,0xFE,0x00,0x07,0xC0,
0x00,0x07,0xC0,0x00,0x07,0xC0,0x00,0x07,0xC0,0x00,0x07,0xC0,0x00,0x07,0xC0,0x00,
0x07,0xC0,0x00,0x07,0xE0,0x00,0x7F,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  5  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xF8,0x0F,0xFF,0xF8,0x0F,0xFF,
0xF8,0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x00,0x00,
0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x00,0x00,0x0E,0x7F,0x00,0x0D,0xFF,0xC0,0x0F,
0xC3,0xE0,0x1F,0x01,0xF0,0x1E,0x00,0xF8,0x1E,0x00,0xF8,0x00,0x00,0x78,0x00,0x00,
0x7C,0x00,0x00,0x7C,0x00,0x00,0x7C,0x00,0x00,0x7C,0x1E,0x00,0x7C,0x3F,0x00,0x7C,
0x3F,0x00,0x78,0x3F,0x00,0x78,0x3E,0x00,0xF8,0x1E,0x00,0xF0,0x1E,0x01,0xF0,0x0E,
0x01,0xE0,0x07,0x87,0xC0,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  6  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xC0,0x00,0xF1,0xE0,0x03,0xC1,
0xF0,0x07,0x81,0xF8,0x07,0x01,0xF8,0x0F,0x00,0xF0,0x1F,0x00,0x00,0x1E,0x00,0x00,
0x1E,0x00,0x00,0x3E,0x00,0x00,0x3E,0x00,0x00,0x3E,0x00,0x00,0x3E,0x3F,0x80,0x3E,
0xFF,0xE0,0x3F,0xE3,0xF0,0x3F,0x80,0xF8,0x3F,0x00,0xF8,0x3F,0x00,0x7C,0x3E,0x00,
0x7C,0x3E,0x00,0x7C,0x3E,0x00,0x3C,0x3E,0x00,0x3C,0x3E,0x00,0x3C,0x3E,0x00,0x3C,
0x3E,0x00,0x7C,0x1E,0x00,0x7C,0x1F,0x00,0x78,0x0F,0x00,0x78,0x0F,0x80,0xF0,0x07,
0xC0,0xE0,0x03,0xE3,0xC0,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  7  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFC,0x1F,0xFF,0xFC,0x1F,0xFF,
0xF8,0x1F,0x00,0x38,0x1C,0x00,0x70,0x1C,0x00,0xE0,0x38,0x00,0xE0,0x38,0x01,0xC0,
0x00,0x01,0xC0,0x00,0x03,0x80,0x00,0x03,0x80,0x00,0x07,0x80,0x00,0x07,0x00,0x00,
0x0F,0x00,0x00,0x0E,0x00,0x00,0x1E,0x00,0x00,0x1E,0x00,0x00,0x3C,0x00,0x00,0x3C,
0x00,0x00,0x3C,0x00,0x00,0x7C,0x00,0x00,0x78,0x00,0x00,0x78,0x00,0x00,0xF8,0x00,
0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,
0xF8,0x00,0x00,0xF8,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  8  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x07,0xC3,0xC0,0x0F,0x00,
0xE0,0x1E,0x00,0xF0,0x1E,0x00,0x78,0x3C,0x00,0x78,0x3C,0x00,0x78,0x3C,0x00,0x7C,
0x3E,0x00,0x78,0x3E,0x00,0x78,0x1F,0x00,0x78,0x1F,0x80,0xF0,0x0F,0xE1,0xE0,0x07,
0xFB,0xC0,0x01,0xFF,0x80,0x01,0xFF,0x80,0x07,0xBF,0xC0,0x0F,0x0F,0xE0,0x1E,0x03,
0xF0,0x3E,0x01,0xF8,0x3C,0x00,0xF8,0x3C,0x00,0x7C,0x7C,0x00,0x7C,0x78,0x00,0x3C,
0x78,0x00,0x3C,0x7C,0x00,0x3C,0x3C,0x00,0x78,0x3C,0x00,0x78,0x1E,0x00,0x70,0x0F,
0x00,0xE0,0x07,0xC3,0xC0,0x01,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  9  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFE,0x00,0x07,0xC7,0x80,0x0F,0x01,
0xE0,0x1E,0x01,0xE0,0x1E,0x00,0xF0,0x3E,0x00,0xF8,0x3C,0x00,0x78,0x3C,0x00,0x78,
0x7C,0x00,0x7C,0x7C,0x00,0x7C,0x7C,0x00,0x7C,0x7C,0x00,0x7C,0x7C,0x00,0x7C,0x3C,
0x00,0xFC,0x3E,0x00,0xFC,0x3E,0x01,0xFC,0x1F,0x03,0xFC,0x1F,0x8F,0xFC,0x0F,0xFF,
0x7C,0x03,0xFC,0x7C,0x00,0x00,0x7C,0x00,0x00,0xF8,0x00,0x00,0xF8,0x00,0x00,0xF8,
0x00,0x00,0xF0,0x00,0x01,0xF0,0x0F,0x01,0xE0,0x1F,0x01,0xE0,0x1F,0x03,0xC0,0x1F,
0x07,0x80,0x0F,0x9F,0x00,0x03,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/*--  文字:  :  --*/
/*--  宋体36;  此字体下对应的点阵为：宽x高=24x48   --*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x7E,0x00,0x00,0xFE,0x00,0x00,
0xFE,0x00,0x00,0x7E,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x7E,0x00,0x00,0xFE,0x00,0x00,
0xFE,0x00,0x00,0x7E,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
