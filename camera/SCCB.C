#include "sccb.h"
#include "stm32f4xx.h"                  // Device header
#include "sysdelay.h"

void SCCB_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
   /* Enable GPIOC  clock */
  RCC_APB1PeriphClockCmd(SCCB_SCL6_GPIO_CLK | SCCB_SDA_GPIO_CLK, ENABLE);
	/* SCL6 */
	GPIO_InitStructure.GPIO_Pin =  SCCB_SIO_C;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(SCCB_SCL6_GPIO_PORT, &GPIO_InitStructure);
	/* SDA */
	GPIO_InitStructure.GPIO_Pin =  SCCB_SIO_D;
  GPIO_Init(SCCB_SDA_GPIO_PORT, &GPIO_InitStructure);
}


void SCCB_SID_DATA_OUT(void)//设置SCCB_SID为输出
{
  	GPIO_InitTypeDef  GPIO_InitStructure;

  	GPIO_InitStructure.GPIO_Pin = SCCB_SIO_D;               
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(SCCB_SDA_GPIO_PORT, &GPIO_InitStructure);
}
void SCCB_SID_DATA_IN(void)//设置SCCB_SID为输入
{
  	GPIO_InitTypeDef  GPIO_InitStructure;

  	GPIO_InitStructure.GPIO_Pin = SCCB_SIO_D;               
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;			
  	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(SCCB_SDA_GPIO_PORT, &GPIO_InitStructure);
}
void SCCB_Start(void)
{
    SCCB_SID_H();     //数据线高电平
    Delay_Nus(50);
    SCCB_SIC_H();	   //在时钟线高的时候数据线由高至低
    Delay_Nus(50);
    SCCB_SID_L();
    Delay_Nus(50);
    SCCB_SIC_L();	 //数据线恢复低电平，单操作函数必要
    Delay_Nus(50);
}
void SCCB_Stop(void)
{
    SCCB_SID_L();
    Delay_Nus(50);
    SCCB_SIC_H();	
    Delay_Nus(50);  
    SCCB_SID_H();	
    Delay_Nus(50);  
}
uint8_t SCCB_Write(uint8_t m_data)
{//返回值为1 发送正常
	uint8_t j,tem;

	for(j=0;j<8;j++) //循环8次发送数据
	{
		if((m_data<<j)&0x80)
			SCCB_SID_H();
		else 
			SCCB_SID_L();
		Delay_Nus(50);
		SCCB_SIC_H();	
		Delay_Nus(50);
		SCCB_SIC_L();	
		Delay_Nus(50);
	}
	Delay_Nus(10);
	SCCB_SID_DATA_IN();
	Delay_Nus(50);
	SCCB_SIC_H();	
	Delay_Nus(10);
	if(SCCB_SID_STATE)
	tem=0;//SDA=1发送失败
	else 
	tem=1;//SDA=0发送成功，返回1
	SCCB_SIC_L();	
	Delay_Nus(50);	
  SCCB_SID_DATA_OUT();
	return tem;  
}
uint8_t SCCB_Read(void)
{
	uint8_t read,j;
	read=0x00;
	
	SCCB_SID_DATA_IN();
	Delay_Nus(50);
	for(j=8;j>0;j--) //循环8次接收数据
	{		     
		Delay_Nus(50);
		SCCB_SIC_H();
		Delay_Nus(50);
		read=read<<1;
		if(SCCB_SID_STATE)
		read=read+1; 
		SCCB_SIC_L();
		Delay_Nus(50);
	}	
    SCCB_SID_DATA_OUT();
	return read;
}
void noAck(void)	 
{	
	SCCB_SID_H();	
	Delay_Nus(50);	
	SCCB_SIC_H();	
	Delay_Nus(50);	
	SCCB_SIC_L();	
	Delay_Nus(50);	
	SCCB_SID_L();	
	Delay_Nus(50);
}
