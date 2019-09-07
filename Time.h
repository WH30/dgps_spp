
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
 *@brief  COMMONTIME  ͨ��ʱ
 **************************/
struct COMMONTIME
{
    unsigned short Year;    ///< ������ ʱ����
    unsigned short Month;
    unsigned short Day;
    unsigned short Hour;
    unsigned short Minute;
    double Second;
    double FracSecond;      ///< ��¼���С������,���⾫�ȶ�ʧ
};
/**************************
 *@brief  MJDTIME ��������
 **************************/
struct MJDTIME
{
    int Days;
    double FracDay;
};
/**************************
 *@brief  GPSTIME GPSʱ
 **************************/
struct GPSTIME
{
    int Week;
    double SecOfWeek;
};



/**********************************************
 *@brief  CommonTimeToMjdTime   ͨ��ʱ ת ��������
 *@param  commontime   I   ͨ��ʱ
 *@param  mjdtime      O   ��������
 **********************************************/
void CommonTimeToMjdTime(struct COMMONTIME *commontime,struct MJDTIME * mjdtime );

/**********************************************
 *@brief  MjdTimeToCommonTime   �������� ת ͨ��ʱ
 *@param  mjdtime      I   ��������
 *@param  commontime   O   ͨ��ʱ
 **********************************************/
void MjdTimeToCommonTime(const struct MJDTIME* mjdtime,struct COMMONTIME *commontime);


/**********************************************
 *@brief  GPSTimeToMjdTime   GPSʱ ת ��������
 *@param  gpstime      I   GPSʱ
 *@param  mjdtime      O   ��������
 **********************************************/
void GPSTimeToMjdTime(const struct GPSTIME* gpstime,struct MJDTIME *mjdtime);

/**********************************************
 *@brief  MjdTimeToGPSTime   �������� ת GPSʱ
 *@param  mjdtime      I   ��������
 *@param  gpstime      O   GPSʱ
 **********************************************/
void MjdTimeToGPSTime(const struct MJDTIME* mjdtime,struct GPSTIME* gpstime);


/**********************************************
 *@brief  GPSTimeToCommonTime   GPSʱ ת ͨ��ʱ
 *@param  gpstime      I   GPSʱ
 *@param  commontime   O   ͨ��ʱ
 **********************************************/
void GPSTimeToCommonTime(const struct GPSTIME *gpstime,struct COMMONTIME *commontime);

/**********************************************
 *@brief  CommonTimeToGPSTime   ͨ��ʱ ת GPSʱ
 *@param  commontime   I   ͨ��ʱ
 *@param  gpstime      O   GPSʱ
 **********************************************/
void CommonTimeToGPSTime(struct COMMONTIME* commontime,struct GPSTIME *gpstime);



#endif /* Time_h */
