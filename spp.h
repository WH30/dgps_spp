//
//  spp.h
//  spp_1.0
//
//  Created by learning_wh on 2019/4/19.
//  Copyright © 2019 learning_wh. All rights reserved.
//
 
#ifndef spp_h
#define spp_h

#include <stdio.h>
#include <stdlib.h>
#include "Time.h"
#include "ReadData.h"
#include "Bias_Correction.h"
#include"Dgps.h"

/**********************************************
 *@brief  Cal_T_tr_GPS    计算卫星发射信号的GPS时
 
 *@param  epoch_obs   I   一个历元的观测数据
 *@param  eph         I   GPS卫星星历，大小为32的数组
 *@param  t_tr_gps    O   卫星发射信号的GPS时,数组，记录所有GPS卫星发射时间，数组下标对应PRN-1
 *@param  deta_t_tr   O   GPS卫星发射信号时的钟差
 *@param  d_clock_bia O   GPS卫星发射信号时的钟速
 **********************************************/
void Cal_T_Tr_GPS(struct EPOCHOBS *epoch_obs, struct GPSEPH eph[], struct GPSTIME t_tr_gps[],double deta_t_tr[],double d_clock_bia[]);



/**********************************************
 *@brief  Cal_Rec_Pos    计算接收机的位置
 
 *@param  epoch_obs    I   一个历元的观测数据
 *@param  gps_xyz      I   GPS卫星位置,大小为32的数组
 *@param  Ion          I   电离层数据
 *@param  Deta_t_tr    I   卫星钟差的改正值,32的数组
 *@param  receive_xyz  O   接收机的位置
 **********************************************/
void Cal_Rec_Pos(struct EPOCHOBS *epoch_obs,struct XYZ gps_xyz[],struct IONUTC *Ion, double Deta_t_tr[],struct XYZ *receive_xyz, struct PsrCrrect Psrcre[], GPSEPH  gpseph[],int flag);


/**********************************************
 *@brief  Cal_Rec_Vel    计算接收机的速度
 
 *@param  epoch_obs    I   一个历元的观测数据
 *@param  gps_xyz      I   GPS卫星位置,大小为32的数组
 *@param  gps_vel      I   GPS卫星速度,大小为32的数组
 *@param  d_clock_bia  I   GPS卫星钟差,大小为32的数组
 *@param  receive_vel  O   接收机的位置
 **********************************************/
int Cal_Rec_Vel(struct EPOCHOBS *epoch_obs,struct XYZ gps_xyz[],struct XYZ gps_vel[],double d_clock_bia[],struct XYZ *receive_pos,struct XYZ *receive_vel);


#endif /* spp_h */
