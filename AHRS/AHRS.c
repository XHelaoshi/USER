//=====================================================================================================
// AHRS.c
// S.O.H. Madgwick
// 25th August 2010
//=================================================================================================// Description:
//
// 四元的“DCM的过滤器”的实施[Mayhony等]。采用了磁性失真
// 我的过滤器[Madgwick]省去了补偿算法

//引用
// 通量（BX BZ）的方向，是预定义的和限制的磁场效应

//扭曲到偏航
// axis only.
//
// User must define 'halfT' as the (sample period / 2), and the filter gains 'Kp' and 'Ki'.
//
// Global variables 'q0', 'q1', 'q2', 'q3' are the quaternion elements representing the estimated
// 方向.  见我概述使用这个应用程序中的四元报告。
//
// 用户必须调用 'AHRSupdate()' 校准每个采样周期和解析 gyroscope ('gx', 'gy', 'gz'),
// 加速度计 ('ax', 'ay', 'ay') and 磁力仪('mx', 'my', 'mz') data.  Gyroscope units are
// 弧度/秒，加速度计和磁强计单位归为载体无关，是归为载体。
//
//=====================================================================================================

//----------------------------------------------------------------------------------------------------
// Header files

#include "AHRS.h"
#include "mpu9250.h"
#include "oled.h"
#include <math.h>
extern void DelayUS(__IO uint32_t nTime);
//----------------------------------------------------------------------------------------------------
// Definitions

#define Kp 5.0f			// 比例增益支配收敛率accelerometer/magnetometer
#define Ki 0.003f		// 积分增益执政速率陀螺仪的衔接gyroscopeases
#define halfT 0.005f		// 采样周期的一半

#define RtA 		(180.0f/3.141593f)
//---------------------------------------------------------------------------------------------------
// Variable definitions

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;	// 四元数的元素，代表估计方向

float pitch = 0, roll = 0, yaw = 0;

float exInt = 0, eyInt = 0, ezInt = 0;	// 按比例缩小积分误差

//====================================================================================================
// Function
//====================================================================================================
float invSqrt(float x)
{
  float halfx = 0.5f * x;
  float y = x;
  long i = *(long*)&y;
  i = 0x5f375a86 - (i>>1);
  y = *(float*)&i;
  y = y * (1.5f - (halfx * y * y));
  y = y * (1.5f - (halfx * y * y));
  return y;
}

void AHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float norm;
	float hx, hy, hz, bx, bz;
	float vx, vy, vz, wx, wy, wz;
	float ex, ey, ez;

	// 辅助变量，以减少重复操作数
	float q0q0 = q0*q0;
	float q0q1 = q0*q1;
	float q0q2 = q0*q2;
	float q0q3 = q0*q3;
	float q1q1 = q1*q1;
	float q1q2 = q1*q2;
	float q1q3 = q1*q3;
	float q2q2 = q2*q2;   
	float q2q3 = q2*q3;
	float q3q3 = q3*q3;          
	
	// 测量正常化
	norm = invSqrt(ax*ax + ay*ay + az*az);
	ax = ax * norm;
	ay = ay * norm;
	az = az * norm;
	norm = invSqrt(mx*mx + my*my + mz*mz);
	mx = mx * norm;
	my = my * norm;
	mz = mz * norm;         
	
	// 计算参考磁通方向
	hx = 2.0f*mx*(0.5f - q2q2 - q3q3) + 2.0f*my*(q1q2 - q0q3) + 2.0f*mz*(q1q3 + q0q2);
	hy = 2.0f*mx*(q1q2 + q0q3) + 2.0f*my*(0.5f - q1q1 - q3q3) + 2.0f*mz*(q2q3 - q0q1);
	hz = 2.0f*mx*(q1q3 - q0q2) + 2.0f*my*(q2q3 + q0q1) + 2.0f*mz*(0.5f - q1q1 - q2q2);         
	bx = sqrt((hx*hx) + (hy*hy));
	bz = hz;        
	
	//估计方向的重力和磁通（V和W）
	vx = 2.0f*(q1q3 - q0q2);
	vy = 2.0f*(q0q1 + q2q3);
	vz = q0q0 - q1q1 - q2q2 + q3q3;
	wx = 2.0f*bx*(0.5f - q2q2 - q3q3) + 2.0f*bz*(q1q3 - q0q2);
	wy = 2.0f*bx*(q1q2 - q0q3) + 2.0f*bz*(q0q1 + q2q3);
	wz = 2.0f*bx*(q0q2 + q1q3) + 2.0f*bz*(0.5f - q1q1 - q2q2);  
	
	// 错误是跨产品的总和之间的参考方向的领域和方向测量传感器
	ex = (ay*vz - az*vy) + (my*wz - mz*wy);
	ey = (az*vx - ax*vz) + (mz*wx - mx*wz);
	ez = (ax*vy - ay*vx) + (mx*wy - my*wx);
	
	// 积分误差比例积分增益
	exInt = exInt + ex*Ki;
	eyInt = eyInt + ey*Ki;
	ezInt = ezInt + ez*Ki;
	
	// 调整后的陀螺仪测量
	gx = gx + Kp*ex + exInt;
	gy = gy + Kp*ey + eyInt;
	gz = gz + Kp*ez + ezInt;
	
	// 整合四元数率和正常化
	q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
	q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
	q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
	q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  
	
	// 正常化四元
	norm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
	q0 = q0 * norm;
	q1 = q1 * norm;
	q2 = q2 * norm;
	q3 = q3 * norm;
  
  //转化为角度
  pitch = asin(-2.0f * q1 * q3 + 2.0f * q0* q2)* RtA;	// pitch
  roll  = atan2(2.0f * q2 * q3 + 2.0f * q0 * q1, -2.0f * q1 * q1 - 2.0f * q2* q2 + 1.0f)* RtA;	// roll
  yaw   = atan2(2.0f*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * RtA;	//yaw
  if(yaw<0) yaw+=360;
  else if(yaw>360) yaw-=360;
}


