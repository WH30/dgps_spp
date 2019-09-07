//
//  Bias_Correction.h
//  spp
//
//  Created by learning_wh on 2019/4/12.
//  Copyright © 2019 learning_wh. All rights reserved.
//
 
#ifndef Bias_Correction_h
#define Bias_Correction_h

#include <stdio.h>
#include "Time.h"
#include "Coordinate.h"
#include "ReadData.h"
#include "Cal_Sat_Pos.h"

/**********************************************
 *@brief  Klobutcharh   Klobutcharh电离层改正改正模型
 
 *@param  ionutc   I   电离层数据
 *@param  gpstime  I   观测的gps时
 *@param  blh      I   大地坐标
 *@param  E        I   高度角
 *@param  A        I   方位角
 
 *@return Klobutchar模型计算出的电离层延迟，单位s
 **********************************************/
double Klobutchar(struct IONUTC * ionutc,struct GPSTIME * gpstime, struct BLH * blh, double E, double A);
//double Klobutchar(struct IONUTC * ionutc,struct GPSTIME * gpstime, struct BLH * blh, struct XYZ * sat_xyz);




/**********************************************
 *@brief  Cal_E_A    根据接收机的位置和卫星的位置计算卫星相对于测站的高度角和方位角
 
 *@param  rec_xyz   I   测站空间直角坐标
 *@param  sat_xyz   I   卫星空间直角坐标
 *@param  E         O   高度角
 *@param  A         O   方位角
 **********************************************/
void Cal_E_A(struct XYZ *rec_xyz , struct XYZ *sat_xyz , double *E,double *A);



/**********************************************
 *@brief  Hopfield   Hopfield对流层改正模型
 
 *@param  E   I   卫星相对测站的高度角
 *@param  H   I   测站高度

 *@note   H0:海平面高度  T0:标准温度  p0:气压  RH0:相对湿度 均使用默认值
 *@return Hopfield模型计算出的距离改正 单位m
 **********************************************/
double Hopfield(double E,double H);






double Hopfield_DIY(double E,double H,double H0,double T0,double p0,double RH0);
//  Hopefield 改正模型  E:卫星相对测站的高度角 H:测站高度  T:测站的干温  p:气压  RH:相对湿度
//  H0:海平面高度  T0:标准温度  p0:气压  RH0:相对湿度


#endif /* Bias_Correction_h */
