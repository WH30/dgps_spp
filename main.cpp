//
//  main.c
//  spp_1.0
//
//  Created by learning_wh on 2019/4/17.
//  Copyright © 2019 learning_wh. All rights reserved.
//
 
#include <stdio.h>
#include <stdlib.h>
#include<Windows.h>
#include "ReadData.h"
#include "Time.h"
#include "spp.h" 
#include"Serial.h"
#include"sockets.h"
#include"Dgps.h"

#define BUFFSIZE 20480
#define SerialPort 10
#define BaudRate 115200

struct GPSEPH GpsEph[32];       ///< 卫星星历
struct GPSTIME Trans_Time[32];  ///< 卫星信号发射的GPS时
double Deta_T_tr[32];           ///< 卫星发射信号的钟差
double D_clock_bia[32];         ///< 卫星钟差变化率
struct IONUTC Ion;              ///< 电离层数据
struct PSRPOS Psrpos;           ///< 接收机伪距定位结果
struct EPOCHOBS EpochObs;       ///< 历元观测值
struct XYZ GPS_xyz[32];         ///< GPS卫星位置
struct XYZ GPS_vel[32];         ///< GPS卫星速度
struct XYZ REC_xyz;              ///< 接收机位置
struct BLH REC_blh;             ///< 接收机位置
struct XYZ REC_vel;              ///< 接收机速度
struct PsrCrrect Psrcre[32];  ///< 伪距改正数组
unsigned char buff[BUFFSIZE];    ///< 串口数据
char recbuff[BUFFSIZE];              ///< socket 原始数据

void ConnectNovatel(CSerial *cs);

int main(int argc, const char * argv[]) {
	FILE *fpp, *fpp2;
	fpp = fopen("spp.txt","a");
	fpp2 = fopen("dgps.txt", "a");
	CSerial *cs = new CSerial();
	ConnectNovatel(cs);
	SOCKET sock;
	OpenDGPSSocket(sock);
	int socketflag = 0;
    while(cs->IsOpened())
    {
        ///从板卡接受数据
		int datnum = cs->ReadDataWaiting();
		cs->ReadData(buff, datnum);           ///<
		socketflag = (socketflag == 1) ? 0 : 1;   ///< 两秒接收一次数据
		if (socketflag)
		{
			int revlenth = recv(sock, recbuff, BUFFSIZE, 0);   ///<  实际接收到的字节数
			printf("收到 %d 个字节 \n", revlenth);
			DecodePsrCroct(recbuff, revlenth, Psrcre);
		}		
		//  解码改变  ：  fp ->buff     由传文件的指针改为传数组的地址
		unsigned char *pbuff = buff;
		for (int i = 0; i < datnum; i++)
		{
			if (buff[i] == 0xAA && buff[i + 1] == 0x44 && buff[i + 2] == 0x12)
			{
				pbuff = buff + i;      ///<   同步之后的指针位置
				int MsgLen = U2(pbuff + 8);   ///<  信息的长度
				if (i + MsgLen + 32 > datnum)      ///< 超长  28+msglen+4，读下个历元数据
				{
					break;
				}
				if (crc32(pbuff, MsgLen + 28) != U4(pbuff + 28 + MsgLen))
				{
					printf("CRC fail\n");
					continue;
				}
				i += MsgLen + 31;
				int ret = DecodeNovTelBinDat(pbuff, &EpochObs, &Ion, GpsEph, &Psrpos);
				switch (ret) {
				case 0:///< 文件结束
					break;
				case 1:///< 观测数据
				   // break;
					Cal_T_Tr_GPS(&EpochObs, GpsEph, Trans_Time, Deta_T_tr, D_clock_bia);  ///< 计算卫星信号发射的时间和对应的钟差
					Cal_Sat_Pos(GpsEph, Trans_Time, GPS_xyz, GPS_vel);      ///< 计算卫星位置速度

					//计算spp结果  
					Cal_Rec_Pos(&EpochObs, GPS_xyz, &Ion, Deta_T_tr, &REC_xyz, Psrcre , GpsEph,0);
					XYZToBLH(&REC_xyz, &REC_blh);
					printf("spp %d  %f  %f  %f  %f   %f  %f  %f  %f\n", EpochObs.Time.Week, EpochObs.Time.SecOfWeek, REC_xyz.x, REC_xyz.y, REC_xyz.z, REC_blh.B / PI * 180, REC_blh.L / PI * 180, REC_blh.H,EpochObs.sig);
					fprintf(fpp, " %d  %f  %f  %f  %f   %f  %f  %f   %f\n", EpochObs.Time.Week, EpochObs.Time.SecOfWeek, REC_xyz.x, REC_xyz.y, REC_xyz.z, REC_blh.B / PI * 180, REC_blh.L / PI * 180, REC_blh.H, EpochObs.sig);  ///< 输出坐标

					REC_xyz.x = 0; REC_xyz.y = 0; REC_xyz.z = 0;

					///计算dgps的结果
					Cal_Rec_Pos(&EpochObs, GPS_xyz, &Ion, Deta_T_tr, &REC_xyz, Psrcre, GpsEph, 1);
					XYZToBLH(&REC_xyz, &REC_blh);
					printf("dgps %d  %f  %f  %f  %f   %f  %f  %f  %f\n \n", EpochObs.Time.Week, EpochObs.Time.SecOfWeek, REC_xyz.x, REC_xyz.y, REC_xyz.z, REC_blh.B / PI * 180, REC_blh.L / PI * 180, REC_blh.H,EpochObs.sig);
					fprintf(fpp2, " %d  %f  %f  %f  %f   %f  %f  %f   %f\n ", EpochObs.Time.Week, EpochObs.Time.SecOfWeek, REC_xyz.x, REC_xyz.y, REC_xyz.z, REC_blh.B / PI * 180, REC_blh.L / PI * 180, REC_blh.H, EpochObs.sig);  ///< 输出坐标
					break;
				case 2:///< 其他数据
					continue;
					break;
				case 3:///< 接收机定位数据PSRPOS
					///更新接收机位置，或赋初值
					REC_blh.B = Psrpos.lat / 180 * PI;
					REC_blh.L = Psrpos.lon / 180 * PI;
					REC_blh.H = Psrpos.hgt +Psrpos.undulation;
					BLHToXYZ(&REC_blh, &REC_xyz);
		    //		printf(" %d  %f  %f  %f  %f   %f  %f  %f  \n", EpochObs.Time.Week, EpochObs.Time.SecOfWeek, REC_xyz.x, REC_xyz.y, REC_xyz.z, REC_blh.B / PI * 180, REC_blh.L / PI * 180, REC_blh.H);

					break;
				default:
					break;
				}
			}
		}
		Sleep(1000);
		
    }
	cs->Close();
    return 0;
}


///向板卡发送信息
void ConnectNovatel(CSerial *cs)
{
	cs->Open(SerialPort, BaudRate);
	const char ch_rangeb[] = "log rangeb ontime 1\r\n";
	const char ch_eph[] = "log gpsephemb onchanged\r\n";
	const char ch_ion[] = "log ionutcb onchanged\r\n";
	const char ch_psr[] = "log psrposb ontime 1\r\n";
	cs->SendData(ch_rangeb, 22);
	cs->SendData(ch_eph, 26);
	cs->SendData(ch_ion, 24);
	cs->SendData(ch_psr, 23);
}
