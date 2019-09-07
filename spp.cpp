//
//  spp.c
//  spp_1.0
//
//  Created by learning_wh on 2019/4/19.
//  Copyright © 2019 learning_wh. All rights reserved.
//

#include "spp.h"
 

#define MIN_CLOCK_BIASE 0.00000000000001
/**********************************************
 *@brief  Cal_T_tr_GPS    计算卫星发射信号的GPS时
 
 *@param  epoch_obs   I   一个历元的观测数据
 *@param  eph         I   GPS卫星星历，大小为32的数组
 *@param  t_tr_gps    O   卫星发射信号的GPS时,大小为32的数组，记录所有GPS卫星发射信号的时间，数组下标对应PRN-1
 *@param  deta_t_tr   O   GPS卫星发射信号时的钟差
 *@param  d_clock_bia O   GPS卫星发射信号时的钟速
 **********************************************/
void Cal_T_Tr_GPS(struct EPOCHOBS *epoch_obs, struct GPSEPH eph[], struct GPSTIME t_tr_gps[], double deta_t_tr[],double d_clock_bia[])
{
    /// 接收机钟的表面时在历元观测值中有记录
    struct GPSTIME T_rec_srf = epoch_obs->Time;  ///<接收机接收到信号的表面时
    //  int N=epoch_obs->NumOfObs;   ///< 观测值的数量
    /// 遍历32科卫星，因为不知道上次观测了那几颗卫星
    for(int i=0;i<32;i++)
    {
        if(epoch_obs->SatObs[i].PRN != i+1 ) continue;  ///< i科卫星观测值的编号PRN不等于i+1，则表示未观测到
        if(eph[i].RRN != i+1 ) ///< 没有星历
        {
            epoch_obs->SatObs[i].PRN =0;  ///< 不要该观测值
            epoch_obs->NumOfObs--;        ///< 卫星数减一
            continue;
        }
        double P=epoch_obs->SatObs[i].psr;  ///< 伪距观测值
        double d_clock_bia_i=0;       ///< 卫星钟速
        double deta_t_j=0;            ///< 卫星钟差改正，初始值设为0
        double last_deta_t_j=-100;    ///< 记录上一次的卫星钟差改正，用于迭代的判断
        struct GPSTIME T_tr_gps;      ///< 当前第i科卫星发射信号的GPS时
        T_tr_gps.SecOfWeek=T_rec_srf.SecOfWeek - P/C; ///< 赋初值
        T_tr_gps.Week=T_rec_srf.Week ;
        while(T_tr_gps.SecOfWeek<0)                  ///< 保证sec在0-604800之间
        {
            T_tr_gps.Week -= 1;
            T_tr_gps.SecOfWeek += 604800;
        }
        /// 迭代计算卫星钟差的改正
        int looptime=10;
        while(fabs(deta_t_j - last_deta_t_j)>MIN_CLOCK_BIASE)  ///< 钟差小于阈值时退出
        {
            if(looptime<=0) break;
            last_deta_t_j=deta_t_j;                          ///< 记录上次的钟差改正
            Cal_Clock_Bia(&eph[i], &T_tr_gps, &deta_t_j,&d_clock_bia_i);    ///< 计算本次钟差改正
         //   double deta_toc=gpstime->SecOfWeek - gpsephem->toc  +  (gpstime->Week - gpsephem->week) * 604800;
         //   *deta_t_svl1=gpsephem->af0 + gpsephem->af1*deta_toc + gpsephem->af2*deta_toc*deta_toc
            T_tr_gps.SecOfWeek=T_tr_gps.SecOfWeek  +last_deta_t_j  -deta_t_j;  ///< 更新发射信号的GPS时
            while(T_tr_gps.SecOfWeek<0)
            {
                T_tr_gps.Week -= 1;
                T_tr_gps.SecOfWeek += 604800;
            }
            looptime--;
        }
         
        /*
        while(fabs(deta_t_j - last_deta_t_j)>MIN_CLOCK_BIASE)
        {
            last_deta_t_j=deta_t_j;
            double deta_toc=T_tr_gps.SecOfWeek - eph[i].toc  +  (T_tr_gps.Week - eph[i].week) * 604800;
            deta_t_j=eph[i].af0 + eph[i].af1*deta_toc + eph[i].af2*deta_toc*deta_toc;
            T_tr_gps.SecOfWeek=T_tr_gps.SecOfWeek  +last_deta_t_j  -deta_t_j;
            while(T_tr_gps.SecOfWeek<0)
            {
                T_tr_gps.Week -= 1;
                T_tr_gps.SecOfWeek += 604800;
            }
        }
        */
        
        t_tr_gps[i]=T_tr_gps;  ///< 给第i号卫星赋予发射信号的GPS时
        d_clock_bia[i]=d_clock_bia_i;
        deta_t_tr[i]=deta_t_j;
     //   printf("卫星:PRN %d发射时间的周内秒为 %f\n",i+1,T_tr_gps.SecOfWeek);
    }
    return;
}




