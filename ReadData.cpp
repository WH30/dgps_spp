//
//  ReadData.c
//  ReadData
//
//  Created by learning_wh on 2019/4/3.
//  Copyright © 2019 learning_wh. All rights reserved.
//

#include "ReadData.h"
#include <stdio.h>
#include <stdlib.h>
#define MAXNOVLEN 2300
#define POLYCRC32   0xEDB88320u

 
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
int DecodeNovTelBinDat(unsigned char * Buff, struct EPOCHOBS* Obs, struct IONUTC* Ion, struct GPSEPH Eph[],struct PSRPOS *Psrpos)
{
    int Val;
    unsigned short MsgID,MsgLen;
    int week;
    //?
    unsigned int sec;
    //unsigned char Buff[MAXNOVLEN]={'\0'};
    
    ///找同步字
	/*
    while (!feof(fp))
    {
        if(fread(Buff+2, 1, 1, fp)!=1)  return 0;
        if(Buff[0]==0xAA && Buff[1]==0x44 && Buff[2]==0x12)  break;
        else{
            Buff[0]=Buff[1];
            Buff[1]=Buff[2];
        }
    };
	*/
    
    ///读25个字节
   // if(fread(Buff+3, 1, 25, fp)!=25)  return 0;
    

    ///解信息头，得到MsgID，MsgLen
    MsgID = U2(Buff+4);
    MsgLen= U2(Buff+8);
    week  = U2(Buff+14);
    sec   =U4(Buff+16);
    
    
    ///读MsgLen+4字节，组成Msg
   // if(MAXNOVLEN<(MsgLen+32))   return 2;
    //if(fread(Buff+28, 1, MsgLen+4, fp)!=(MsgLen+4))  return 0;
    
    ///CRC校验
	/*
    if(crc32(Buff, MsgLen+28)!=U4(Buff+28+MsgLen) )
    {
        printf("CRC fail\n");
        return 2;
    }
      */

    ///根据MsgID，调用解码子程序
    Val=2;
    ///、:  43:观测值OBS  7:GPS星历   8:电离层    47:接收机定位结果
    switch (MsgID)
    {
        case 43:
            Val = 1;
            Obs->Time.Week=week;
            Obs->Time.SecOfWeek=sec/1000;      ///< 接受机的表面时
            DecodeOBS(Buff+28,MsgLen,Obs);
            break;
        case 7:
            DecodeGPSEphem(Buff+28,MsgLen,Eph);
            break;
        case 8:
            DecodeIONUTC(Buff+28,MsgLen,Ion);
            break;
        case 47:
            Val = 3;
            Obs->Time.Week=week;
            Obs->Time.SecOfWeek=sec/1000;      ///< 接受机的表面时
            DecodePSRPOS(Buff+28,MsgLen,Psrpos);
            break;
        default:
            break;
    }
    /*
    if(Val==3)
    {
        FILE *fpp;
        fpp=fopen("/Users/learning_wh/Desktop/log2ref.txt","a");
        struct BLH blh;
        struct XYZ xyz;
        blh.B=Psrpos->lat;
        blh.L=Psrpos->lon;
        blh.H=Psrpos->hgt;
        BLHToXYZ(&blh, &xyz);
        fpp=fopen("/Users/learning_wh/Desktop/log2ref.txt","a");
        fprintf(fpp," %d  %d  %f  %f  %f   %f  %f  %f\n",week,sec,xyz.x,xyz.y,xyz.z,blh.B/PI*180,blh.L/PI*180,blh.H);
        fclose( fpp );
    }
     */
    return Val;
}


/************************************
*@brief  DecodeOBS  解码卫星观测值信息

*@param  Buff    I  缓存区指针
*@param  MsgLen  I  星历的字节数
*@param  Obs     O  观测值指针

*@note
************************************/
void DecodeOBS(const unsigned char *Buff,unsigned int MsgLen,struct EPOCHOBS* Obs)
{
    for(int i=0;i<32;i++)
        Obs->SatObs[i].PRN=0;
    Obs->NumOfObs=U4(Buff);
    for(int i=0;i<Obs->NumOfObs;i++)
    {
        unsigned short PRN=U2(Buff+i*44+4);
        Obs->SatObs[PRN-1].PRN=PRN;
        Obs->SatObs[PRN-1].psr=R8(Buff+i*44+8);
        Obs->SatObs[PRN-1].psr_std=R4(Buff+i*44+16);
        Obs->SatObs[PRN-1].adr=R8(Buff+i*44+20);
        Obs->SatObs[PRN-1].adr_std=R4(Buff+i*44+28);
        Obs->SatObs[PRN-1].dopp=R4(Buff+i*44+32);
        Obs->SatObs[PRN-1].C_No=R4(Buff+i*44+36);
        Obs->SatObs[PRN-1].locktime=R4(Buff+i*44+40);
        Obs->SatObs[PRN-1].ch_tr_status=U4(Buff+i*44+44);
      //  printf("RANGE MESSAGE : PRN %d PSR %f \n" , Obs->SatObs[PRN-1].PRN,Obs->SatObs[PRN-1].psr);
    }
}



