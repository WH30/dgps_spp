//
//  Bias_Correction.c
//  spp
//
//  Created by learning_wh on 2019/4/12.
//  Copyright © 2019 learning_wh. All rights reserved.
//

#include "Bias_Correction.h"

/**********************************************
 *@brief  Klobutcharh   Klobutcharh电离层改正改正模型
 
 *@param  ionutc   I   电离层数据
 *@param  gpstime  I   观测的gps时
 *@param  blh      I   大地坐标
 *@param  E        I   高度角
 *@param  A        I   方位角
 
 *@return Klobutchar模型计算出的距离改正
 **********************************************/
double Klobutchar(struct IONUTC * ionutc,struct GPSTIME * gpstime, struct BLH * blh, double E, double A)
{
    //struct XYZ rec_xyz;
    //BLHToXYZ(blh, &rec_xyz);
   // double E=0,A=0;
   // Cal_E_A(&rec_xyz,sat_xyz, &E, &A); ///< 计算方位角和高度角
   // if(E/PI*180<15) return 0;
    double fai=0.0137*1.0/(E/PI+0.11) - 0.022;  ///< Earth center angle
    double Fai_I = blh->B/PI + fai *cos(A);   ///< 电离层纬度 Fai_I;
    if(Fai_I > 0.416) Fai_I =0.416;
    else if(Fai_I < -0.416) Fai_I = -0.416;
    double Lon_I = blh->L/PI + fai*sin(A)*1.0/cos(Fai_I*PI);
    
    double Fai_m=Fai_I + 0.064*cos(Lon_I*PI-1.617); // 地理纬度 Fai_m;
    double t_gps=gpstime->SecOfWeek ;//+ gpstime->Week*604800;
    double t=43200*Lon_I +t_gps;
    while(t>=86400) t-=86400;
    while(t<0) t+=86400;
    
    double A_I= ionutc->a0 + ionutc->a1 * Fai_m + ionutc->a2 * Fai_m *Fai_m + ionutc->a3*Fai_m*Fai_m*Fai_m;
    if(A_I<0) A_I=0;
    double P_I=ionutc->b0 + ionutc->b1 * Fai_m + ionutc->b2 * Fai_m *Fai_m + ionutc->b3*Fai_m*Fai_m*Fai_m;
    if(P_I<72000) P_I=72000;
    double X_I=2*PI*(t-50400)*1.0/P_I;  ///< rad
    double F=1.0 + 16.0*  pow( (0.53-E/PI),3);
    
    double T_iono;
    if(fabs(X_I)<1.57) T_iono=F*( 5*pow(10, -9) + A_I * (1-X_I*X_I*0.5 +pow(X_I, 4)*1.0/24) ) ;
    else T_iono=F* 5*pow(10, -9);
    
    return T_iono;
}




/**********************************************
 *@brief  Hopfield   Hopfield对流层改正模型
 
 *@param  E   I   卫星相对测站的高度角
 *@param  H   I   测站高度
 
 *@note   H0:海平面高度  T0:标准温度  p0:气压  RH0:相对湿度 均使用默认值
 *@return Hopfield模型计算出的距离改正
 **********************************************/
double Hopfield(double E,double H)
{
    ///计算时，E以度为单位，计算结果是米
    if(H<0) return 0;
    E=E*180/PI;
    if(E<15) return 0;
    double H0=0;
    if( 1-0.0000226*(H-H0) <0 ) return 0;
    double T0=15+273.15;    ///< 物理温度，+273.16/5 ？
    double p0=1013.25;
    double RH0=0.5;
    double RH=RH0 * exp(-0.0006396*(H-H0));
    double p= p0* ( pow ( 1-0.0000226*(H-H0) , 5.225) ) ;
    double T= T0-0.0065*(H-H0);
    double e=RH * exp(0-37.2465 + 0.213166*T - 0.000256908*T*T );
    double hw=11000;
    double hd=40136+148.72*(T0-273.16);

    double Kw=155.2*pow(10, -7) *4810/T/T*e*(hw-H);
    double Kd=155.2*pow(10, -7) * p/T*(hd-H);
    double s1=sin(sqrt(E*E+6.25)/180*PI);
    double s2=sin(sqrt(E*E+2.25)/180*PI);
    double T_trop= Kd/s1  +  Kw /s2;
    
    return T_trop;
}



