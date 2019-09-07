//
//  Time.c
//  spp
//
//  Created by learning_wh on 2019/4/12.
//  Copyright © 2019 learning_wh. All rights reserved.
#include <stdio.h>
#include "Time.h"
extern const long    _MINUTE_IN_SECOND = 60L;
extern const long    _HOUR_IN_MINUTE = 60L;
extern const long    _DAY_IN_HOUR = 24L;
extern const long    _WEEK_IN_DAY = 7L;
extern const long    _HOUR_IN_SECOND = 1 * 60 * 60;
extern const long    _DAY_IN_SECOND = 24 * 3600;
extern const long    _WEEK_IN_SECOND = 7 * 24*3600;


/**********************************************
 *@brief  CommonTimeToMjdTime   通用时 转 简化儒略日
 
 *@param  commontime   I   通用时
 *@param  mjdtime      O   简化儒略日
 **********************************************/
void CommonTimeToMjdTime(struct COMMONTIME *commontime,struct MJDTIME * mjdtime )
{
    commontime->FracSecond=commontime->Second-(int)(commontime->Second);
    unsigned short Y=commontime->Year;
    unsigned short M=commontime->Month;
    unsigned short D=commontime->Day;
    double Min=commontime->Minute;
    Min=Min*1.0/60;
    double Sec=commontime->Second;
    Sec=Sec*1.0/60/60;
    double UT=commontime->Hour+Min+Sec;  ///< 单位为天
    unsigned short m=0,y=0;
    if(M<=2) {y=Y-1; m=M+12;}
    else if(M>2) {y=Y;m=M;}
    double MJD=0;                       ///< 整数天加小数天
    MJD=(long)(365.25*y)+(long)(30.6001*(m+1))+D+UT*1.0/24-679019;
    mjdtime->Days=(int)MJD;          ///< 整数部分
    mjdtime->FracDay=MJD-mjdtime->Days;   ///< 小数部分  单位为（天）
}

/**********************************************
 *@brief  MjdTimeToCommonTime   简化儒略日 转 通用时
 
 *@param  mjdtime      I   简化儒略日
 *@param  commontime   O   通用时
 **********************************************/
void MjdTimeToCommonTime(const struct MJDTIME* mjdtime,struct COMMONTIME *commontime)
{
    double JD =mjdtime->Days+mjdtime->FracDay*1.0 + 2400000.5;
    long a=(int)(JD+0.5);
    long b=a+1537;
    long c=(int)((b-122.1)*1.0/365.25);
    long d=(int)(365.25*c);
    long e=(int)((b-d)*1.0/30.6001);
    double DAY=b-d-(long)(30.6001*e*1.0) + JD+0.5-a;  /// <日 带小数的
    
    unsigned short Month=e-1-12*(int)(e*1.0/14);  //月 短整形
    unsigned short Year=c-4715-(int)((7+Month)*1.0/10);  //年 短整形
    unsigned short Day=(int) DAY;
    // unsigned short N=a%7;
    double Frac_DAY=DAY-(int)(Day);
    unsigned short Hour= (int) (Frac_DAY*24.0);
    unsigned short Minute=(int) ( (Frac_DAY*24.0-Hour)*60.0 )  ;
    double Second =0.0;
    //Second = Frac_DAY*24.0*60*60 - int(Frac_DAY*24.0*60*60);
    Second = (int)(Frac_DAY*24*60*60 - Hour*60*60 -Minute*60)+commontime->FracSecond;   //
    //  时分秒怎么计算？？？？
    commontime->Year=Year;
    commontime->Month=Month;
    commontime->Day=Day;
    commontime->Hour=Hour;
    commontime->Minute=Minute;
    commontime->Second=Second;
}

/**********************************************
 *@brief  GPSTimeToMjdTime   GPS时 转 简化儒略日
 
 *@param  gpstime      I   GPS时
 *@param  mjdtime      O   简化儒略日
 **********************************************/
void GPSTimeToMjdTime(const struct GPSTIME* gpstime,struct MJDTIME *mjdtime)
{
    double MJD=44244 + gpstime->Week * 7 + gpstime->SecOfWeek *1.0 / 86400; // 整数加小数
    mjdtime->Days=(int)MJD;          //整数部分
    mjdtime->FracDay=MJD-mjdtime->Days;   //小数部分  单位为（天）
}


/**********************************************
 *@brief  MjdTimeToGPSTime   简化儒略日 转 GPS时
 
 *@param  mjdtime      I   简化儒略日
 *@param  gpstime      O   GPS时
 **********************************************/
void MjdTimeToGPSTime(const struct MJDTIME* mjdtime,struct GPSTIME* gpstime)
{
    double MJD=mjdtime->Days+mjdtime->FracDay;
    gpstime->Week=(int)((MJD-44244) * 1.0 /7);
    gpstime->SecOfWeek=(MJD - 44244 -gpstime->Week*7) * 86400;
}

/**********************************************
 *@brief  GPSTimeToCommonTime   GPS时 转 通用时
 
 *@param  gpstime      I   GPS时
 *@param  commontime   O   通用时
 **********************************************/
void GPSTimeToCommonTime(const struct GPSTIME *gpstime,struct COMMONTIME *commontime)
{
    struct MJDTIME  mjdtime;
    GPSTimeToMjdTime(gpstime,&mjdtime);
    MjdTimeToCommonTime(&mjdtime,commontime);
}

/**********************************************
 *@brief  CommonTimeToGPSTime   通用时 转 GPS时
 
 *@param  commontime   I   通用时
 *@param  gpstime      O   GPS时
 **********************************************/
void CommonTimeToGPSTime(struct COMMONTIME* commontime,struct GPSTIME *gpstime)
{
    struct MJDTIME mjdtime;
    CommonTimeToMjdTime(commontime,&mjdtime);
    MjdTimeToGPSTime(&mjdtime,gpstime);
}
