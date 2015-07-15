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

#define	FILEMAX			2+1			//升级文件的个数
#define	SEND_MAX_LENGTH	1300
#define	SEND_MAX_SIZE	1024

using namespace std;

typedef struct _file_contont
{
	char csData[SEND_MAX_LENGTH];	//要发送的数据
	int  nSize;						//当前包的发送数据大小
}file_contont;

typedef struct _file_info
{
	char	cCheck; 				//当前文件是否选
	char	Version[16]; 			//版本号
	char	nFileID[4]; 			//文件ID
	char	filePath[100];			//文件路径
	char	nSoftType;				//文件类型
	long	CheckSum;  				//校验值
	long	lFileLen; 				//文件长度
	int 	iMaxPack;				//文件包的个数
	file_contont FileData[400];		//文件内容
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


