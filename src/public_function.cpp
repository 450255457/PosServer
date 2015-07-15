/*************************************************************************
	> File Name: public_function.cpp
	> Author: linden
	> Mail: 450255457@qq.com 
	> Created Time: 2015年03月29日 星期四 10时30分31秒
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
函数名称:GenerateCRC32
函数功能:CRC32算法
输入参数:unsigned char *DataBuf
		unsigned long len,注意传进来的长度
输出参数:
返回说明:需要的crc
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
	unsigned int crc;	//linux下要把long改成int，win64下long int 都是4，linux下64位，int是4，long是8
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
函数名称:AsciiStrToBcd
函数功能:将Asc码转换成BCD数据
输入参数:unsigned char *b
		unsigned char len
输出参数:unsigned char *outbuf
返回说明:
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
函数名称:DecTo2Hex
函数功能:将10进制数值转换成2位的16HEX
输入参数:int dec
输出参数:char *data
返回说明:
*************************************************/
void CPublicFunction::DecTo2Hex(int dec,char *data)
{
    *(data+0) = ((dec >> 8) & 0XFF);
    *(data+1) = dec & 0XFF;
}