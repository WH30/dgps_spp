//
//  Cal_Sat_Pos.c
//  spp
//
//  Created by learning_wh on 2019/4/10.
//  Copyright © 2019 learning_wh. All rights reserved.
//

#include "Cal_Sat_Pos.h"

extern const double PI = 3.1415926535898;  //
extern const double C = 299792458.0;       //光速
extern const double u = 398600441500000.0;  //GM
extern const double Wmiga_e = 0.000072921151467; //地球自转角速度 rad/s
extern const double F = -0.0000000004442807633;  //相对论改正常量
 

/**********************************************
 *@brief  Cal_Sat_Pos      计算所有卫星位置/速度

 *@param  eph          I   GPS星历数据,数组
 *@param  gpstime      I   信号发射时的GPS时,数组
 *@param  xyz          O   卫星的空间直角坐标,数组
 *@param  vel          O   卫星的空间直角速度,数组
 **********************************************/
void Cal_Sat_Pos(struct GPSEPH *eph, struct GPSTIME gpstime[], struct XYZ  xyz[], struct XYZ vel[])
{
	///卫星坐标解算
	for (int i = 0; i < 32; i++)
	{
		if (eph[i].RRN != i + 1)  continue;

		double e = eph[i].e;  ///< 偏心率
		double A = eph[i].A;
		double A3 = A * A*A;
		double n0 = sqrt(u*1.0 / A3);
		double tk = gpstime[i].SecOfWeek - eph[i].toe + (gpstime[i].Week - eph[i].week) * 604800;
		if (tk > 3600)
		{
			eph[i].RRN = 0;
			xyz[i].x = 0;
			vel[i].x = 0;
			continue;
		}
		double n = n0 + eph[i].delta_n;
		double Mk = eph[i].M0 + n * tk;
		double Ek = Mk;
		double last_Ek = 0;

		int looptime = 10;
		while (fabs(Ek - last_Ek) > 0.000000000001)
		{
			if (looptime <= 0) break;
			last_Ek = Ek;
			Ek = Mk + e * sin(Ek);
			looptime--;
		}
		double vk = atan2(sqrt(1 - e * e) *sin(Ek), (cos(Ek) - e));
		double Faik = vk + eph[i].wmiga;
		double deta_uk = eph[i].Cus * sin(2 * Faik) + eph[i].Cuc * cos(2 * Faik);
		double deta_rk = eph[i].Crs * sin(2 * Faik) + eph[i].Crc * cos(2 * Faik);
		double deta_ik = eph[i].Cis * sin(2 * Faik) + eph[i].Cic * cos(2 * Faik);
		double uk = Faik + deta_uk;
		double rk = A * (1 - e * cos(Ek)) + deta_rk;
		double ik = eph[i].I0 + deta_ik + (eph[i].I_DOT) * tk;
		double _xk = rk * cos(uk);
		double _yk = rk * sin(uk);
		double Wmigak = eph[i].wmiga0 + (eph[i].wmiga_DOT - Wmiga_e)*tk - Wmiga_e * eph[i].toe;
		double xk = _xk * cos(Wmigak) - _yk * cos(ik)*sin(Wmigak);
		double yk = _xk * sin(Wmigak) + _yk * cos(ik)*cos(Wmigak);   ///< e1.公式打错！
		double zk = _yk * sin(ik);

		xyz[i].x = xk;
		xyz[i].y = yk;
		xyz[i].z = zk;

		//   printf("PRN: %d  x:%f  y:%f  z:%f\n",i+1,xk,yk,zk);


		   ///钟差改正
		double deta_tr = F * e * sqrt(A) * sin(Ek);
		double deta_toc = gpstime[i].SecOfWeek - eph[i].toe + (gpstime[i].Week - eph[i].week) * 604800;
		// * deta_t_svl1=gpsephem->af0 + gpsephem->af1*deta_toc + gpsephem->af2*deta_toc*deta_toc + deta_tr + gpsephem->TGD;
		//???


		///卫星运动速度计算

		//double d_Ek=eph[i].N*1.0/(1-e*cos(Ek));
		double d_Ek = n * 1.0 / (1 - e * cos(Ek));
		double d_Faik = sqrt((1 + e)*1.0 / (1 - e)) * cos(vk*0.5) * cos(vk*0.5) / cos(Ek*0.5) / cos(Ek*0.5) *d_Ek;
		double d_uk = 2 * (eph[i].Cus*cos(2 * Faik) - eph[i].Cuc*sin(2 * Faik)) *d_Faik + d_Faik;
		double d_rk = A * e*sin(Ek)*d_Ek + 2 * (eph[i].Crs*cos(2 * Faik) - eph[i].Crc*sin(2 * Faik)) *d_Faik;
		double d_Ik = eph[i].I_DOT + 2 * (eph[i].Cis *cos(2 * Faik) - eph[i].Cic*sin(2 * Faik))*d_Faik;
		double d_Wmigak = eph[i].wmiga_DOT - Wmiga_e;


		struct Matrix d_R;
		d_R.Row = 3; d_R.Col = 4;
		d_R.Val[0] = cos(Wmigak);
		d_R.Val[1] = -sin(Wmigak)*cos(ik);
		d_R.Val[2] = -(_xk * sin(Wmigak) + _yk * cos(Wmigak) * cos(ik));
		d_R.Val[3] = _yk * sin(Wmigak)*sin(ik);

		d_R.Val[4] = sin(Wmigak);
		d_R.Val[5] = cos(Wmigak) * cos(ik);
		d_R.Val[6] = _xk * cos(Wmigak) - _yk * sin(Wmigak)*cos(ik);
		d_R.Val[7] = _yk * cos(Wmigak)*sin(ik);

		d_R.Val[8] = 0;
		d_R.Val[9] = sin(ik);
		d_R.Val[10] = 0;
		d_R.Val[11] = _yk * cos(ik);

		struct Matrix d_xyz1;  // [d_xk' ;  d_yk' ; d_wmigak' ; Ik' ]
		d_xyz1.Row = 4; d_xyz1.Col = 1;
		//??
		d_xyz1.Val[0] = d_rk * cos(uk) - rk * d_uk *sin(uk);
		d_xyz1.Val[1] = d_rk * sin(uk) + rk * d_uk *cos(uk);
		d_xyz1.Val[2] = d_Wmigak;
		d_xyz1.Val[3] = d_Ik;

		struct Matrix d_xyz;
		d_xyz.Row = 3;  d_xyz.Col = 1;
		Matrix_Mutiply(&d_R, &d_xyz1, &d_xyz);

		vel[i].x = d_xyz.Val[0];
		vel[i].y = d_xyz.Val[1];
		vel[i].z = d_xyz.Val[2];


		///卫星钟速计算
	 //   double d_deta_t_svl1 = eph[i].af1 + 2*eph[i].af2 * deta_toc;
	 //   double d_deta_tr = F*e*sqrt(A)*cos(Ek)*d_Ek;
	}


}



