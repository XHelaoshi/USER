//=====================================================================================================
// IMU.c
// S.O.H. Madgwick
// 25th September 2010
//=====================================================================================================
// Description:
//
// Quaternion implementation of the 'DCM filter' [Mayhony et al].
//
// User must define 'halfT' as the (sample period / 2), and the filter gains 'Kp' and 'Ki'.
//
// Global variables 'q0', 'q1', 'q2', 'q3' are the quaternion elements representing the estimated
// orientation.  See my report for an overview of the use of quaternions in this application.
//
// 用户必须调用 'IMUupdate()' 校准每个采样周期和解析 gyroscope ('gx', 'gy', 'gz')
// and 加速度计('ax', 'ay', 'ay') data.  陀螺仪的单位是弧度/秒，加速度
//单位是不相关的，是归为载体。
//
//=====================================================================================================

//----------------------------------------------------------------------------------------------------
// Header files

#include "IMU.h"
#include <math.h>

//----------------------------------------------------------------------------------------------------
// Definitions

#define Kp 2.0f			// 比例增益支配率收敛到加速度计/磁强计
#define Ki 0.005f		// 积分增益支配率的陀螺仪偏见的衔接
#define halfT 0.005f		// 采样周期的一半

#define RtA 		57.324841							
//---------------------------------------------------------------------------------------------------
// 变量定义

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;	// 四元数的元素，代表估计方向
float pitch = 0, roll = 0, yaw = 0;
float exInt = 0, eyInt = 0, ezInt = 0;	// 按比例缩小积分误差

//====================================================================================================
// Function
//====================================================================================================

// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float x)
{
  float halfx = 0.5f * x;
  float y = x;
  long i = *(long*)&y;
  i = 0x5f3759df - (i>>1);
  y = *(float*)&i;
  y = y * (1.5f - (halfx * y * y));
  return y;
}

void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az) {
	float norm;
	float vx, vy, vz;
	float ex, ey, ez;         
	
	// 测量正常化
	norm = invSqrt(ax*ax + ay*ay + az*az);       
	ax = ax * norm;
	ay = ay * norm;
	az = az * norm;      
	
	// 估计方向的重力
	vx = 2*(q1*q3 - q0*q2);
	vy = 2*(q0*q1 + q2*q3);
	vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;
	
	// 错误的领域和方向传感器测量参考方向之间的交叉乘积的总和
	ex = (ay*vz - az*vy);
	ey = (az*vx - ax*vz);
	ez = (ax*vy - ay*vx);
	
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
}

short abs_short(short in)
{
	return in>0?in:-in;
}

float abs_float(float x)
{
   if(x < 0)  return -x;
	 else return x; 
}
//void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
//{
//  float norm;
//  float vx, vy, vz;// wx, wy, wz;
//  float ex, ey, ez;

//  // 先把这些用得到的值算好
//  float q0q0 = q0*q0;
//  float q0q1 = q0*q1;
//  float q0q2 = q0*q2;
////  float q0q3 = q0*q3;//
//  float q1q1 = q1*q1;
////  float q1q2 = q1*q2;//
//  float q1q3 = q1*q3;
//  float q2q2 = q2*q2;
//  float q2q3 = q2*q3;
//  float q3q3 = q3*q3;
//	
//  if(ax*ay*az==0)
//	  return;
//		
//  norm = invSqrt(ax*ax + ay*ay + az*az);       //acc数据归一化
//  ax = ax *norm;
//  ay = ay * norm;
//  az = az * norm;

//  // estimated direction of gravity and flux (v and w)              估计重力方向和流量/变迁
//  vx = 2*(q1q3 - q0q2);												//四元素中xyz的表示
//  vy = 2*(q0q1 + q2q3);
//  vz = q0q0 - q1q1 - q2q2 + q3q3 ;

//  // error is sum of cross product between reference direction of fields and direction measured by sensors
//  ex = (ay*vz - az*vy) ;                           					 //向量外积在相减得到差分就是误差
//  ey = (az*vx - ax*vz) ;
//  ez = (ax*vy - ay*vx) ;

//  exInt = exInt + ex * Ki;								  //对误差进行积分
//  eyInt = eyInt + ey * Ki;
//  ezInt = ezInt + ez * Ki;
//// adjusted gyroscope measurements
//	if(abs_float(ay) < 0.8f && abs_float(az-1) < 0.8f)
//       gx = gx + Kp*ex + exInt;					   							//将误差PI后补偿到陀螺仪，即补偿零点漂移
//	else
//       gx = gx; 
//	if(abs_float(ax) < 0.8f && abs_float(az-1) < 0.8f)
//       gy = gy + Kp*ey + eyInt;				   							
//	else
//       gy = gy; 
//	//if(abs_float(ax)< 0.8 && abs_float(ay) < 0.8)
//   //    gz = gz + Kp*ez + ezInt;					   					//这里的gz由于没有观测者进行矫正会产生漂移，表现出来的就是积分自增或自减		
//	//else
//   //    gz = gz;   							
//	
//  // integrate quaternion rate and normalise						   //四元素的微分方程
//  q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
//  q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
//  q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
//  q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;

//  // normalise quaternion
//  norm = invSqrt(q0q0 + q1q1 + q2q2 + q3q3);
//  q0 = q0 * norm;
//  q1 = q1 * norm;
//  q2 = q2 * norm;
//  q3 = q3 * norm;
//}

//====================================================================================================
// END OF CODE
//====================================================================================================
