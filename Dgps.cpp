#include"Dgps.h"
//#define BUFFSIZE 20480
char sysbuff[BUFFSIZE];              ///< socket ͬ���������   3 + 1 ����ʽ
int BIT29 = 1, BIT30 = 1;        ///< ȫ�ֱ���  29/30 ȡ����ʶ   30 - 0���� 1ȡ��


/************************************
 *@brief  convert16  ʵ���ַ�1-6λ�Ĺ���
 *@param  c    I   ���������ַ�
 *@retval   ��������ַ�
 ************************************/
char convert16(char c)
{
	char res = 0x00;
	char c78 = (c & 0xC0);              ///< ����λ����
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
 *@brief     DecodePsrCroct  ʵ�ֶ�RTCM2.x�Ľ���
 *@param  buff           I  ������ָ��
 *@param  lenth         I  ���ζ������ݵĳ���
 *@param  psr_cro     O  ����õ��Ĳ�ָ����ṹ��
 *@retval    -1            ����ʧ��
 ************************************/
int DecodePsrCroct(char * buff, int lenth, struct PsrCrrect  * psr_cro)
{
	for (int i = 0; i < lenth; i++)
	{
		buff[i] = convert16(buff[i]);     ///< ��ת 1-6
	}
	int pos = 0, pos1 = 0, pos2 = 0;  ///< ԭʼ���ݵ�λ�ñ�ʶ ��   ƴ������λ�ñ�ʶ
	for (int p = 0; p < lenth; )    ///<   һ��recv��������ж���ͷ  �ҵ�ͷ֮��  p =pos1 + (numsseqnece+2) *5 
	{
		//int pos = 0, pos1 = 0, pos2 = 0;  ///< ԭʼ���ݵ�λ�ñ�ʶ ��   ƴ������λ�ñ�ʶ
		/// Ĭ�ϳ�ʼȡ��   bit30 = 1
		for (int i = p; i < lenth - 10; i++)
		{
			char c1 = buff[i];
			char c2 = buff[i + 1];
			c1 = (c1 & 63) << 2;  ///<    & 00111111  63
			c2 = (c2 & 48) >> 4;    ///<    & 00110000  48
			c1 = c1 | c2;                 ///<  c1 c2 ������
			if (BIT30 == 1) c1 = ~c1;
			if (c1 == 0b01100110)    
			{
				pos = i;
				pos1 = i;
				break;
			}
		}


		/// ͬ�����
		/// ȡͷ�ĵ�һ��word   3+1 
		sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
		sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
		sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
		sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< ����λ
		if (BIT30 == 1)
		{
			for (int k = 0; k < 3; k++)
				sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
		}
		///< У��
	//	if (check(sysbuff, pos2) == -1) { return -1;/*����ʧ�� ������-1*/ }
		///< ����bit29/30
		BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
		BIT30 = sysbuff[pos2 + 3] & 0x01;

		/// ȡͷ�ĵڶ���word 
		pos1 += 5;  pos2 += 4;
		sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
		sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
		sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
		sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< ����λ
		if (BIT30 == 1)
		{
			for (int k = 0; k < 3; k++)
				sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
		}
		if (check(sysbuff, pos2) == -1) { return -1;/*����ʧ�� ������-1*/ }
		BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
		BIT30 = sysbuff[pos2 + 3] & 0x01;

		int sysfailflag = 0;                ///< У��ʧ�ܱ�ʶ��  1��ʧ��
		short numsequence =sysbuff[pos2 + 2];                 ///< ֡����
		numsequence = numsequence & 0x00ff;
		numsequence = (numsequence & 248) >> 3    ;      ///<  11111000     N+2 ֡��/��  ������ͷ

		unsigned short zcount = (0x0000 | sysbuff[pos2]) & 0x00ff;   ///< &00ff ֻ�����Ͱ�λ
		zcount = (zcount << 5) | ((sysbuff[pos2 + 1] >> 3) & 0b00011111);   ///< ��һ���ֽ�����5λ�� ȡ�ڶ����ֽڵĵ�5λ������
		char number = 0x00;/// ��������
		number =( sysbuff[pos2 -4 + 1] >>2 )& 0b00111111;
		if (number != 0x01)
		{
			p = pos1 + 5;
			sysfailflag = 1;         /// ��־У��ʧ��
			continue;
		}

		///< �������� 
		if (pos + numsequence * 5 > BUFFSIZE)   ///< ���޴���
		{
			printf("revbuff overflow \n");
			return -1;
		}
		///< ����ȫ������
		for (int i = 0; i < numsequence; i++)
		{
			pos1 += 5;  pos2 += 4;
			sysbuff[pos2] = ((buff[pos1] & 63) << 2) | ((buff[pos1 + 1] & 48) >> 4);     ///<   00111111-63    00110000-48
			sysbuff[pos2 + 1] = ((buff[pos1 + 1] & 15) << 4) | ((buff[pos1 + 2] & 60) >> 2);       ///<  00001111 - 15      00111100 - 60
			sysbuff[pos2 + 2] = ((buff[pos1 + 2] & 3) << 6) | ((buff[pos1 + 3] & 63));     ///<     00000011   -3  00111111  - 63 
			sysbuff[pos2 + 3] = buff[pos1 + 4];    ///< ����λ
			if (BIT30 == 1)
			{
				for (int k = 0; k < 3; k++)
					sysbuff[pos2 + k] = ~sysbuff[pos2 + k];
			}
			BIT29 = (sysbuff[pos2 + 3] & 0x02) >> 1;
			BIT30 = sysbuff[pos2 + 3] & 0x01;
			///< У��
			if (check(sysbuff, pos2) == -1) 
			{
				p = pos1 + 5;
				sysfailflag = 1;    ///  ��ʶͬ��ʧ��  �˳������ݵĶ�ȡ
				break;
			}
		}
		if (sysfailflag == 1)    ///< У��ʧ��  ���¿�ʼͬ��
		{
			continue;
		}
		///< ÿ5��������������������   һ������3+1���ֽ�
		pos2 = pos + 8;   ///< �����忪ʼ���½���
		///  int sflag = 0;    /// s��־   0-> 0.02  1-> 0.32
		for (int i = 0; i < numsequence; i = i + 5)
		{
			///< 5��wordΪһ�飬��У������ʣ15byte,  ȡ���е�3��������  
			char temp[15];                ///< ��ʱ���У�������15��byte
			int p = 0;
			if (i + 5 > numsequence)
			{
				int  y = (numsequence-i) % 5;   ///< �Բ�������ֵĴ���
				if (y == 1) p = 3;
				else if (y == 2 || y == 3) p = 2;
				else if (y == 4) p = 1;
				else p = 0;
			}
			for (int j = 0; j < 15; j++)
			{
				int shang = j / 3;
				temp[j] = sysbuff[pos2 + j + shang];    ///< ���˵�У����
			}
			for (int j = 0; j < 15-p*5; j += 5)
			{
				int sflag = 0;    /// s��־   0-> 0.02  1-> 0.32
				sflag = (temp[j] & 128) >> 7;    ///  10000000 - 128
				int satnum = (temp[j] & 31);     /// 00011111   -  31  �������
				if (satnum == 0) satnum = 32;   ///  ���Ǻ�0-> 32
				//short  psrcrrect = U2((unsigned char *)(temp + 1));   ///< ����α�������   int ?  double ?
				short  prc = temp[j + 1];          ///< α������� short �����ֽ� �ȸ�ֵ��λ������8λ�������8λ��
				prc = prc << 8;
				prc = prc | (unsigned char)temp[j + 2];
				char chrrc = 0x0;    ///< �з��ŵ�α���ֱ仯��
				short rrc = (short)chrrc | temp[j + 3];   ///< ǿתΪshort

				char chage = 0x0;      ///< �з��ŵ�age/iode
				unsigned int age = (unsigned int)(chage | temp[j + 4]); ///<  ǿתΪ unsigned char   ��iode��Ӧ

				///��ֵ
				psr_cro[satnum - 1].PRN = satnum;   ///< ���Ǻ�
				psr_cro[satnum - 1].Zcount = zcount;  ///< z����
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
 *@brief     check        ʵ�ֶ�RTCM2.xһ��word��У��
 *@param  sysbuff      I  �ϲ������ݻ�����ָ��
 *@param  pos2         I  һ��word�Ŀ�ͷ
 *@retval    -1            У��ʧ��
 *@retval    1             У��ɹ�
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
		/// �� 25+k �� 
		char c1 = sysbuff[pos2] & n1[k];
		char c2 = sysbuff[pos2 + 1] & n2[k];
		char c3 = sysbuff[pos2 + 2] & n3[k];
		char c4 = sysbuff[pos2 - 1] & n4[k];  ///< ��һword��У��λ
		for (int t = 0; t < 8; t++)    ///  ��8λ��ÿλ���
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
	///< �ж�b��6λ���µ�У�������λ�Ƿ����
	char check = 0x00;    ///<   �������6λ ��25-30 ���δ���check�ַ���
	for (int k = 0; k < 6; k++)
	{
		check = check << 1;
		check |= b[k];
		//check = check << 1;
	}
	char check2 = sysbuff[pos2 + 3] & 0b00111111;    ///<  ȡ����У��λ�ĺ���λ ��Ϊcheck2
	if (check != check2)
	{
	//	printf("У��ʧ��\n");
		return -1;  ///< У��ʧ��   
	}
	return 1;  ///< У��ɹ�
}