/**********************************************
 *@brief  Cal_Clock_Bia    计算单颗卫星的钟差/钟速

 *@param  gpsephem     I   GPS星历数据,数组
 *@param  gpstime      I   信号发射时的GPS时
 *@param  deta_t_svl1  O   卫星钟差
 *@param  d_clock_bia  O   卫星钟速
 **********************************************/
void Cal_Clock_Bia(struct GPSEPH * gpsephem, struct GPSTIME *gpstime, double* deta_t_svl1, double* d_clock_bia)
{
	///钟差改正
	double e = gpsephem->e;  ///< 偏心率
	double A = gpsephem->A;
	double A3 = A * A*A;
	double n0 = sqrt(u*1.0 / A3);
	double tk = gpstime->SecOfWeek - gpsephem->toe + (gpstime->Week - gpsephem->week) * 604800;
	double n = n0 + gpsephem->delta_n;
	double Mk = gpsephem->M0 + n * tk;
	double Ek = Mk;
	double last_Ek = 0;
	int looptime = 10;
	while (fabs(Ek - last_Ek) > 0.000000000001)
	{
		if (looptime <= 0) break;
		last_Ek = Ek;
		Ek = Mk + e * sin(Ek);
		looptime--;
	}
	double deta_tr = F * e * sqrt(A) * sin(Ek);
	double deta_toc = gpstime->SecOfWeek - gpsephem->toc + (gpstime->Week - gpsephem->week) * 604800;
	*deta_t_svl1 = gpsephem->af0 + gpsephem->af1*deta_toc + gpsephem->af2*deta_toc*deta_toc + deta_tr - gpsephem->TGD;

	///卫星钟速计算
	// double d_Ek=gpsephem->delta_n*1.0/(1-e*cos(Ek));
	double d_Ek = n * 1.0 / (1 - e * cos(Ek));
	double d_deta_t = gpsephem->af1 + 2 * gpsephem->af2 * deta_toc;
	double d_deta_tr = F * e*sqrt(A)*cos(Ek)*d_Ek;
	*d_clock_bia = d_deta_t + d_deta_tr;
}



/**********************************************
 *@brief  Cal_Sat_Pos    计算卫星位置和卫星钟差

 *@param  gpsephem   I   GPS星历数据
 *@param  gpstime    I   接收到信号时的GPS时
 *@param  v          O   卫星在空间直角坐标系中的三轴速度
 **********************************************/
