#include "oled.h"
#include "ASCII.h"

uint8_t oled_data[4][128]={0};

extern void DelayUS(__IO uint32_t nTime);

#define I2C_SDA_LOW()   GPIOE->BSRRH = GPIO_Pin_2
#define I2C_SDA_HIGH()  GPIOE->BSRRL = GPIO_Pin_2

#define I2C_SCL_LOW()   GPIOE->BSRRH = GPIO_Pin_3
#define I2C_SCL_HIGH()  GPIOE->BSRRL = GPIO_Pin_3

#define I2C_SDA_Read()  (GPIOE->IDR & GPIO_Pin_2)

//#define I2C_Delay() DelayUS(1);//实现延时4us

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
    //I2C_SCL_LOW();
    I2C_SDA_LOW();
    //I2C_Delay();
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
        //I2C_Delay();
        //if(I2C_SDA_Read())
        return 1;
    }
    I2C_SCL_LOW();
    I2C_Delay();
    return 0;
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

static void OLED_WriteCmd(uint8_t Cmd)
{
    I2C_Start();
    I2C_SendByte(0x78);
    I2C_WaitAck();
    I2C_SendByte(0x00);
    I2C_WaitAck();
    I2C_SendByte(Cmd);
    I2C_WaitAck();
    I2C_Stop();
}

static void OLED_WriteData(uint8_t Data)
{
    I2C_Start();
    I2C_SendByte(0x78);
    I2C_WaitAck();
    I2C_SendByte(0x40);
    I2C_WaitAck();
    I2C_SendByte(Data);
    I2C_WaitAck();
    I2C_Stop();
}

static void OLED_SetAddr(uint8_t page,uint8_t col)
{
    OLED_WriteCmd(0xb0|page);//设置页
    OLED_WriteCmd(((col>>4)&0x0f)|0x10);//设置列高4位
    OLED_WriteCmd(0x0f&col);//设置列低4位
}

void OLED_Clear(void)
{
    uint8_t x,y;
    for(y=0;y<4;y++)
        for(x=0;x<128;x++)
            oled_data[y][x] = 0;
  
    for(y=4;y<8;y++)
    {
        OLED_SetAddr(y,0);
        for(x=0;x<128;x++)
        {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_Update(uint8_t spage, uint8_t epage, uint8_t scol, uint8_t ecol)
{
    uint8_t x,y;
  
    for(y=spage;y<=epage;y++)
    {
        OLED_SetAddr(y+4,scol);
        for(x=scol;x<=ecol;x++)
        {
            OLED_WriteData(oled_data[y][x]);
        }
    }
}

void OLED_String(uint8_t page, uint8_t col, uint8_t *str)
{
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t c = *str;
  while(c && (c < 0x80))
  {
    if(c<0x20)
      c = 0x20;
    else
      c -= 0x20;
    if(page<4)
    {
      if((col+i+5)<128)
      {
        for(j=0; j<5; j++)
          oled_data[page][col+i++] = ASCII0507[5 * c + j];
      }
    }
    c = *++str;
  }
  OLED_Update(page, page, col, col + i);
}

void OLED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  GPIO_SetBits(GPIOE, GPIO_Pin_2 | GPIO_Pin_3);
  
  DelayUS(10000);

  OLED_WriteCmd(0xae); //关显示
  OLED_WriteCmd(0xd5); //晶振频率
  OLED_WriteCmd(0x80);
  OLED_WriteCmd(0xa8); //duty 设置
  OLED_WriteCmd(0x3f); //duty=1/64
  OLED_WriteCmd(0xd3); //显示偏移
  OLED_WriteCmd(0x00);
  OLED_WriteCmd(0xa1); //段重定向设置
  OLED_WriteCmd(0xa6);
  OLED_WriteCmd(0x40); //起始行
  OLED_WriteCmd(0xb0);
  OLED_WriteCmd(0xd5);
  OLED_WriteCmd(0xf0);
  OLED_WriteCmd(0x8d); //升压允许
  OLED_WriteCmd(0x14);
  OLED_WriteCmd(0x20); //page address mode
  OLED_WriteCmd(0x02);
  OLED_WriteCmd(0xc8); //行扫描顺序：从上到下
  OLED_WriteCmd(0xa1); //列扫描顺序：从左到右
  OLED_WriteCmd(0xda); //sequential configuration
  OLED_WriteCmd(0x02); // ****************
  OLED_WriteCmd(0x81); //微调对比度,本指令的 0x81 不要改动，改下面的值
  OLED_WriteCmd(0xcf); //微调对比度的值，可设置范围 0x00～0xff
  OLED_WriteCmd(0xd9); //Set Pre-Charge Period
  OLED_WriteCmd(0xf1);
  OLED_WriteCmd(0xdb); //Set VCOMH Deselect Level
  OLED_WriteCmd(0x49);
//  OLED_WriteCmd(0x40);
  OLED_WriteCmd(0xaf); //开显示

  OLED_Clear(); //初始清屏
  OLED_Update(1,2,20,100);
}
