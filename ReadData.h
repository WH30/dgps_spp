//
//  ReadData.h
//  ReadData
//
//  Created by learning_wh on 2019/4/3.
//  Copyright © 2019 learning_wh. All rights reserved.
//
  
#ifndef ReadData_h
#define ReadData_h

#include <stdio.h>
#include "Cal_Sat_Pos.h"
#include "Time.h"
#define MAXCHANNUM 32

//unsiged long 为8个字节 与所给的标准4j字节不符，改为int

/*****************
 *@brief GNSSSYS GNSS系统类型
 *****************/
enum GNSSSYS{
    OTHERS, GPS, BDS ,GALILEO, GLONASS
};


/*****************
 *@brief GPSEPH GPS星历
 *****************/
struct GPSEPH
{
    unsigned int    RRN;            ///< 卫星号
    double          tow;            ///< 周内秒数
    enum            GNSSSYS SatSys;
    unsigned int    HEALTH;         ///< 卫星健康状态
    unsigned int    IODE1;          ///< 数据、星历发布时间
    unsigned int    IODE2;
    //struct GPSTIME          TOE;//星历的参考时间，计算卫星位置时使用该时间
    //struct GPSTIME          TOC;
    unsigned int    week;           ///< gps参考周
    unsigned int    z_week;
    double          toe;            ///< 星历参考时间秒
    double          A;              ///< 卫星轨道半长轴，单位：m
    double          delta_n;        ///< rad/s
    double          M0;//rad
    double          e;              ///< 轨道偏心率
    double          wmiga;          ///< 近地点角距，rad
    double          Cuc;
    double          Cus;
    double          Crc;
    double          Crs;
    double          Cic;
    double          Cis;
    double          I0;             ///< 参考时间倾角，rad
    double          I_DOT;
    double          wmiga0;
    double          wmiga_DOT;
    unsigned int    IODC;   
    double          toc;
    double          TGD;
    double          af0;
    double          af1;
    double          af2;
    unsigned long   AS;
    double          N;              ///<修正平均运动
    double          URA;
};

/*****************
 *@brief IONUTC 电离层信息
 *****************/
struct IONUTC
{
    int             IsValid;
    double          a0;
    double          a1;
    double          a2;
    double          a3;
    double          b0;
    double          b1;
    double          b2;
    double          b3;
    unsigned int    utc_wn; //此处unsiged long 为8个字节 与所给的标准4j字节不符，改为int
    unsigned int    tot;
    double          A0;
    double          A1;
    unsigned int    wn_lsf;
    unsigned int    dn;
    int             deltat_ls;
    int             deltat_lsf;
    unsigned int    deltat_utc;
};

/*****************
 *@brief SATOBS 一颗卫星的观测值信息
 *****************/
struct SATOBS
{
    unsigned short   PRN;
    enum GNSSSYS     SatSys;
    unsigned short   glofreq;
    double           psr;        ///< 伪距测量值
    double           adr;        ///< 载波相位（单位：m）
    float            dopp;       ///< 瞬时多普勒频率
    float            psr_std;    ///< 伪距测量值标准差
    float            adr_std;    ///< 载波相位标准差
    float            C_No;       ///< 载噪比
    float            locktime;
    unsigned int     ch_tr_status;
};

/*****************
 *@brief PSRPOS 接收机定位结果
 *****************/
struct PSRPOS
{
    double          lat;        ///< 纬度
    double          lon;        ///< 经度
    double          hgt;        ///< 大地高
    float           undulation;
    float           lat_sigma;  ///< 纬度标准差
    float           lon_sigma;
    float           hgt_sigma;
    char            stn_id[4];
    float           sol_age;
    unsigned char   SVs;
    unsigned char   solnSVs;
};

/*****************
 *@brief EPOCHOBS 一个历元卫星的观测值信息
 *****************/
struct EPOCHOBS
{
    struct   GPSTIME Time;
    unsigned int     NumOfObs;
    struct   SATOBS  SatObs[MAXCHANNUM];
    struct   PSRPOS  RcvPos;
    double   sig;    ///< 验后单位权方差
    double   dop;    ///< dopp值的开根号的和
};



/*************************************************
 *@brief DecodeNovTelBinDat  从Novtel OEMStar接收机二进制数据文件读取数据，解码得到观测值，星历，
 定位结果等信息
 
 *@param  fp       I      二进制数据文件指针
 *@param  Obs      O      观测数据
 *@param  Ion      O      电离层参数
 *@param  Eph      O      星历数据，PRN-1来索引
 *@param  Psrpos   O      接收机输出d的位置信息
 
 @retval 0 文件结束
 @retval 1 观测值
 @retval 2 其他数据
 @retval 3 接收机定位数据
 **************************************************/
int DecodeNovTelBinDat(unsigned char * Buff, struct EPOCHOBS* Obs, struct IONUTC* Ion, struct GPSEPH Eph[],struct PSRPOS *Psrpos);




/************************************
 *@brief  DecodeOBS  解码卫星观测值信息
 
 *@param  Buff    I  缓存区指针
 *@param  MsgLen  I  星历的字节数
 *@param  Obs     O  观测值指针
 ************************************/
void DecodeOBS(const unsigned char *Buff,unsigned int MsgLen,struct EPOCHOBS* Obs);






/***********************************************
 *@brief  DecodeGPSEphem 解码卫星的星历信息。
 
 *@param  Buff    I  缓存区指针
 *@param  MsgLen  I  星历的字节数
 *@param  Eph     O  GPS星历数组的指针
 
 *@note 卫星号PRN在Buff中获得，并给Eph数组的第PRN号卫星的星历赋本次读取的结果
 ***********************************************/
void DecodeGPSEphem(const unsigned char *Buff,unsigned int MsgLen,struct GPSEPH Eph[]);






/*********************************************
 *@brief  DecodeIONUTC 解码电离层信息
 
 *@param  Buff   I 缓存区指针
 *@param  MsgLen I 电离层的字节数
 *@param  Ion    O 电离层信息
 
 *********************************************/
void DecodeIONUTC(const unsigned char *Buff,unsigned int MsgLen,struct IONUTC* Ion);





/**********************************************
 *@brief  DecodePSRPOS 解码接收机解算的定位结果
 
 *@param  Buff   I 缓存区指针
 *@param  MsgLen I 定位结果的字节数
 *@param  Psrpos O 接收机解算的位置信息
 **********************************************/
void DecodePSRPOS(const unsigned char *Buff,unsigned int MsgLen,struct PSRPOS* Psrpos);





/**********************************************
 *@brief  crc32   crc校验
 
 *@param  buff   I 缓存区指针
 *@param  len    I 星历的字节数
 
 *@return crc计算结果
 **********************************************/
unsigned int crc32(const unsigned char *buff, int len);





unsigned short U2(const unsigned char *p);
unsigned int   U4(const unsigned char *p);
float  R4(const unsigned char *p);
double R8(const unsigned char *p);


#endif /* ReadData_h */