/**********************************************
 *@brief  Cal_Rec_Pos    计算接收机的位置
 
 *@param  epoch_obs    I   一个历元的观测数据
 *@param  gps_xyz      I   GPS卫星位置,大小为32的数组
 *@param  Ion          I   电离层数据
 *@param  Deta_t_tr    I   GPS卫星钟差,大小为32的数组
 *@param  receive_xyz  O   接收机的位置
 *@param  Psrcre  I   差分数据

 **********************************************/
void Cal_Rec_Pos(struct EPOCHOBS * epoch_obs,struct XYZ gps_xyz[],struct IONUTC *Ion, double Deta_t_tr[],struct XYZ *receive_xyz, struct PsrCrrect Psrcre[],GPSEPH gpseph[],int flag)
{
    int num_obs=epoch_obs->NumOfObs;
    if(num_obs<4) return ;
    struct Matrix Y; Y.Row=num_obs; Y.Col=1;      ///< 观测值
    struct Matrix H; H.Row=num_obs; H.Col=4;      ///< H矩阵
    struct Matrix X; X.Row=4; X.Col=1;            ///< 待估坐标和钟差
    struct Matrix W; W.Row=num_obs; W.Col=num_obs;///< 观测值的权矩阵 , 对角阵
    for(int i=0;i<14*14;i++)
    {
        W.Val[i]=0;
    }
    struct Matrix N; N.Col=4;N.Row=4;          ///< N  4x4
    struct BLH rec_blh;
    XYZToBLH(receive_xyz, &rec_blh);
 //   printf("接收机坐标的计算坐标为: x: %f y:%f z :%f\n",receive_xyz->x,receive_xyz->y,receive_xyz->z);
 //   printf("接收机坐标的计算坐标为: b: %f l:%f h :%f\n",rec_blh.B/PI*180,rec_blh.L/PI*180,rec_blh.H);
    int loop_time=10; ///< 迭代最大次数
    int k=0;
    int IsEarthCov=0;
    struct XYZ last_rec_xyz;
    last_rec_xyz.x= receive_xyz->x;
    last_rec_xyz.y= receive_xyz->y;
    last_rec_xyz.z= receive_xyz->z;
  //  receive_xyz->x=0;
  //  receive_xyz->y=0;
  //  receive_xyz->z=0;
    double d_clok=0;
    double deta_wmiga=0;  ///< 自转改正
    double T_Klo=0;
    double T_Hop=0;
    double t_klo=0;
 //   for(int i=0;i<32;i++) d_clok[i]=0;
    while(loop_time>=0)
    {
        k=0;
        num_obs=epoch_obs->NumOfObs;
        
        for(int i=0;i<32;i++)
        {
            if(epoch_obs->SatObs[i].PRN != i+1 ) continue;
            if(gps_xyz[i].x==0)
            {
                num_obs--;
                continue;
            }
            /*
            if(epoch_obs->SatObs[i].C_No<40)
            {
                num_obs--;
                continue;
            }
            */
            
        }
        
        for(int i=0;i<32;i++)
        {
            if(epoch_obs->SatObs[i].PRN != i+1 ) continue;  ///< i科卫星观测值的编号PRN不等于i+1，则表示未观测到
           // if(gps_xyz[i].x==0||epoch_obs->SatObs[i].C_No<40) continue;
            if(gps_xyz[i].x==0) continue;
             
          //  Cal_Clock_Bia(&eph[i], &T_tr_gps, &deta_t_j,&d_clock_bia_i); 
            // int k=obs[i].PRN;    //第i个观测值的卫星号k;  数组中为k-1;
            XYZToBLH(receive_xyz, &rec_blh);
            double E=0,A=0;      ///< 卫星相对于接收机的高度角和方向角
            T_Klo=0;
            T_Hop=0;
            t_klo=0;
            if(receive_xyz->x==0) {
                E=PI/2; A=0;
                rec_blh.B=0;
                rec_blh.L=0;
                rec_blh.H=0;
                T_Klo=0;
                T_Hop=0;
            }
            else
            {
                XYZToBLH(receive_xyz, &rec_blh);
                Cal_E_A(receive_xyz, &gps_xyz[i], &E, &A);  ///< 解算E和A
               //  printf("PRN: %d E: %f A: %f \n",i+1,E*180/PI,A*180/PI);
                T_Klo=Klobutchar(Ion, &epoch_obs->Time, &rec_blh, E, A);  ///< 单位是s
                t_klo=T_Klo*C;
                T_Hop=Hopfield(E, rec_blh.H);
            }                          ///< 单位是m ？
            /*
            if( (E/PI*180) <10) ///小于10删掉
            {
                num_obs--;
                continue;
            }
            */
            //? 伪距的定义式
            double p0=Distance(&gps_xyz[i], receive_xyz);    ///< 第k号卫星的与测站初值的距离
            //p0=p0 + (Deta_t_tr[i] - T_Klo ) *C - T_Hop ;    ///< 伪距改正：加卫星钟差，减电离层/对流层延迟
           // cout<<i+1<<" : "<<t_klo<<" "<<T_Hop<<endl;
            
            double clock_delay=Deta_t_tr[i] *C;
      //      printf("%d : %f %f %10.10f" , i+1,t_klo,T_Hop,Deta_t_tr[i]);
            double psr_correct=epoch_obs->SatObs[i].psr + (Deta_t_tr[i] - T_Klo ) *C - T_Hop;//+X.Val[3];///< 伪距改正：加卫星钟差，减电离层/对流层延迟
           //

            if(IsEarthCov==0 && flag==0)   // flag=0 是因为dgsp接着spp  
            {
                double t_c=psr_correct/C ; ///< t_c信号传播时间
                deta_wmiga=Wmiga_e*t_c;  ///< 地球自转角度
                struct BLH gps_blh_i;
                XYZToBLH(&gps_xyz[i], &gps_blh_i);  ///< 改正卫星位置
                gps_blh_i.L-=deta_wmiga;
                BLHToXYZ(&gps_blh_i, &gps_xyz[i]);
               // IsEarthCov=1;
            }
            
            
            p0=Distance(&gps_xyz[i], receive_xyz);    ///< 第k号卫星的与测站初值的距离
            psr_correct=epoch_obs->SatObs[i].psr+ (Deta_t_tr[i] - T_Klo ) *C - T_Hop -d_clok;///< 伪距改正：加卫星钟差，减电离层/对流层延迟
			/// 改正数改正伪距 +PRC
			if (Psrcre[i].age == gpseph[i].IODE1 && flag==1)
			{
				double zt0 = Psrcre[i].Zcount * 0.6;     /// 发布改正数的时间（一个小时内的秒数） 
				double prc, rrc;
				int sflag = Psrcre[i].s;
				if (sflag == 0)
				{
					prc = Psrcre[i].prc * 0.02;
					rrc = Psrcre[i].rrc * 0.002;
				}
				else
				{
					prc = Psrcre[i].prc * 0.32;
					rrc = Psrcre[i].rrc * 0.032;
				}
				prc = prc + (fmod(epoch_obs->Time.SecOfWeek, 3600) - zt0) *rrc;   /// 对3600秒取余数  得到当前时刻的prc
				psr_correct = epoch_obs->SatObs[i].psr + Deta_t_tr[i] * C - d_clok + prc;
			}

            
        //    printf("PRN: %d x: %f y: %f z: %f  改正后伪距: %f  几何距离: %f 高度角E: %f 方位角A: %f\n",i+1,gps_xyz[i].x,gps_xyz[i].y,gps_xyz[i].z, psr_correct,  p0,E*180/PI,A*180/PI);
            
            ///线性化。计算H矩阵
            Y.Val[k]=psr_correct - p0 ; ///< 第i个观测值的与线性化初始距离测差。改正后的 , 再减去线性化的初值距离。
            double l0= (receive_xyz->x - gps_xyz[i].x) /p0;
            double m0= (receive_xyz->y - gps_xyz[i].y) /p0;
            double n0= (receive_xyz->z - gps_xyz[i].z) /p0;
            H.Val[k*4+0]=l0;
            H.Val[k*4+1]=m0;
            H.Val[k*4+2]=n0;
            H.Val[k*4+3]=1; ///< 接收机钟差改正
            
            ///W权矩阵
			if (flag == 1)
				W.Val[k*num_obs + k] = 1.0/ epoch_obs->SatObs[i].psr_std / epoch_obs->SatObs[i].psr_std;        ///< 权值为观测值标准差的倒数
			else  if(flag==0)
			{
				W.Val[k*num_obs + k] = 1.0 / epoch_obs->SatObs[i].psr_std/ epoch_obs->SatObs[i].psr_std;;
			}
            ++k;
          //  printf("%d \n",k);
        }
        /// 实际数量与记录数量不同，退出
        if(k!=num_obs)
        {
            printf("实际卫星数量与记录数量不同,error! %d %d  \n",k,num_obs);
            receive_xyz->x=last_rec_xyz.x;
            receive_xyz->y=last_rec_xyz.y;
            receive_xyz->z=last_rec_xyz.z;
            return ;
        }
         if(num_obs<4)
         {
             receive_xyz->x=last_rec_xyz.x;
             receive_xyz->y=last_rec_xyz.y;
             receive_xyz->z=last_rec_xyz.z;
             return ;
         }
     //   if(fabs(receive_xyz->x)>3000000) return ;
        
        Y.Row=num_obs; Y.Col=1;      ///< 观测值
        H.Row=num_obs; H.Col=4;      ///< H矩阵
        X.Row=4; X.Col=1;            ///< 待估坐标和钟差
        W.Row=num_obs; W.Col=num_obs;///< 观测值的权矩阵 , 对角阵
        
        /*
        for(int i=0;i<num_obs;i++)
        {
            printf("%f  %f  %f  %d \n", H.Val[i*4+0],H.Val[i*4+1],H.Val[i*4+2],1);
        }
        printf("\n");
        for(int i=0;i<num_obs;i++)
        {
            printf("%f \n", Y.Val[i]);
        }
        printf("\n");
        */
        
        struct Matrix Ht; Ht.Row=4;Ht.Col=num_obs;  ///< Ht  4xN
        Matrix_Transpose(&H, &Ht);
        
        struct Matrix T; T.Row=4;T.Col=num_obs;      ///< T   Ht x W  4xN
        Matrix_Mutiply(&Ht, &W, &T);
        
       // struct Matrix N; N.Col=4;N.Row=4;          ///< N  4x4
        Matrix_Mutiply(&T, &H, &N);
        int mflag=Matrix_Inv(&N, &N);
        if(mflag==0)
        {
            receive_xyz->x=last_rec_xyz.x;
            receive_xyz->y=last_rec_xyz.y;
            receive_xyz->z=last_rec_xyz.z;
            return ;
        }
        
        struct Matrix M; M.Col=1;M.Row=4;          ///< M  4x1
        //Matrix_Mutiply(&Ht, &Ht, &N);
        Matrix_Mutiply(&T, &Y, &M);
        Matrix_Mutiply(&N, &M, &X);
        if( (X.Val[0]*X.Val[0] + X.Val[1]*X.Val[1] +X.Val[2]*X.Val[2] + X.Val[3]*X.Val[3] ) <0.0000001 ) break;
        receive_xyz->x += X.Val[0];
        receive_xyz->y += X.Val[1];
        receive_xyz->z += X.Val[2];
        d_clok+=X.Val[3];
        ///下debug用
        struct BLH blh;
        struct XYZ xyz;
        xyz.x=receive_xyz->x;
        xyz.y=receive_xyz->y;
        xyz.z=receive_xyz->z;
        XYZToBLH(&xyz, &blh);
      //  printf("接收机坐标的计算坐标为: x: %f y:%f z :%f\n",receive_xyz->x,receive_xyz->y,receive_xyz->z);
      //  printf("接收机坐标的计算坐标为: b: %f l:%f h :%f\n",blh.B/PI*180,blh.L/PI*180,blh.H);

        
        loop_time--;
        for(int i=0;i<k;i++)
        {
            W.Val[i*num_obs+i]=0;
        }
        IsEarthCov=1;
    }
   // IsEarthCov=1;
	
    if(loop_time<=1 || fabs(receive_xyz->x)>2300000 )
    {
        receive_xyz->x=last_rec_xyz.x;
        receive_xyz->y=last_rec_xyz.y;
        receive_xyz->z=last_rec_xyz.z;
        return ;
    }
    
    struct Matrix v;
    v.Row=num_obs; v.Col=1;
    struct Matrix y1;
    y1.Row=num_obs; y1.Col=1;
    
    Matrix_Mutiply(&H, &X, &y1);
    Matrix_Subtract(&y1, &Y, &v);
    
 //   Matrix_Mutiply(&W, &v, &pv);
    
    struct Matrix vt;
    vt.Row=1; vt.Col=num_obs;
    Matrix_Transpose(&v, &vt);
    
    struct Matrix pv;
    pv.Col=num_obs; pv.Row=1;
    Matrix_Mutiply(&vt, &W, &pv);
    
    
    
    struct Matrix sig;
    sig.Row=1;sig.Col=1;
    Matrix_Mutiply(&pv, &v, &sig);
    
    
    double sigma;           ///< 验后单位圈方差
    if(num_obs<=4) sigma=0;
    else
        sigma= sqrt(sig.Val[0]/(num_obs-4) );
    
    
     
    epoch_obs->sig=sigma;
    
    double dop=0;
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(i==j) dop+=sqrt(N.Val[i*4+j])*sigma;
        }
    }
    epoch_obs->dop=dop;
    
    /*
    if(sigma>12)
    {
        receive_xyz->x=last_rec_xyz.x;
        receive_xyz->y=last_rec_xyz.y;
        receive_xyz->z=last_rec_xyz.z;
        return ;
    }
    */
   // FILE * fpp;
    /*
    fpp=fopen("/Users/learning_wh/Desktop/log1dop.txt","a");
    //接收机坐标的计算坐标为
    //fprintf(fpp,"时间: %d  %f  :x: %f y: %f z: %f  b: %f l: %f h: %f  \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,xyz.x,xyz.y,xyz.z  ,blh.B/PI*180,blh.L/PI*180,blh.H); fclose( fpp );
    fprintf(fpp," %d  %f  %f \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,sigma);
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            fprintf(fpp," %f  ",N.Val[i*4+j]*sigma);
        }
        fprintf(fpp,"\n");
    }
     fclose( fpp );
    
    fpp=fopen("/Users/learning_wh/Desktop/dop1.txt","a");
    //接收机坐标的计算坐标为
    //fprintf(fpp,"时间: %d  %f  :x: %f y: %f z: %f  b: %f l: %f h: %f  \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,xyz.x,xyz.y,xyz.z  ,blh.B/PI*180,blh.L/PI*180,blh.H); fclose( fpp );
   // fprintf(fpp," %d  %f  %f \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,sigma);
    double sum=0;
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(i==j) sum+=N.Val[i*4+j]*sigma*N.Val[i*4+j]*sigma;
        }
    }
     fprintf(fpp," %f \n",sum);
    fclose( fpp );
    */
    
    
    
    struct BLH blh;
    struct XYZ xyz;
    xyz.x=receive_xyz->x;
    xyz.y=receive_xyz->y;
    xyz.z=receive_xyz->z;
    XYZToBLH(&xyz, &blh);
    
    
    
    if(fabs (blh.H)>100 )
    {
        receive_xyz->x=last_rec_xyz.x;
        receive_xyz->y=last_rec_xyz.y;
        receive_xyz->z=last_rec_xyz.z;
      //  xyz.z=receive_xyz->z;
      //  printf("error \n");
    }
    if(fabs(blh.H)<20)
    {
        xyz.z=receive_xyz->z;
    }
    
    
   // printf("接收机坐标的计算坐标为: x: %f y:%f z :%f\n",receive_xyz->x,receive_xyz->y,receive_xyz->z);
    
 //   FILE * fpp;
  //  fpp=fopen("/Users/learning_wh/Desktop/log2.txt","a+");
    //接收机坐标的计算坐标为
    //fprintf(fpp,"时间: %d  %f  :x: %f y: %f z: %f  b: %f l: %f h: %f  \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,xyz.x,xyz.y,xyz.z  ,blh.B/PI*180,blh.L/PI*180,blh.H); fclose( fpp );
   // fprintf(fpp," %d  %f  %f  %f  %f   %f  %f  %f  \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,xyz.x,xyz.y,xyz.z  ,blh.B/PI*180,blh.L/PI*180,blh.H);
    
   // fclose(fpp);
    return ;
}