double Hopfield_DIY(double E,double H,double H0,double T0,double p0,double RH0)
{
    double RH=RH0 * exp(-0.0006396*(H-H0));
    double p= p0* ( pow ( 1-0.0000226*(H-H0) , 5.225) ) ;
    double T= T0-0.0065*(H-H0);
    double e=RH * exp(0-37.2465 + 0.213166*T - 0.000256908*T*T );
    double hw=11000;
    double hd=40136+148.72*(T0-273.16);
    double Kw=155.2*pow(10, -7) *4810/T/T*e*(hw-H);
    double Kd=155.2*pow(10, -7) * p/T*(hd-H);
    double T_trop=Kd/( sqrt(sin(E*E+6.25)) )  +  Kw /(sqrt(sin(E*E+2.25)));
    
    return T_trop;
}



/**********************************************
 *@brief  Cal_E_A    根据接收机的位置和卫星的位置计算卫星相对于测站的高度角和方位角
 
 *@param  rec_xyz   I   测站空间直角坐标
 *@param  sat_xyz   I   卫星空间直角坐标
 *@param  E         I   高度角
 *@param  A         I   方位角
 **********************************************/
void Cal_E_A(struct XYZ *rec_xyz , struct XYZ *sat_xyz , double *E,double *A)
{
    if( (rec_xyz->x*rec_xyz->x + rec_xyz->y*rec_xyz->y + rec_xyz->z*rec_xyz->z ) < 10000)
    {
        *E=PI/2;
        *A=0;
        return ;
    }
    struct BLH rec_blh;
    XYZToBLH(rec_xyz, &rec_blh);
    struct Matrix vec_recTosat; ///< 接收机到卫星的向量在地心地固系下的表示
    vec_recTosat.Row=3; vec_recTosat.Col=1;
    vec_recTosat.Val[0]=sat_xyz->x - rec_xyz->x;
    vec_recTosat.Val[1]=sat_xyz->y - rec_xyz->y;
    vec_recTosat.Val[2]=sat_xyz->z - rec_xyz->z;
     
    ///单位化
   // double lenth=sqrt(vec_recTosat.Val[0]*vec_recTosat.Val[0] + vec_recTosat.Val[1]*vec_recTosat.Val[1] +vec_recTosat.Val[2]*vec_recTosat.Val[2] );
   // for(int i=0;i<3;i++)
    //    vec_recTosat.Val[i]/=lenth;
    
    
    struct Matrix Er;           ///< 地心地固系到站心地平系的转换矩阵
    Er.Row=3;Er.Col=3;
    Er.Val[0]= -sin(rec_blh.L) ;                 Er.Val[1]=  cos(rec_blh.L);                  Er.Val[2]=0;
    Er.Val[3]= -sin(rec_blh.B)*cos(rec_blh.L);   Er.Val[4]= -sin(rec_blh.B)*sin(rec_blh.L);   Er.Val[5]=cos(rec_blh.B);
    Er.Val[6]= cos(rec_blh.B)*cos(rec_blh.L);    Er.Val[7]= cos(rec_blh.B)*sin(rec_blh.L);    Er.Val[8]=sin(rec_blh.B);
    
    struct Matrix vec_local;    ///< 接收机到卫星的向量在站心地平系下的表示
    vec_local.Row=3; vec_local.Col=1;
    Matrix_Mutiply(&Er, &vec_recTosat,  &vec_local);
    double lenth=sqrt(vec_local.Val[0]*vec_local.Val[0] + vec_local.Val[1]*vec_local.Val[1] +vec_local.Val[2]*vec_local.Val[2] );
    *A=atan2(vec_local.Val[0], vec_local.Val[1]);
    if(*A<0) *A+=2*PI;
    *E=asin(vec_local.Val[2]/lenth);
}



