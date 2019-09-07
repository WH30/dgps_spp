#include"Dgps.h"
//#define BUFFSIZE 20480
char sysbuff[BUFFSIZE];              ///< socket 同步后的数据   3 + 1 的形式
int BIT29 = 1, BIT30 = 1;        ///< 全局变量  29/30 取反标识   30 - 0不变 1取反


/************************************
 *@brief  convert16  实现字符1-6位的滚动
 *@param  c    I   待滚动的字符
 *@retval   滚动后的字符
 ************************************/
char convert16(char c)
{
	char res = 0x00;
	char c78 = (c & 0xC0);              ///< 高两位不变
	char c1 = (c & 0x01) << 5;
	char c2 = (c & 0x02) << 3;
	char c3 = (c & 0x04) << 1;
	char c4 = (c & 0x08) >> 1;
	char c5 = (c & 0x10) >> 3;
	char c6 = (c & 0x20) >> 5;
	res = res | c1|c2|c3| c4|c5|c6|c78;
	return res;
}

/************************************
 *@brief     DecodePsrCroct  实现对RTCM2.x的解码
 *@param  buff           I  缓存区指针
 *@param  lenth         I  本次读入数据的长度
 *@param  psr_cro     O  解码得到的差分改正结构体
 *@retval    -1            解码失败
 ************************************/
int DecodePsrCroct(char * buff, int lenth, struct PsrCrrect  * psr_cro)
{
	for (int i = 0; i < lenth; i++)
	{
		buff[i] = convert16(buff[i]);     ///< 反转 1-6
	}
	int pos = 0, pos1 = 0, pos2 = 0;  ///< 原始数据的位置标识 、   拼接数据位置标识
	for (int p = 0; p < lenth; )    ///<   一条recv里面可能有多条头  找到头之后  p =pos1 + (numsseqnece+2) *5 
	{
		//int pos = 0, pos1 = 0, pos2 = 0;  ///< 原始数据的位置标识 、   拼接数据位置标识
		/// 默认初始取反   bit30 = 1
		for (int i = p; i < lenth - 10; i++)
		{
			char c1 = buff[i];
			char c2 = buff[i + 1];
			c1 = (c1 & 63) << 2;  ///<    & 00111111  63
			c2 = (c2 & 48) >> 4;    ///<    & 00110000  48
			c1 = c1 | c2;                 ///<  c1 c2 或运算
			if (BIT30 == 1) c1 = ~c1;
			if (c1 == 0b01100110)    
			{
				pos = i;
				pos1 = i;
				break;
			}
		}


		/// 同步完成
		/// 取头的第一个word   3+1 
		sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
		sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
		sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
		sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< 检验位
		if (BIT30 == 1)
		{
			for (int k = 0; k < 3; k++)
				sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
		}
		///< 校验
	//	if (check(sysbuff, pos2) == -1) { return -1;/*解码失败 ，返回-1*/ }
		///< 重置bit29/30
		BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
		BIT30 = sysbuff[pos2 + 3] & 0x01;

		/// 取头的第二个word 
		pos1 += 5;  pos2 += 4;
		sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
		sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
		sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
		sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< 检验位
		if (BIT30 == 1)
		{
			for (int k = 0; k < 3; k++)
				sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
		}
		if (check(sysbuff, pos2) == -1) { return -1;/*解码失败 ，返回-1*/ }
		BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
		BIT30 = sysbuff[pos2 + 3] & 0x01;

		int sysfailflag = 0;                ///< 校验失败标识符  1：失败
		short numsequence =sysbuff[pos2 + 2];                 ///< 帧长度
		numsequence = numsequence & 0x00ff;
		numsequence = (numsequence & 248) >> 3    ;      ///<  11111000     N+2 帧数/字  不包括头

		unsigned short zcount = (0x0000 | sysbuff[pos2]) & 0x00ff;   ///< &00ff 只保留低八位
		zcount = (zcount << 5) | ((sysbuff[pos2 + 1] >> 3) & 0b00011111);   ///< 第一个字节左移5位， 取第二个字节的低5位，相与
		char number = 0x00;/// 数据类型
		number =( sysbuff[pos2 -4 + 1] >>2 )& 0b00111111;
		if (number != 0x01)
		{
			p = pos1 + 5;
			sysfailflag = 1;         /// 标志校验失败
			continue;
		}

		///< 解算主体 
		if (pos + numsequence * 5 > BUFFSIZE)   ///< 超限处理
		{
			printf("revbuff overflow \n");
			return -1;
		}
		///< 可以全部解算
		for (int i = 0; i < numsequence; i++)
		{
			pos1 += 5;  pos2 += 4;
			sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
			sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
			sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
			sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< 检验位
			if (BIT30 == 1)
			{
				for (int k = 0; k < 3; k++)
					sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
			}
			BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
			BIT30 = sysbuff[pos2 + 3] & 0x01;
			///< 校验
			if (check(sysbuff, pos2) == -1) 
			{
				p = pos1 + 5;
				sysfailflag = 1;    ///  标识同步失败  退出该数据的读取
				break;
			}
		}
		if (sysfailflag == 1)    ///< 校验失败  重新开始同步
		{
			continue;
		}
		///< 每5个字里面有三个改正数   一个字是3+1个字节
		pos2 = pos + 8;   ///< 从主体开始重新解算
		///  int sflag = 0;    /// s标志   0-> 0.02  1-> 0.32
		for (int i = 0; i < numsequence; i = i + 5)
		{
			///< 5个word为一组，除校验码外剩15byte,  取其中的3个改正数  
			char temp[15];                ///< 临时存除校验码外的15个byte
			int p = 0;
			if (i + 5 > numsequence)
			{
				int  y = (numsequence-i) % 5;   ///< 对不满五个字的处理
				if (y == 1) p = 3;
				else if (y == 2 || y == 3) p = 2;
				else if (y == 4) p = 1;
				else p = 0;
			}
			for (int j = 0; j < 15; j++)
			{
				int shang = j / 3;
				temp[j] = sysbuff[pos2 + j + shang];    ///< 过滤掉校验码
			}
			for (int j = 0; j < 15-p*5; j += 5)
			{
				int sflag = 0;    /// s标志   0-> 0.02  1-> 0.32
				sflag = (temp[j] & 128) >> 7;    ///  10000000 - 128
				int satnum = (temp[j] & 31);     /// 00011111   -  31  卫星序号
				if (satnum == 0) satnum = 32;   ///  卫星号0-> 32
				//short  psrcrrect = U2((unsigned char *)(temp + 1));   ///< 卫星伪距改正数   int ?  double ?
				short  prc = temp[j + 1];          ///< 伪距改正数 short 两个字节 先赋值高位，左移8位，再与低8位与
				prc = prc << 8;
				prc = prc | (unsigned char)temp[j + 2];
				char chrrc = 0x0;    ///< 有符号的伪距差分变化率
				short rrc = (short)chrrc | temp[j + 3];   ///< 强转为short

				char chage = 0x0;      ///< 有符号的age/iode
				unsigned int age = (unsigned int)(chage | temp[j + 4]); ///<  强转为 unsigned char   与iode对应

				///赋值
				psr_cro[satnum - 1].PRN = satnum;   ///< 卫星号
				psr_cro[satnum - 1].Zcount = zcount;  ///< z计数
				psr_cro[satnum - 1].prc = prc;
				psr_cro[satnum - 1].rrc = rrc;
				psr_cro[satnum - 1].s = sflag;
				psr_cro[satnum - 1].age = age;
		//		printf("%d \t %d \t %d \t  %d \n", satnum, zcount, prc,sflag);
			}
			pos2 += 20;
		}
		p = pos1;
	}
}



