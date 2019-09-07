//
//  Coordinate.c
//  spp
//
//  Created by learning_wh on 2019/4/12.
//  Copyright © 2019 learning_wh. All rights reserved.
//

#include <stdio.h>
#include "Coordinate.h"

extern const double e2=0.0066943799901;   ///< 第一偏心率平方
extern const double a=6.3781370e6;        ///< 长半轴

 
/**********************************************
 *@brief  XYZToBLH   空间直角坐标转大地坐标
 
 *@param  xyz   I   空间直角坐标
 *@param  blh   O   大地坐标
 
 *@note 大地坐标的角度单位为弧度
 **********************************************/
void XYZToBLH(const struct XYZ* xyz,struct BLH* blh)
{
    double x=xyz->x; double y=xyz->y; double z=xyz->z;
    double L=atan2(y,x);
    double fai=atan(z*1.0/sqrt(x*x+y*y)),R=sqrt(x*x + y*y +z*z);
    double H=0; double B=fai;     ///<初值
    double lastH=-100;
    double W=0,N=0;
    while(fabs(lastH-H)>0.1)
    {
        lastH=H;
        W=sqrt(1-e2*sin(B)*sin(B));
        N=a*1.0/W;
        B=atan(tan(fai)* (1+ a*e2/z*sin(B)/W) );
        H=R*cos(fai)*1.0/cos(B)-N;
    }
    blh->B=B;
    blh->L=L;
    blh->H=H;
}


/**********************************************
 *@brief  BLHToXYZ   大地坐标转空间直角坐标
 
 *@param  blh   I   大地坐标
 *@param  xyz   O   空间直角坐标
 
 *@note 大地坐标的角度单位为弧度
 **********************************************/
void BLHToXYZ(const struct BLH* blh, struct XYZ* xyz)
{
    double B = blh->B;
    double L = blh->L;
    double H = blh->H;
    double N=a/sqrt(1-e2*sin(B)*sin(B));///<卯酉圈半径
    xyz->x=(N+H)*cos(B)*cos(L);
    xyz->y=(N+H)*cos(B)*sin(L);
    xyz->z=(N*(1-e2)+H)*sin(B);
}


/**********************************************
 *@brief  Distance   计算两个空间直角坐标点的欧式距离
 
 *@param  p1   I   第一个点的空间直角坐标
 *@param  p2   I   第二个点的空间直角坐标
 
 *@retval 两点间的欧式距离 单位:m
 **********************************************/
double Distance(struct XYZ *p1 , struct XYZ *p2)
{
    double d=sqrt(  (p1->x - p2->x) * (p1->x - p2->x) + (p1->y - p2->y) * (p1->y - p2->y) +(p1->z - p2->z) * (p1->z - p2->z)  );
    return d;
}
