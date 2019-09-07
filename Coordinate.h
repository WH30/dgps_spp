#include<math.h>
#ifndef Coordinate_h
#define Coordinate_h
extern const double e2; ///< 第一偏心率平方
extern const double a;  ///< 长半轴

 
/*****************
 *@brief XYZ 空间直角坐标系
 *****************/
struct XYZ
{
    double x;    ///< 空间直角坐标x
    double y;    ///< 空间直角坐标y
    double z;    ///< 空间直角坐标z
};

/*****************
 *@brief BLH 大地坐标
 *****************/
struct BLH
{
    double L;    ///< 经度 L
    double B;    ///< 纬度 B
    double H;    ///< 高程 h
};


/**********************************************
 *@brief  XYZToBLH   空间直角坐标转大地坐标
 *@param  xyz   I   空间直角坐标
 *@param  blh   O   大地坐标
 *@note 大地坐标的角度单位为弧度
 **********************************************/
void XYZToBLH(const struct XYZ* xyz,struct BLH* blh);


/**********************************************
 *@brief  BLHToXYZ   大地坐标转空间直角坐标
 *@param  blh   I   大地坐标
 *@param  xyz   O   空间直角坐标
 *@note 大地坐标的角度单位为弧度
 **********************************************/
void BLHToXYZ(const struct BLH* blh,struct XYZ* xyz);


/**********************************************
 *@brief  Distance   计算两个空间直角坐标点的欧式距离
 *@param  p1   I   第一个点的空间直角坐标
 *@param  p2   I   第二个点的空间直角坐标
 *@retval 两点间的欧式距离 单位:m
 **********************************************/
double Distance(struct XYZ *p1 , struct XYZ *p2);

#endif