void Cal_Sat_Vec(struct GPSEPH * gpsephem, struct GPSTIME* gpstime, struct Matrix *v)
{
	double e = gpsephem->e;  ///< 偏心率
	double A = gpsephem->A;
	double A3 = A * A*A;
	double n0 = sqrt(u*1.0 / A3);
	double tk = gpstime->SecOfWeek - gpsephem->toe + (gpstime->Week - gpsephem->week) * 604800;
	double n = n0 + gpsephem->delta_n;
	double Mk = gpsephem->M0 + n * tk;
	double Ek = Mk;
	double last_Ek = 0;

	int looptime = 10;
	while (fabs(Ek - last_Ek) > 0.000000000001)
	{
		if (looptime <= 0) break;
		last_Ek = Ek;
		Ek = Mk + e * sin(Ek);
		looptime--;
	}
	double vk = atan2(sqrt(1 - e * e) *sin(Ek), (cos(Ek) - e));
	double Faik = vk + gpsephem->wmiga;
	double deta_uk = gpsephem->Cus * sin(2 * Faik) + gpsephem->Cuc * cos(2 * Faik);
	double deta_rk = gpsephem->Crs * sin(2 * Faik) + gpsephem->Crc * cos(2 * Faik);
	double deta_ik = gpsephem->Cis * sin(2 * Faik) + gpsephem->Cic * cos(2 * Faik);
	double uk = Faik + deta_uk;
	double rk = A * (1 - e * cos(Ek)) + deta_rk;
	double ik = gpsephem->I0 + deta_ik + (gpsephem->I_DOT) * tk;
	double _xk = rk * cos(uk);
	double _yk = rk * sin(uk);
	double Wmigak = gpsephem->wmiga0 + (gpsephem->wmiga_DOT - Wmiga_e)*tk - Wmiga_e * gpsephem->toe;
	double xk = _xk * cos(Wmigak) - _yk * cos(ik)*sin(Wmigak);
	double yk = _xk * sin(Wmigak) - _yk * cos(ik)*cos(Wmigak);
	double zk = _yk * sin(ik);

	///卫星运动速度计算
	double d_Ek = gpsephem->N*1.0 / (1 - e * cos(Ek));
	double d_Faik = sqrt((1 + e)*1.0 / (1 - e)) * cos(vk*0.5) * cos(vk*0.5) / cos(Ek*0.5) / cos(Ek*0.5) *d_Ek;
	double d_uk = 2 * (gpsephem->Cus*cos(2 * Faik) - gpsephem->Cuc*sin(2 * Faik)) *d_Faik + d_Faik;
	double d_rk = A * e*sin(Ek)*d_Ek + 2 * (gpsephem->Crs*cos(2 * Faik) - gpsephem->Crc*sin(2 * Faik)) *d_Faik;
	double d_Ik = gpsephem->I_DOT + 2 * (gpsephem->Cis *cos(2 * Faik) - gpsephem->Cic*sin(2 * Faik))*d_Faik;
	double d_Wmigak = gpsephem->wmiga_DOT - Wmiga_e;

	struct Matrix d_R;
	d_R.Row = 3; d_R.Col = 4;
	d_R.Val[0] = cos(Wmigak);
	d_R.Val[1] = -sin(Wmigak)*cos(ik);
	d_R.Val[2] = -(_xk * sin(Wmigak) + _yk * cos(Wmigak) * cos(ik));
	d_R.Val[3] = _yk * sin(Wmigak)*sin(ik);

	d_R.Val[4] = sin(Wmigak);
	d_R.Val[5] = cos(Wmigak) * cos(ik);
	d_R.Val[6] = _xk * cos(Wmigak) - _yk * sin(Wmigak)*cos(ik);
	d_R.Val[7] = _yk * cos(Wmigak)*sin(ik);

	d_R.Val[8] = 0;
	d_R.Val[9] = sin(ik);
	d_R.Val[10] = 0;
	d_R.Val[11] = _yk * cos(ik);

	struct Matrix d_xyz1;  ///< [d_xk' ;  d_yk' ; d_wmigak' ; Ik' ]
	d_xyz1.Row = 4; d_xyz1.Col = 1;
	//??
	d_xyz1.Val[0] = d_rk * cos(uk) - rk * d_uk *sin(uk);
	d_xyz1.Val[1] = d_rk * sin(uk) + rk * d_uk *cos(uk);
	d_xyz1.Val[2] = d_Wmigak;
	d_xyz1.Val[3] = d_Ik;

	struct Matrix d_xyz;
	d_xyz.Row = 3;  d_xyz.Col = 1;
	Matrix_Mutiply(&d_R, &d_xyz1, &d_xyz);
	v->Val[0] = d_xyz.Val[0];
	v->Val[1] = d_xyz.Val[1];
	v->Val[2] = d_xyz.Val[2];
}



