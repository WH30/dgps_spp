#include<math.h>
#ifndef Coordinate_h
#define Coordinate_h
extern const double e2; ///< ��һƫ����ƽ��
extern const double a;  ///< ������

 
/*****************
 *@brief XYZ �ռ�ֱ������ϵ
 *****************/
struct XYZ
{
    double x;    ///< �ռ�ֱ������x
    double y;    ///< �ռ�ֱ������y
    double z;    ///< �ռ�ֱ������z
};

/*****************
 *@brief BLH �������
 *****************/
struct BLH
{
    double L;    ///< ���� L
    double B;    ///< γ�� B
    double H;    ///< �߳� h
};


/**********************************************
 *@brief  XYZToBLH   �ռ�ֱ������ת�������
 *@param  xyz   I   �ռ�ֱ������
 *@param  blh   O   �������
 *@note �������ĽǶȵ�λΪ����
 **********************************************/
void XYZToBLH(const struct XYZ* xyz,struct BLH* blh);


/**********************************************
 *@brief  BLHToXYZ   �������ת�ռ�ֱ������
 *@param  blh   I   �������
 *@param  xyz   O   �ռ�ֱ������
 *@note �������ĽǶȵ�λΪ����
 **********************************************/
void BLHToXYZ(const struct BLH* blh,struct XYZ* xyz);


/**********************************************
 *@brief  Distance   ���������ռ�ֱ��������ŷʽ����
 *@param  p1   I   ��һ����Ŀռ�ֱ������
 *@param  p2   I   �ڶ�����Ŀռ�ֱ������
 *@retval ������ŷʽ���� ��λ:m
 **********************************************/
double Distance(struct XYZ *p1 , struct XYZ *p2);

#endif