/**********************************************
 *@brief  Cal_Rec_Vel    计算接收机的速度
 
 *@param  epoch_obs    I   一个历元的观测数据
 *@param  gps_xyz      I   GPS卫星位置,大小为32的数组
 *@param  gps_vel      I   GPS卫星速度,大小为32的数组
 *@param  d_clock_bia  I   GPS卫星钟差,大小为32的数组
 *@param  receive_xyz  I   接收机的位置
 *@param  receive_vel  O   接收机的速度
 **********************************************/
int Cal_Rec_Vel(struct EPOCHOBS *epoch_obs,struct XYZ gps_xyz[],struct XYZ gps_vel[],double d_clock_bia[],struct XYZ *receive_xyz,struct XYZ *receive_vel)
{
    int num_obs=epoch_obs->NumOfObs;
    if(num_obs<4) return 0;
    struct Matrix Y; Y.Row=num_obs; Y.Col=1;      ///< 观测值
    struct Matrix H; H.Row=num_obs; H.Col=4;      ///< H矩阵
    struct Matrix X; X.Row=4; X.Col=1;            ///< 待估速度和钟差
    struct Matrix W; W.Row=num_obs; W.Col=num_obs;///< 观测值的权矩阵 , 对角阵
    struct BLH rec_blh;
    XYZToBLH(receive_xyz, &rec_blh);
    struct XYZ last_v;
    last_v.x=receive_xyz->x;
    last_v.y=receive_xyz->y;
    last_v.z=receive_xyz->z;
   // printf("接收机坐标的计算坐标为: x: %f y:%f z :%f\n",receive_xyz->x,receive_xyz->y,receive_xyz->z);
   // printf("接收机坐标的计算坐标为: b: %f l:%f h :%f\n",rec_blh.B/PI*180,rec_blh.L/PI*180,rec_blh.H);
   // int loop_time=10; ///< 迭代最大次数
    int k=0;          ///< 矩阵的行数
    
    for(int i=0;i<32;i++)
    {
        if(epoch_obs->SatObs[i].PRN != i+1 ) continue;  ///< i科卫星观测值的编号PRN不等于i+1，则表示未观测到
        if(gps_vel[i].x==0)
        {
            num_obs--;
            continue;
        }
        double p0=Distance(&gps_xyz[i], receive_xyz);    ///< 第i+1号卫星的与测站的距离
        double D=epoch_obs->SatObs[i].dopp;              ///< i+1号卫星的多普勒测量值Hz
        D=D*C/1575.42/1000000;  ///< 转为m
        
        ///线性化。计算H矩阵
        // Y.Val[k]=psr_correct - p0;; ///< 第i个观测值的与线性化初始距离测差。改正后的 , 再减去线性化的初值距离。
        double l0= (receive_xyz->x - gps_xyz[i].x) /p0;
        double m0= (receive_xyz->y - gps_xyz[i].y) /p0;
        double n0= (receive_xyz->z - gps_xyz[i].z) /p0;
        
        struct BLH blh_vel;
        XYZToBLH(&gps_vel[i], &blh_vel);
        blh_vel.L -= p0/C*Wmiga_e;
        BLHToXYZ(&blh_vel, &gps_vel[i]);
         
        double d_p0= 0 - gps_vel[i].x *l0 - gps_vel[i].y *m0 - gps_vel[i].z *n0;
         
        Y.Val[k]=- D - d_p0 + C*d_clock_bia[i] ;
        
        H.Val[k*4+0]=l0;
        H.Val[k*4+1]=m0;
        H.Val[k*4+2]=n0;
        H.Val[k*4+3]=1; ///< 接收机钟差改正
        
        ///W权矩阵
        W.Val[k*num_obs+k] = epoch_obs->SatObs[i].C_No;///epoch_obs->SatObs[i].psr_std;    ///< 权值为观测值标准差的倒数
        k++;
    }
    /// 实际数量与记录数量不同，退出
    if(k!=num_obs)
    {
        printf("实际卫星数量与记录数量不同,error!\n");
        receive_xyz->x=last_v.x;
        receive_xyz->y=last_v.y;
        receive_xyz->z=last_v.z;
        return 0;  ///< 算不了速度
    }
    struct Matrix Ht; Ht.Row=4;Ht.Col=num_obs;  ///< Ht  4xN
    Matrix_Transpose(&H, &Ht);
    
    struct Matrix T; T.Row=4;T.Col=num_obs;      ///< T   Ht x W  4xN
    Matrix_Mutiply(&Ht, &W, &T);
    
    struct Matrix N; N.Col=4;N.Row=4;          ///< N  4x4
    Matrix_Mutiply(&T, &H, &N);
    Matrix_Inv(&N, &N);
    
    struct Matrix M; M.Col=1;M.Row=4;       ///< M  4x1
    //Matrix_Mutiply(&Ht, &Ht, &N);
    Matrix_Mutiply(&T, &Y, &M);
    Matrix_Mutiply(&N, &M, &X);
    
    if(fabs(X.Val[0])+fabs(X.Val[1])+fabs(X.Val[2])>1)
    {
        receive_xyz->x=last_v.x;
        receive_xyz->y=last_v.y;
        receive_xyz->z=last_v.z;
        return 0;
    }
    
    ///< 更新接收机速度
    receive_vel->x=X.Val[0];
    receive_vel->y=X.Val[1];
    receive_vel->z=X.Val[2];

   // printf("接收机的速度为: x: %f y:%f z :%f\n",receive_xyz->x,receive_xyz->y,receive_xyz->z);
    
  //  FILE * fpp;
  //  fpp=fopen("/Users/learning_wh/Desktop/log2v.txt","a");
   // fprintf(fpp,"时间: %d  %f  接收机的速度为:Vx: %f Vy: %f Vz: %f \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,receive_vel->x,receive_vel->y,receive_vel->z); fclose( fpp );
  //  fprintf(fpp,"%d  %f %f %f  %f \n",epoch_obs->Time.Week,epoch_obs->Time.SecOfWeek,receive_vel->x,receive_vel->y,receive_vel->z); fclose( fpp );
    return 1;
}