void init_quaternion(void)
{ 
  //float init_Yaw, init_Pitch, init_Roll;
  //float init_ax, init_ay, init_az;
  //float init_mx, init_my, init_mz;
  //float norm;
  float gyro_sum = 10.0f;
  float gyro_x, gyro_y, gyro_z;
  float mag_x_max = -5000.0f, mag_x_min = 5000.0f;
  float mag_y_max = -5000.0f, mag_y_min = 5000.0f;
  float mag_z_max = -5000.0f, mag_z_min = 5000.0f;
  #define mag_calx_th 70.0f
  #define mag_caly_th 70.0f
  #define mag_calz_th 50.0f
  int i;
  
  OLED_String(1,0,">Gyroscope Calibration..."); //陀螺仪校准
  do
  {
    gyro_sum = 0;
    gyro_x = 0;
    gyro_y = 0;
    gyro_z = 0;
    for(i=0;i<50;i++)
    {
      READ_MPU9250_GYRO();
      DelayUS(10000);
      gyro_sum += sqrt(GYRO_DATA[0]*GYRO_DATA[0] + GYRO_DATA[1]*GYRO_DATA[1] + GYRO_DATA[2]*GYRO_DATA[2]);
      gyro_x += GYRO_DATA[0];
      gyro_y += GYRO_DATA[1];
      gyro_z += GYRO_DATA[2];
    }
    //__nop();
  }
  while(gyro_sum > 0.2f * 50.0f); 
  Gyro_Ex = gyro_x / 50.0f;
  Gyro_Ey = gyro_y / 50.0f;
  Gyro_Ez = gyro_z / 50.0f;
  
  OLED_String(2,0,">Compass Calibration... ");
  do{
    for(i=0;i<100;i++)
    {
      READ_MPU9250_MAG();
      if(MAG_DATA[0] > mag_x_max) mag_x_max = MAG_DATA[0];
      if(MAG_DATA[0] < mag_x_min) mag_x_min = MAG_DATA[0];
      if(MAG_DATA[1] > mag_y_max) mag_y_max = MAG_DATA[1];
      if(MAG_DATA[1] < mag_y_min) mag_y_min = MAG_DATA[1];
      if(MAG_DATA[2] > mag_z_max) mag_z_max = MAG_DATA[2];
      if(MAG_DATA[2] < mag_z_min) mag_z_min = MAG_DATA[2];
      DelayUS(10000);
    }
  }
  while((mag_x_max-mag_x_min<mag_calx_th)||(mag_y_max-mag_y_min<mag_caly_th)||(mag_z_max-mag_z_min<mag_calz_th));
  Mag_Ex = (mag_x_max+mag_x_min) / 2.0f;
  Mag_Ey = (mag_y_max+mag_y_min) / 2.0f;
  Mag_Ez = (mag_z_max+mag_z_min) / 2.0f;

  /*
  norm = invSqrt(ACCEL_DATA[0]*ACCEL_DATA[0] + ACCEL_DATA[1]*ACCEL_DATA[1] + ACCEL_DATA[2]*ACCEL_DATA[2]);
	init_ax = ACCEL_DATA[0] * norm;
	init_ay = ACCEL_DATA[1] * norm;
	init_az = ACCEL_DATA[2] * norm;
  
  norm = invSqrt(MAG_DATA[0]*MAG_DATA[0] + MAG_DATA[1]*MAG_DATA[1] + MAG_DATA[2]*MAG_DATA[2]);       
	init_mx = MAG_DATA[0] * norm;
	init_my = MAG_DATA[1] * norm;
	init_mz = MAG_DATA[2] * norm;

  init_Roll  = atan2(init_ay, init_az);
  init_Pitch = -asin(init_ax);              //init_Pitch = asin(ax / 1);      
  init_Yaw   = -atan2(init_mx*cos(init_Roll) + init_my*sin(init_Roll)*sin(init_Pitch) + init_mz*sin(init_Roll)*cos(init_Pitch),
                      init_my*cos(init_Pitch) - init_mz*sin(init_Pitch));
                   //atan2(mx, my);
  
  if(init_Yaw < 0){init_Yaw = init_Yaw + 2*3.141593f;}
  if(init_Yaw > 2*3.141593f){init_Yaw = init_Yaw - 2*3.141593f;}

  q0 = cos(0.5f*init_Roll)*cos(0.5f*init_Pitch)*cos(0.5f*init_Yaw) + sin(0.5f*init_Roll)*sin(0.5f*init_Pitch)*sin(0.5f*init_Yaw);  //w
  q1 = sin(0.5f*init_Roll)*cos(0.5f*init_Pitch)*cos(0.5f*init_Yaw) - cos(0.5f*init_Roll)*sin(0.5f*init_Pitch)*sin(0.5f*init_Yaw);  //x   绕x轴旋转是roll
  q2 = cos(0.5f*init_Roll)*sin(0.5f*init_Pitch)*cos(0.5f*init_Yaw) + sin(0.5f*init_Roll)*cos(0.5f*init_Pitch)*sin(0.5f*init_Yaw);  //y   绕y轴旋转是pitch
  q3 = cos(0.5f*init_Roll)*cos(0.5f*init_Pitch)*sin(0.5f*init_Yaw) - sin(0.5f*init_Roll)*sin(0.5f*init_Pitch)*cos(0.5f*init_Yaw);  //z   绕z轴旋转是Yaw
*/
}

//====================================================================================================
// END OF CODE
//====================================================================================================
