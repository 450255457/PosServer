/*************************************************************************
	> File Name: public_function.cpp
	> Author: linden
	> Mail: 450255457@qq.com 
	> Created Time: 2015��03��29�� ������ 10ʱ30��31��
 ************************************************************************/

#include "public_function.h"

CPublicFunction::CPublicFunction()  
{  
}  
  
CPublicFunction::~CPublicFunction()  
{  
}

unsigned char CPublicFunction::get_bcc_data(unsigned char *data,int len){
	unsigned char bcc = 0;
	int i = 0;
	for(i = 0; i < len; i++)
	{
		bcc = bcc ^ data[i];
	}
	return	bcc;
}

/************************************************
��������:GenerateCRC32
��������:CRC32�㷨
�������:unsigned char *DataBuf
		unsigned long len,ע�⴫�����ĳ���
�������:
����˵��:��Ҫ��crc
*************************************************/
unsigned long byte_int(unsigned char *bytes){
	unsigned long  num = bytes[3] & 0xFF;    
	num |= ((bytes[2] << 8) & 0xFF00);    
	num |= ((bytes[1] << 16) & 0xFF0000);    
	num |= ((bytes[0] << 24) & 0xFF000000);    
	return num;    
}

unsigned long CPublicFunction::GenerateCRC32(unsigned char *DataBuf,unsigned long len){   
	unsigned char bSrc[4];
	unsigned int crc;	//linux��Ҫ��long�ĳ�int��win64��long int ����4��linux��64λ��int��4��long��8
	unsigned long i,j;
	unsigned int temp;
	crc = 0xFFFFFFFF;
	for( i=0;i<len;i++)
	{
		memset(bSrc,0x00,4);
		if (i == len-1)
		{
			if (DataBuf[i*4] ==0xcd)
			{
				DataBuf[i*4] =0x00;
			}
			if (DataBuf[i*4+1] ==0xcd)
			{
				DataBuf[i*4+1] =0x00;
			}
			if (DataBuf[i*4+2] ==0xcd)
			{
				DataBuf[i*4+2] =0x00;
			}
			if (DataBuf[i*4+3] ==0xcd)
			{
				DataBuf[i*4+3] =0x00;
			}
		}
     	bSrc[0] = DataBuf[i*4];
		bSrc[1] = DataBuf[i*4+1];
		bSrc[2] = DataBuf[i*4+2];
		bSrc[3] = DataBuf[i*4+3];
	    temp = byte_int(bSrc);
		for(j=0; j < 32; j++)
		{
	  		if( (crc ^ temp) & 0x80000000 )
			{
	    		crc = 0x04C11DB7 ^ (crc<<1);
	  		}
		    else
            {
                crc <<= 1;
            }
            temp<<=1;
//			printf("crc %ld = %x\n",j,crc);
		}
    }
    return crc;
}

/************************************************
��������:AsciiStrToBcd
��������:��Asc��ת����BCD����
�������:unsigned char *b
		unsigned char len
�������:unsigned char *outbuf
����˵��:
*************************************************/
unsigned char tohex(unsigned char b)
{
	if(b>0x2f&&b<0x3a)return b-0x30;
	else if(b=='a'||b=='A')
		return 0xA;
	else if(b=='b'||b=='B')
		return 0xB;
	else if(b=='c'||b=='C')
		return 0xC;
	else if(b=='d'||b=='D'||b=='=')
		return 0xD;
	else if(b=='e'||b=='E')
		return 0xE;
	else if(b=='f'||b=='F')
		return 0xF;
	else return 0;
}

void CPublicFunction::AsciiStrToBcd(unsigned char *outbuf,unsigned char *b,int len)
{
	unsigned char i,j,k,m;
	i=0;
	m=0;
	if (len%2 != 0)
	{
    	len++;
	}
	for(k=0;k<len;k++)
	{
		j=tohex(*b++);
		i=j<<4;
		j=tohex(*b++);
		i|=j;
		outbuf[m++]=i;
		k++;
	}
}

/************************************************
��������:DecTo2Hex
��������:��10������ֵת����2λ��16HEX
�������:int dec
�������:char *data
����˵��:
*************************************************/
void CPublicFunction::DecTo2Hex(int dec,char *data)
{
    *(data+0) = ((dec >> 8) & 0XFF);
    *(data+1) = dec & 0XFF;
}