/************************************
 *@brief     check        实现对RTCM2.x一个word的校验
 *@param  sysbuff      I  合并后数据缓存区指针
 *@param  pos2         I  一个word的开头
 *@retval    -1            校验失败
 *@retval    1             校验成功
 ************************************/
int check(char *sysbuff, int pos2)
{
	int n1[6], n2[6], n3[6], n4[6];
	n1[0] = 0b11101100, n2[0] = 0b01111100, n3[0] = 0b11010010, n4[0] = 0x02;   ///< 25 
	n1[1] = 0b01110110, n2[1] = 0b00111110, n3[1] = 0b01101001, n4[1] = 0x01;   ///< 26
	n1[2] = 0b10111011, n2[2] = 0b00011111, n3[2] = 0b00110100, n4[2] = 0x02;   ///< 27
	n1[3] = 0b01011101, n2[3] = 0b10001111, n3[3] = 0b10011010, n4[3] = 0x01;   ///< 28
	n1[4] = 0b10101110, n2[4] = 0b11000111, n3[4] = 0b11001101, n4[4] = 0x01;   ///< 29
	n1[5] = 0b00101101, n2[5] = 0b11101010, n3[5] = 0b00100111, n4[5] = 0x02;   ///< 30
	char b[6] = { 0,0,0,0,0,0 };
	for (int k = 0; k < 6; k++)
	{
		/// 第 25+k 号 
		char c1 = sysbuff[pos2] & n1[k];
		char c2 = sysbuff[pos2 + 1] & n2[k];
		char c3 = sysbuff[pos2 + 2] & n3[k];
		char c4 = sysbuff[pos2 - 1] & n4[k];  ///< 上一word的校验位
		for (int t = 0; t < 8; t++)    ///  移8位，每位异或
		{
			b[k] ^= (c1&0x01);
			b[k] ^= (c2&0x01);
			b[k] ^= (c3&0x01);
			b[k] ^= (c4&0x01);
			c1 = c1 >> 1;
			c2 = c2 >> 1;
			c3 = c3 >> 1;
			c4 = c4 >> 1;
		}
	}
	///< 判断b的6位与新的校验码后六位是否相等
	char check = 0x00;    ///<   把数组的6位 从25-30 依次存入check字符中
	for (int k = 0; k < 6; k++)
	{
		check = check << 1;
		check |= b[k];
		//check = check << 1;
	}
	char check2 = sysbuff[pos2 + 3] & 0b00111111;    ///<  取本次校验位的后六位 存为check2
	if (check != check2)
	{
	//	printf("校验失败\n");
		return -1;  ///< 校验失败   
	}
	return 1;  ///< 校验成功
}