/************************************
*@brief  DecodeGPSEphem 解码卫星的星历信息。
 
*@param  Buff    I  缓存区指针
*@param  MsgLen  I  星历的字节数
*@param  Eph     O  GPS星历数组的指针
 
*@note 卫星号PRN在Buff中获得，并给Eph数组的第PRN号卫星的星历赋本次读取的结果
************************************/
void DecodeGPSEphem(const unsigned char *Buff,unsigned int MsgLen,struct GPSEPH Eph[])
{
    ///记录到星历数组中对应的PRN
    unsigned int k=U4(Buff);
    if(k<=0||k>32) return ;///< PRN号不在1-32之间，返回
    Eph[k-1].RRN=U4(Buff);
    Eph[k-1].tow=R8(Buff+4);
    Eph[k-1].HEALTH=U4(Buff+12);
    Eph[k-1].IODE1=U4(Buff+16);
    Eph[k-1].IODE2=U4(Buff+20);
    Eph[k-1].week=U4(Buff+24);
    Eph[k-1].z_week=U4(Buff+28);
    Eph[k-1].toe=R8(Buff+32);
    //toc  toe
    Eph[k-1].A=R8(Buff+40);
    Eph[k-1].delta_n=R8(Buff+48);
    Eph[k-1].M0=R8(Buff+56);
    Eph[k-1].e=R8(Buff+64);
    Eph[k-1].wmiga=R8(Buff+72);
    Eph[k-1].Cuc=R8(Buff+80);
    Eph[k-1].Cus=R8(Buff+88);
    Eph[k-1].Crc=R8(Buff+96);
    Eph[k-1].Crs=R8(Buff+104);
    Eph[k-1].Cic=R8(Buff+112);
    Eph[k-1].Cis=R8(Buff+120);
    Eph[k-1].I0=R8(Buff+128);
    Eph[k-1].I_DOT=R8(Buff+136);
    Eph[k-1].wmiga0=R8(Buff+144);
    Eph[k-1].wmiga_DOT=R8(Buff+152);
    Eph[k-1].IODC=U4(Buff+160);
    Eph[k-1].toc=R8(Buff+164);
    //struct GPSTIME g;
    //????
    Eph[k-1].TGD=R8(Buff+172);
    Eph[k-1].af0=R8(Buff+180);
    Eph[k-1].af1=R8(Buff+188);
    Eph[k-1].af2=R8(Buff+196);
    Eph[k-1].AS=U4(Buff+204);
    Eph[k-1].N=R8(Buff+208);
    Eph[k-1].URA=R8(Buff+216);
  //  printf("GPSEPHEM MESSAGE : PRN %d Time %f %f \n" , Eph[k-1].RRN, Eph[k-1].week,Eph[k-1].tow);
}




/************************************
 *@brief  DecodeIONUTC 解码电离层信息
 
 *@param  Buff   I  缓存区指针
 *@param  MsgLen I  电离层的字节数
 *@param  Ionutc    O  电离层信息
 
 *@note 未初始化时IsValid=0  初始化后IsValid=1
 ************************************/
void DecodeIONUTC(const unsigned char *Buff,unsigned int MsgLen,struct IONUTC* Ionutc)
{
    Ionutc->IsValid=1;
    Ionutc->a0=R8(Buff);
    Ionutc->a1=R8(Buff+8);
    Ionutc->a1=R8(Buff+16);
    Ionutc->a3=R8(Buff+24);
    Ionutc->b0=R8(Buff+32);
    Ionutc->b1=R8(Buff+40);
    Ionutc->b2=R8(Buff+48);
    Ionutc->b3=R8(Buff+56);
    Ionutc->utc_wn=*(Buff+64);
    Ionutc->tot=*(Buff+68);
    Ionutc->A0=R8(Buff+72);
    Ionutc->A1=R8(Buff+80);
    Ionutc->wn_lsf=*(Buff+88);
    Ionutc->dn=*(Buff+92);
    Ionutc->deltat_ls=*(Buff+96);
    Ionutc->deltat_lsf=*(Buff+100);
    Ionutc->deltat_utc=*(Buff+104);
}




/************************************
 *@brief  DecodePSRPOS 解码接收机解算的定位结果
 
 *@param  Buff   I 缓存区指针
 *@param  MsgLen I 定位结果的字节数
 *@param  Psrpos O 接收机解算的位置信息
 ************************************/
void DecodePSRPOS(const unsigned char *Buff,unsigned int MsgLen,struct PSRPOS* Psrpos)
{
    Psrpos->lat=R8(Buff+8);
    Psrpos->lon=R8(Buff+16);
    Psrpos->hgt=R8(Buff+24);
    Psrpos->undulation=R4(Buff+32);
    //psrpos->datum_id=*(buff+36);
    Psrpos->lat_sigma=R4(Buff+40);
    Psrpos->lon_sigma=R4(Buff+44);
    Psrpos->hgt_sigma=R4(Buff+48);
  //  printf("PSRPOS MESSAGE : lat %f lon %f hgt%f \n" , Psrpos->lat, Psrpos->lon,Psrpos->hgt);
    
    
}


/**********************************************
 *@brief  crc32   crc校验
 
 *@param  buff  I  缓存区指针
 *@param  len   I  星历的字节数
 
 *@return crc计算结果
 **********************************************/
unsigned int crc32(const unsigned char *buff, int len)
{
    int i,j;
    unsigned int crc=0;
    for (i=0;i<len;i++)
    {
        crc^=buff[i];
        for (j=0;j<8;j++)
        {
            if (crc&1) crc=(crc>>1)^POLYCRC32;
            else crc>>=1;
        }
    }
    return crc;
}








unsigned short U2(const unsigned char *p)
{
    unsigned short r;
    memcpy(&r, p, 2);
    return r;
}

unsigned int U4(const unsigned char *p)
{
    unsigned int r;
    memcpy(&r, p, 4);
    return r;
}

double R8(const unsigned char *p)
{
    double r;
    memcpy(&r, p, 8);
    return r;
}

float R4(const unsigned char *p)
{
    float r;
    memcpy(&r, p, 4);
    return r;
}









