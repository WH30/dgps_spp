//
//  Cal_Sat_Pos.h
//  spp
//
//  Created by learning_wh on 2019/4/10.
//  Copyright © 2019 learning_wh. All rights reserved.
//
 
#ifndef Cal_Sat_Pos_h
#define Cal_Sat_Pos_h

#include <stdio.h>
#include "ReadData.h"
#include "Coordinate.h"
#include "Time.h"
#include "Matrix.h"

extern const double PI;      ///< 圆周率
extern const double C;       ///< 光速C
extern const double u;       ///< GM
extern const double Wmiga_e; ///< 地球自转角速度 rad/s
extern const double F;       ///< 相对论改正常量


/**********************************************
 *@brief  Cal_Sat_Pos    计算卫星位置和卫星速度
 
 *@param  eph          I   GPS星历数据
 *@param  gpstime      I   信号发射时的GPS时
 *@param  xyz          O   卫星的空间直角坐标
 *@param  vel          O   卫星的空间直角速度
 **********************************************/
void Cal_Sat_Pos(struct GPSEPH *eph,struct GPSTIME gpstime[], struct XYZ  xyz[],struct XYZ vel[]);


/**********************************************
 *@brief  Cal_Clock_Bia    计算卫星钟差和卫星钟速
 
 *@param  gpsephem     I   GPS星历数据
 *@param  gpstime      I   信号发射时的GPS时
 *@param  deta_t_svl1  O   卫星钟差
 *@param  d_clock_bia  O   卫星钟速
 **********************************************/
void Cal_Clock_Bia(struct GPSEPH * gpsephem,struct GPSTIME* gpstime,double* deta_t_svl1,double* d_clock_bia);


/**********************************************
 *@brief  Cal_Sat_Pos    计算卫星位置和卫星钟差
 
 *@param  gpsephem   I   GPS星历数据
 *@param  gpstime    I   接收到信号时的GPS时
 *@param  v          O   卫星在空间直角坐标系中的三轴速度
 **********************************************/
void Cal_Sat_Vec(struct GPSEPH * gpsephem,struct GPSTIME* gpstime, struct Matrix *v);


#endif /* Cal_Sat_Pos_h */
