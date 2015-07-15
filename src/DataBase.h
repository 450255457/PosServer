#ifndef _DATABASE_H
#define _DATABASE_H

#include <iostream>  
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <list>
#include <stdio.h>

#define	FILEMAX			2+1			//�����ļ��ĸ���
#define	SEND_MAX_LENGTH	1300
#define	SEND_MAX_SIZE	1024

using namespace std;

typedef struct _file_contont
{
	char csData[SEND_MAX_LENGTH];	//Ҫ���͵�����
	int  nSize;						//��ǰ���ķ������ݴ�С
}file_contont;

typedef struct _file_info
{
	char	cCheck; 				//��ǰ�ļ��Ƿ�ѡ
	char	Version[16]; 			//�汾��
	char	nFileID[4]; 			//�ļ�ID
	char	filePath[100];			//�ļ�·��
	char	nSoftType;				//�ļ�����
	long	CheckSum;  				//У��ֵ
	long	lFileLen; 				//�ļ�����
	int 	iMaxPack;				//�ļ����ĸ���
	file_contont FileData[400];		//�ļ�����
}file_info;

class CDatabase
{  
public:  
    CDatabase();  
    ~CDatabase();
		
    bool	initDB(string host, string user, string pwd, string db_name);  
    bool	executeSQL(string sql);
	bool	init_file_info();
	int		get_file_info(const int iNum,const char *cFilePath);
	bool	sn_bin_crc(string sql,unsigned char *softCrc,int *file_id);
private:  
    MYSQL *connection;  
    MYSQL_RES *result;  
    MYSQL_ROW row;  
};  

#endif


