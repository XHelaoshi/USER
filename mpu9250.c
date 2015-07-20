#include "mpu9250.h"
#include  <math.h>


extern void DelayUS(__IO uint32_t nTime);

#define I2C_SDA_LOW()   GPIOB->BSRRH = GPIO_Pin_7
#define I2C_SDA_HIGH()  GPIOB->BSRRL = GPIO_Pin_7

#define I2C_SCL_LOW()   GPIOB->BSRRH = GPIO_Pin_6
#define I2C_SCL_HIGH()  GPIOB->BSRRL = GPIO_Pin_6

#define I2C_SDA_Read()  (GPIOB->IDR & GPIO_Pin_7)

//#define I2C_Delay() DelayUS(1);//实现延时4us

#define bool uint8_t
#define FALSE 0
#define TRUE  1

// 定义MPU9250内部地址
//****************************************
#define	SMPLRT_DIV		0x19	//陀螺仪采样率，典型值：0x07(125Hz)
#define	CONFIG			0x1A	//低通滤波频率，典型值：0x06(5Hz)
#define	GYRO_CONFIG		0x1B	//陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	ACCEL_CONFIG	0x1C	//加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)

#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40

#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42

#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48

		
#define MAG_XOUT_L		0x03
#define MAG_XOUT_H		0x04
#define MAG_YOUT_L		0x05
#define MAG_YOUT_H		0x06
#define MAG_ZOUT_L		0x07
#define MAG_ZOUT_H		0x08

#define	PWR_MGMT_1		0x6B	//电源管理，典型值：0x00(正常启用)
#define	WHO_AM_I		  0x75	//IIC地址寄存器(默认数值0x68，只读)

#define	GYRO_ADDRESS   0xD0	  //陀螺地址
#define MAG_ADDRESS    0x18   //磁场地址
#define ACCEL_ADDRESS  0xD0 

#define Gyro_Gr	(float)((2 * 2000.0f) / 65536.0f / 180.0f * 3.141593f)
float Gyro_Ex = 0.118f;
float Gyro_Ey = 0.023f;
float Gyro_Ez = 0.072f;

#define Mag_Gr 0.15f
float Mag_Ex = 132.0f;
float Mag_Ey = -29.0f;
float Mag_Ez = -74.0f;

uint8_t BUF[10];       //接收数据缓存区
float ACCEL_DATA[3];
float GYRO_DATA[3];
float MAG_DATA[3];
float TEMP_DATA[1];

static void I2C_Delay()
{
  uint8_t i=24;
  while(i--);
}

static void I2C_Start(void)
{
    I2C_SCL_HIGH();
    I2C_SDA_HIGH();
    I2C_Delay();
    I2C_SDA_LOW();
    I2C_Delay();
    I2C_SCL_LOW();
    I2C_Delay();
}

static void I2C_Stop(void)
{
    I2C_SDA_LOW();
    I2C_SCL_HIGH();
    I2C_Delay();
    I2C_SDA_HIGH();
    I2C_Delay();
}

static uint8_t I2C_WaitAck(void)  //0:Ack     1:NoAck
{
    I2C_SDA_HIGH();
    I2C_SCL_HIGH();
    I2C_Delay();
    if(I2C_SDA_Read())
    {
        return 1;
    }
    I2C_SCL_LOW();
    I2C_Delay();
    return 0;
}

static void I2C_Ack(void)
{	
	I2C_SCL_LOW();
	I2C_Delay();
	I2C_SDA_LOW();
	I2C_Delay();
	I2C_SCL_HIGH();
	I2C_Delay();
	I2C_SCL_LOW();
	I2C_Delay();
}

static void I2C_NoAck(void)
{	
	I2C_SCL_LOW();
	I2C_Delay();
	I2C_SDA_HIGH();
	I2C_Delay();
	I2C_SCL_HIGH();
	I2C_Delay();
	I2C_SCL_LOW();
	I2C_Delay();
} 

static void I2C_SendByte(uint8_t Byte)
{
    uint8_t Cnt;
    for(Cnt=0;Cnt<8;Cnt++)
    {
        if(Byte&0x80)
            I2C_SDA_HIGH();
        else
            I2C_SDA_LOW();
        Byte <<= 1;
        I2C_Delay();
        I2C_SCL_HIGH();
        I2C_Delay();
        I2C_SCL_LOW();
        I2C_Delay();
    }
}

