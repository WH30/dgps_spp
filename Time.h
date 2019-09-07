
#ifndef Time_h
#define Time_h
extern const long    _MINUTE_IN_SECOND ;
extern const long    _HOUR_IN_MINUTE   ;
extern const long    _DAY_IN_HOUR      ;
extern const long    _WEEK_IN_DAY      ;
extern const long    _HOUR_IN_SECOND   ;
extern const long    _DAY_IN_SECOND    ;
extern const long    _WEEK_IN_SECOND   ;
 
/**************************
 *@brief  COMMONTIME  通用时
 **************************/
struct COMMONTIME
{
    unsigned short Year;    ///< 年月日 时分秒
    unsigned short Month;
    unsigned short Day;
    unsigned short Hour;
    unsigned short Minute;
    double Second;
    double FracSecond;      ///< 记录秒的小数部分,避免精度丢失
};
/**************************
 *@brief  MJDTIME 简化儒略日
 **************************/
struct MJDTIME
{
    int Days;
    double FracDay;
};
/**************************
 *@brief  GPSTIME GPS时
 **************************/
struct GPSTIME
{
    int Week;
    double SecOfWeek;
};



/**********************************************
 *@brief  CommonTimeToMjdTime   通用时 转 简化儒略日
 *@param  commontime   I   通用时
 *@param  mjdtime      O   简化儒略日
 **********************************************/
void CommonTimeToMjdTime(struct COMMONTIME *commontime,struct MJDTIME * mjdtime );

/**********************************************
 *@brief  MjdTimeToCommonTime   简化儒略日 转 通用时
 *@param  mjdtime      I   简化儒略日
 *@param  commontime   O   通用时
 **********************************************/
void MjdTimeToCommonTime(const struct MJDTIME* mjdtime,struct COMMONTIME *commontime);


/**********************************************
 *@brief  GPSTimeToMjdTime   GPS时 转 简化儒略日
 *@param  gpstime      I   GPS时
 *@param  mjdtime      O   简化儒略日
 **********************************************/
void GPSTimeToMjdTime(const struct GPSTIME* gpstime,struct MJDTIME *mjdtime);

/**********************************************
 *@brief  MjdTimeToGPSTime   简化儒略日 转 GPS时
 *@param  mjdtime      I   简化儒略日
 *@param  gpstime      O   GPS时
 **********************************************/
void MjdTimeToGPSTime(const struct MJDTIME* mjdtime,struct GPSTIME* gpstime);


/**********************************************
 *@brief  GPSTimeToCommonTime   GPS时 转 通用时
 *@param  gpstime      I   GPS时
 *@param  commontime   O   通用时
 **********************************************/
void GPSTimeToCommonTime(const struct GPSTIME *gpstime,struct COMMONTIME *commontime);

/**********************************************
 *@brief  CommonTimeToGPSTime   通用时 转 GPS时
 *@param  commontime   I   通用时
 *@param  gpstime      O   GPS时
 **********************************************/
void CommonTimeToGPSTime(struct COMMONTIME* commontime,struct GPSTIME *gpstime);



#endif /* Time_h */
