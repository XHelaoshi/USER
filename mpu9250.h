#ifndef _MPU9250_H
#define _MPU9250_H

#include "stm32f4xx.h"

extern void Init_MPU9250(void);
extern void READ_MPU9250_ACCEL(void);
extern void READ_MPU9250_GYRO(void);
extern void READ_MPU9250_TEMP(void);
extern void READ_MPU9250_MAG(void);

extern float ACCEL_DATA[3];
extern float GYRO_DATA[3];
extern float MAG_DATA[3];
extern float TEMP_DATA[1];

extern float Gyro_Ex, Gyro_Ey, Gyro_Ez;
extern float Mag_Ex, Mag_Ey, Mag_Ez;
#endif