static uint8_t I2C_ReadByte(void)  //数据从高位到低位//
{ 
    u8 i=8;
    u8 ReceiveByte=0;
    I2C_SDA_HIGH();
    while(i--)
    {
      ReceiveByte<<=1;      
      I2C_SCL_LOW();
      I2C_Delay();
      I2C_SCL_HIGH();
      I2C_Delay();	
      if(I2C_SDA_Read())
      {
        ReceiveByte|=0x01;
      }
    }
    I2C_SCL_LOW();
    return ReceiveByte;
}

bool Single_Write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_data)
{
  	I2C_Start();
    I2C_SendByte(SlaveAddress);   //发送设备地址+写信号//I2C_SendByte(((REG_Address & 0x0700) >>7) | SlaveAddress & 0xFFFE);//设置高起始地址+器件地址 
    if(I2C_WaitAck()){I2C_Stop(); return 0;}
    I2C_SendByte(REG_Address);   //设置低起始地址      
    I2C_WaitAck();	
    I2C_SendByte(REG_data);
    I2C_WaitAck();   
    I2C_Stop();
    return 1;
}

unsigned char Single_Read(unsigned char SlaveAddress,unsigned char REG_Address)
{
    unsigned char REG_data;     	
    I2C_Start();
    I2C_SendByte(SlaveAddress); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(I2C_WaitAck()){I2C_Stop(); return 0;}
    I2C_SendByte((u8) REG_Address);   //设置低起始地址      
    I2C_WaitAck();
    I2C_Stop();
    I2C_Start();
    I2C_SendByte(SlaveAddress+1);
    I2C_WaitAck();
    REG_data= I2C_ReadByte();
    I2C_NoAck();
    I2C_Stop();
    return REG_data;
}						      

unsigned char *Multi_Read(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char *data,unsigned char size)
{
    unsigned char i;     	
    I2C_Start();
    I2C_SendByte(SlaveAddress); //I2C_SendByte(((REG_Address & 0x0700) >>7) | REG_Address & 0xFFFE);//设置高起始地址+器件地址 
    if(I2C_WaitAck()){I2C_Stop(); return 0;}
    I2C_SendByte((u8) REG_Address);   //设置低起始地址      
    I2C_WaitAck();
    I2C_Stop();
    I2C_Start();
    I2C_SendByte(SlaveAddress+1);
    I2C_WaitAck();
    for(i=0;i<size-1;i++)
    {
      data[i]= I2C_ReadByte();
      I2C_Ack();
    }
    data[i]= I2C_ReadByte();
    I2C_NoAck();
    I2C_Stop();
    return data;
}

//初始化MPU9250，根据需要请参考pdf进行修改************************
void Init_MPU9250(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
  
  DelayUS(10000);
  
  
/*
   Single_Write(GYRO_ADDRESS,PWR_M, 0x80);   //
   Single_Write(GYRO_ADDRESS,SMPL, 0x07);    //
   Single_Write(GYRO_ADDRESS,DLPF, 0x1E);    //±2000°
   Single_Write(GYRO_ADDRESS,INT_C, 0x00 );  //
   Single_Write(GYRO_ADDRESS,PWR_M, 0x00);   //
*/
  Single_Write(GYRO_ADDRESS,PWR_MGMT_1, 0x00);	// 解除休眠状态
	Single_Write(GYRO_ADDRESS,SMPLRT_DIV, 0x07); // 1/8 smaple rate
	Single_Write(GYRO_ADDRESS,CONFIG, 0x04); // 20Hz
	Single_Write(GYRO_ADDRESS,GYRO_CONFIG, 0x18); // 2000 degree/s
	Single_Write(GYRO_ADDRESS,ACCEL_CONFIG, 0x01); // 2 g
  Single_Write(GYRO_ADDRESS,0x37,0x02);//turn on Bypass Mode 
  DelayUS(10000);	
  Single_Write(MAG_ADDRESS,0x0A,0x16);
  DelayUS(10000);	
  //----------------
//	Single_Write(GYRO_ADDRESS,0x6A,0x00);//close Master Mode	

}
	
