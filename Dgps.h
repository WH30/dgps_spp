#pragma once
#ifndef Dgps_h
#define Dgps_h

#include<stdio.h>
#define BUFFSIZE 20480

/************************************
 *@brief  PsrCrrect  伪距改正结构体
 ************************************/
struct PsrCrrect
{
	double Zcount;  ///<z计数   13位2进制乘 0.6   一个小时内的秒数
	int s;    ///<精度标识
	int PRN;  ///<卫星号
	short prc; ///<卫星伪距原始原始改正
	short rrc;  ///<卫星伪距改正变化率
	unsigned int age; ///<iode
};

/************************************
 *@brief  convert16  实现字符1-6位的滚动
 *@param  c    I   待滚动的字符
 *@retval   滚动后的字符
 ************************************/
char convert16(char c);


/************************************
 *@brief     DecodePsrCroct  实现对RTCM2.x的解码
 *@param  buff           I  缓存区指针
 *@param  lenth         I  本次读入数据的长度
 *@param  psr_cro     O  解码得到的差分改正结构体
 *@retval    -1            解码失败
 ************************************/
int DecodePsrCroct(char * buff, int lenth, struct PsrCrrect  * psr_cro);

/************************************
 *@brief     check        实现对RTCM2.x一个word的校验
 *@param  sysbuff      I  合并后数据缓存区指针
 *@param  pos2         I  一个word的开头
 *@retval    -1            校验失败
 *@retval    1             校验成功
 ************************************/
int check(char *sysbuff, int pos2);


#endif // !DGPS_H
