#include <rtthread.h>
#include "rda5875.h"
#include "stm32f4xx_gpio.h"

extern const u8 rda5875_init_cmd[][14];
extern const uint16_t RDA5875_RF_INIT_TAB[][2];
extern const u32 RDA5875_PSKEY_RF_INIT_TAB[][2];
extern const uint16_t RDA5875_DC_CAL_INIT_TAB[][2];
extern const u32 RDA5875_TRAP_TAB[56 + 12][2];
extern const u8 RDA5875_ADDR[10];



#define RDA5875_LDO_CMD(x)  ((x)?GPIO_SetBits(GPIOB,GPIO_Pin_14):GPIO_ResetBits(GPIOB,GPIO_Pin_14))
	//ldo
void ldo_init(){
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	 /* GPIOD Periph clock enable */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

//	/* Configure PB1 in output pushpull mode */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitTypeDef  GPIO_InitStructure;
	  /* GPIOG Peripheral clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  /* Configure PG6 and PG8 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

int receive_flag = 0;
void RDA5875_DelayMs(u32 ms)
{
  volatile u32 i;
  while(ms--)
  {
    for(i = 0; i < 168000/10; i++);
  }
}


int RDA5875_WriteReg(int type, u32 addr, u32 data)
{
  u8 hci_cmd_str[] = 
  {
    0x01,                 //0x0 -- 0x1 HCI Command Head
    0x02, 0xfd, 0x0a,     //0x1 -- 0x3 OPCODE; length of command parameters
    0x00, 0x01,           //0x4 -- 0x2 memory type; length of data
    0x00,0x00,0x24,0x40,  //0x6 -- 0x4 addr
    0x9c,0xf3,0x04,0x00   //0xA -- 0x4 data
                          //0xE
  };
  if(type)
  {
    if(addr >= 0x80)
      return -1;
    hci_cmd_str[ 4] = 0x01;
    addr = 0x200 + addr * 4;
  }
  //address field
  hci_cmd_str[ 6] = (addr >>  0) & 0xFF;
  hci_cmd_str[ 7] = (addr >>  8) & 0xFF;
  hci_cmd_str[ 8] = (addr >> 16) & 0xFF;
  hci_cmd_str[ 9] = (addr >> 24) & 0xFF;
  //data field
  hci_cmd_str[10] = (data >>  0) & 0xFF;
  hci_cmd_str[11] = (data >>  8) & 0xFF;
  hci_cmd_str[12] = (data >> 16) & 0xFF;
  hci_cmd_str[13] = (data >> 24) & 0xFF;
  //
  return RDA5875_Write(hci_cmd_str, sizeof(hci_cmd_str));
}



int RDA5875_Init(void)
{
  int i,k;
  u8 evt[16];
  RDA5875_IOConfig(115200);
	ldo_init();
  RDA5875_LDO_CMD(1);
  RDA5875_DelayMs(20);
  /*
    {0x40240000,0x2004f39c},
    C1:C47
    UartWaitMs(50);
    D1:D36
    BtLDOEn(0);
    UartWaitMs(30);
    BtLDOEn(1);
    UartWaitMs(30);
  */
  RDA5875_WriteReg(0, 0x40240000, 0x2004F39C);
  for(i = 0; i < 47; i++)
  {
    RDA5875_WriteReg(1, RDA5875_RF_INIT_TAB[i][0], RDA5875_RF_INIT_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  RDA5875_DelayMs(50);
  for(i = 0; i < 36; i++)
  {
    RDA5875_WriteReg(0, RDA5875_PSKEY_RF_INIT_TAB[i][0], RDA5875_PSKEY_RF_INIT_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  RDA5875_DelayMs(50);
  RDA5875_LDO_CMD(0);
  RDA5875_DelayMs(30);
  RDA5875_LDO_CMD(1);
  RDA5875_DelayMs(30);
  /*
    {0x40240000,0x2004f39c},
    C1:C47
    UartWaitMs(50);
    D1:D36
    UartWaitMs(10);
  */
  RDA5875_WriteReg(0, 0x40240000, 0x2004F39C);
  for(i = 0; i < 47; i++)
  {
    RDA5875_WriteReg(1, RDA5875_RF_INIT_TAB[i][0], RDA5875_RF_INIT_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  RDA5875_DelayMs(50);
  for(i = 0; i < 36; i++)
  {
    RDA5875_WriteReg(0, RDA5875_PSKEY_RF_INIT_TAB[i][0], RDA5875_PSKEY_RF_INIT_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  RDA5875_DelayMs(10);
  /*
    {0x003F,0x0000},
    {0x0030,0x0129},
    {0x0030,0x012B},
  */
  for(i = 0; i < 3; i++)
  {
    RDA5875_WriteReg(1, RDA5875_DC_CAL_INIT_TAB[i][0], RDA5875_DC_CAL_INIT_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  /*
    {0x40240000,0x2000F29C}
    E1:E56(E56+12)
  */
  for(i = 0; i < 56+12; i++)
  {
    RDA5875_WriteReg(0, RDA5875_TRAP_TAB[i][0], RDA5875_TRAP_TAB[i][1]);
    RDA5875_DelayMs(2);
  }
  /*
    {0x80000060, 0x000E1000}
    {0x80000040, 0x00000100}
  */

	
  RDA5875_DelayMs(200);
  RDA5875_WriteReg(0, 0x80000060, 0x000E1000);
  RDA5875_WriteReg(0, 0x80000040, 0x00000100);
  RDA5875_DelayMs(100);
  RDA5875_IOConfig(921600);
  RDA5875_WriteReg(0, 0x80000060, 0x000E1000);
  RDA5875_DelayMs(20);
  RDA5875_WriteReg(0, 0x80000060, 0x000E1000);
  RDA5875_DelayMs(20);
	RDA5875_WriteReg(0, 0x50000010, 0x00000122);
	RDA5875_DelayMs(10);
	
  RDA5875_Write((u8 *)RDA5875_ADDR, sizeof(RDA5875_ADDR));
  RDA5875_DelayMs(2);
  return k;
}