//******读取MPU9250数据****************************************
void READ_MPU9250_ACCEL(void)
{ 
//   BUF[0]=Single_Read(ACCEL_ADDRESS,ACCEL_XOUT_L); 
//   BUF[1]=Single_Read(ACCEL_ADDRESS,ACCEL_XOUT_H);
//   BUF[2]=Single_Read(ACCEL_ADDRESS,ACCEL_YOUT_L);
//   BUF[3]=Single_Read(ACCEL_ADDRESS,ACCEL_YOUT_H);
//   BUF[4]=Single_Read(ACCEL_ADDRESS,ACCEL_ZOUT_L);
//   BUF[5]=Single_Read(ACCEL_ADDRESS,ACCEL_ZOUT_H);
  
   Multi_Read(ACCEL_ADDRESS, ACCEL_XOUT_H, BUF, 6);
  
   ACCEL_DATA[0]=	(int16_t)((BUF[0]<<8)+BUF[1]);
   ACCEL_DATA[1]=	(int16_t)((BUF[2]<<8)+BUF[3]);
   ACCEL_DATA[2]=	(int16_t)((BUF[4]<<8)+BUF[5]);
 
}

	
void READ_MPU9250_GYRO(void)
{ 
//   BUF[0]=Single_Read(GYRO_ADDRESS,GYRO_XOUT_L); 
//   BUF[1]=Single_Read(GYRO_ADDRESS,GYRO_XOUT_H);
//   BUF[2]=Single_Read(GYRO_ADDRESS,GYRO_YOUT_L);
//   BUF[3]=Single_Read(GYRO_ADDRESS,GYRO_YOUT_H);
//   BUF[4]=Single_Read(GYRO_ADDRESS,GYRO_ZOUT_L);
//   BUF[5]=Single_Read(GYRO_ADDRESS,GYRO_ZOUT_H);
  
   Multi_Read(ACCEL_ADDRESS, GYRO_XOUT_H, BUF, 6);
  
   GYRO_DATA[0]=(int16_t)((BUF[0]<<8)|BUF[1]);
   GYRO_DATA[0] = GYRO_DATA[0] * Gyro_Gr - Gyro_Ex;
  
   GYRO_DATA[1]=(int16_t)((BUF[2]<<8)|BUF[3]);
   GYRO_DATA[1] = GYRO_DATA[1] * Gyro_Gr - Gyro_Ey; 
  
   GYRO_DATA[2]=(int16_t)((BUF[4]<<8)|BUF[5]);
   GYRO_DATA[2] = GYRO_DATA[2] * Gyro_Gr - Gyro_Ez;  
}

void READ_MPU9250_TEMP(void)
{ 
//   BUF[0]=Single_Read(GYRO_ADDRESS,TEMP_OUT_L); 
//   BUF[1]=Single_Read(GYRO_ADDRESS,TEMP_OUT_H); 
  
   Multi_Read(ACCEL_ADDRESS, TEMP_OUT_H, BUF, 2);
  
   TEMP_DATA[0] = (BUF[0]<<8)|BUF[1];
   TEMP_DATA[0] = 35+ ((double) (TEMP_DATA[0] + 13200)) / 280;// 读取计算出温度
}

void READ_MPU9250_MAG(void)
{ 
   BUF[0]=Single_Read(MAG_ADDRESS,MAG_XOUT_L);
   BUF[1]=Single_Read(MAG_ADDRESS,MAG_XOUT_H);
   BUF[2]=Single_Read(MAG_ADDRESS,MAG_YOUT_L);
   BUF[3]=Single_Read(MAG_ADDRESS,MAG_YOUT_H);
   BUF[4]=Single_Read(MAG_ADDRESS,MAG_ZOUT_L);
   BUF[5]=Single_Read(MAG_ADDRESS,MAG_ZOUT_H);
  
   //Multi_Read(ACCEL_ADDRESS, MAG_XOUT_L, BUF, 6);
  
   Single_Read(MAG_ADDRESS,0x09); // read ST2 at the end in continuous measurement mode
  
   MAG_DATA[0] = (int16_t)((BUF[3]<<8)|BUF[2]);
   MAG_DATA[0] = Mag_Gr * MAG_DATA[0] - Mag_Ex;
   MAG_DATA[1] = (int16_t)((BUF[1]<<8)|BUF[0]);
   MAG_DATA[1] = Mag_Gr * MAG_DATA[1] - Mag_Ey;
   MAG_DATA[2] = -(int16_t)((BUF[5]<<8)|BUF[4]);
   MAG_DATA[2] = Mag_Gr * MAG_DATA[2] - Mag_Ez;
  
}

/*************结束***************/
