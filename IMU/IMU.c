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
// �û�������� 'IMUupdate()' У׼ÿ���������ںͽ��� gyroscope ('gx', 'gy', 'gz')
// and ���ٶȼ�('ax', 'ay', 'ay') data.  �����ǵĵ�λ�ǻ���/�룬���ٶ�
//��λ�ǲ���صģ��ǹ�Ϊ���塣
//
//=====================================================================================================

//----------------------------------------------------------------------------------------------------
// Header files

#include "IMU.h"
#include <math.h>

//----------------------------------------------------------------------------------------------------
// Definitions

#define Kp 2.0f			// ��������֧�������������ٶȼ�/��ǿ��
#define Ki 0.005f		// ��������֧���ʵ�������ƫ�����ν�
#define halfT 0.005f		// �������ڵ�һ��

#define RtA 		57.324841							
//---------------------------------------------------------------------------------------------------
// ��������

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;	// ��Ԫ����Ԫ�أ�������Ʒ���
float pitch = 0, roll = 0, yaw = 0;
float exInt = 0, eyInt = 0, ezInt = 0;	// ��������С�������

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
	
	// ����������
	norm = invSqrt(ax*ax + ay*ay + az*az);       
	ax = ax * norm;
	ay = ay * norm;
	az = az * norm;      
	
	// ���Ʒ��������
	vx = 2*(q1*q3 - q0*q2);
	vy = 2*(q0*q1 + q2*q3);
	vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;
	
	// ���������ͷ��򴫸��������ο�����֮��Ľ���˻����ܺ�
	ex = (ay*vz - az*vy);
	ey = (az*vx - ax*vz);
	ez = (ax*vy - ay*vx);
	
	// ������������������
	exInt = exInt + ex*Ki;
	eyInt = eyInt + ey*Ki;
	ezInt = ezInt + ez*Ki;
	
	// ������������ǲ���
	gx = gx + Kp*ex + exInt;
	gy = gy + Kp*ey + eyInt;
	gz = gz + Kp*ez + ezInt;
	
	// ������Ԫ���ʺ�������
	q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
	q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
	q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
	q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;  
	
	// ��������Ԫ
	norm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
	q0 = q0 * norm;
	q1 = q1 * norm;
	q2 = q2 * norm;
	q3 = q3 * norm;
  
  //ת��Ϊ�Ƕ�
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

//  // �Ȱ���Щ�õõ���ֵ���
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
//  norm = invSqrt(ax*ax + ay*ay + az*az);       //acc���ݹ�һ��
//  ax = ax *norm;
//  ay = ay * norm;
//  az = az * norm;

//  // estimated direction of gravity and flux (v and w)              �����������������/��Ǩ
//  vx = 2*(q1q3 - q0q2);												//��Ԫ����xyz�ı�ʾ
//  vy = 2*(q0q1 + q2q3);
//  vz = q0q0 - q1q1 - q2q2 + q3q3 ;

//  // error is sum of cross product between reference direction of fields and direction measured by sensors
//  ex = (ay*vz - az*vy) ;                           					 //�������������õ���־������
//  ey = (az*vx - ax*vz) ;
//  ez = (ax*vy - ay*vx) ;

//  exInt = exInt + ex * Ki;								  //�������л���
//  eyInt = eyInt + ey * Ki;
//  ezInt = ezInt + ez * Ki;
//// adjusted gyroscope measurements
//	if(abs_float(ay) < 0.8f && abs_float(az-1) < 0.8f)
//       gx = gx + Kp*ex + exInt;					   							//�����PI�󲹳��������ǣ����������Ư��
//	else
//       gx = gx; 
//	if(abs_float(ax) < 0.8f && abs_float(az-1) < 0.8f)
//       gy = gy + Kp*ey + eyInt;				   							
//	else
//       gy = gy; 
//	//if(abs_float(ax)< 0.8 && abs_float(ay) < 0.8)
//   //    gz = gz + Kp*ez + ezInt;					   					//�����gz����û�й۲��߽��н��������Ư�ƣ����ֳ����ľ��ǻ����������Լ�		
//	//else
//   //    gz = gz;   							
//	
//  // integrate quaternion rate and normalise						   //��Ԫ�ص�΢�ַ���
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
