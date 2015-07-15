#include "DataBase.h"
#include "public_function.h"

using namespace std;

file_info filelist[FILEMAX];		//升级文件的个数
long Prom_Write_Address = 0x08020000;

CPublicFunction MyPF;

CDatabase::CDatabase()  
{  
    connection = mysql_init(NULL); // 初始化数据库连接变量  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);
    }  
}  
  
CDatabase::~CDatabase()  
{  
    if(connection != NULL)  // 关闭数据库连接  
    {  
        mysql_close(connection);  
    }  
}  
  
bool CDatabase::initDB(string host, string user, string pwd, string db_name)  
{  
    // 函数mysql_real_connect建立一个数据库连接  
    // 成功返回MYSQL*连接句柄，失败返回NULL  
    connection = mysql_real_connect(connection, host.c_str(),  
            user.c_str(), pwd.c_str(), db_name.c_str(), 0, NULL, 0);  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);  
    }  
    return true;  
}  
  
bool CDatabase::executeSQL(string sql)  
{  
    // mysql_query()执行成功返回0，失败返回非0值。与PHP中不一样  
    if(mysql_query(connection, sql.c_str()))
    {  
        cout << "Query Error:" << mysql_error(connection);  
        exit(1);  
    }  
    else  
    {  
        result = mysql_use_result(connection); // 获取结果集  
        // mysql_field_count()返回connection查询的列数  
        for(int i=0; i < mysql_field_count(connection); ++i)  
        {  
            // 获取下一行  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()返回结果集中的字段数  
            for(int j=0; j < mysql_num_fields(result); ++j)  
            {  
                cout << row[j] << " ";  
            }  
            cout << endl;  
        }  
        // 释放结果集的内存  
        mysql_free_result(result);  
    }  
    return true;  
}

/************************************************
函数名称:get_file_info
函数功能:获取文件信息
输入参数:const int iNum
		const char *cFilePath
输出参数:
返回说明:0/-1
*************************************************/
int CDatabase::get_file_info(const int iNum,const char *cFilePath)
{
	FILE *fp;
	long fileLen = 0;
	unsigned char *filePtr = NULL;
	int i = 0;
	
	if((fp = fopen(cFilePath, "rb")) != NULL)
	{
		fseek(fp,0L, SEEK_END);		//定位到文件末尾
		fileLen = ftell(fp);		//得到文件总大小
		filelist[iNum].lFileLen = fileLen;
		//堆动态分配内存空间
		if(filePtr = (unsigned char *)malloc(fileLen + 14))
		{
			//分配成功
			memset(filePtr,0x00,fileLen);
			fseek(fp,0L,SEEK_SET);				//定位到文件开头
			fread(filePtr,fileLen,1,fp); 		//一次性读取全部文件内容
//			*(filePtr+fileLen)=0; 				//字符串结束标志 
			//数据处理
			if(fileLen % SEND_MAX_SIZE)
				filelist[iNum].iMaxPack = (fileLen/SEND_MAX_SIZE) + 1;
			else
				filelist[iNum].iMaxPack = (fileLen/SEND_MAX_SIZE);
			for(i = 0; i < filelist[iNum].iMaxPack; i++)
			{
				long lOffset = i*SEND_MAX_SIZE;
				memset(filelist[iNum].FileData[i].csData,0X00,SEND_MAX_SIZE);
				filelist[iNum].FileData[i].csData[0] = 0X84;
				filelist[iNum].FileData[i].csData[1] = 0X00;
				filelist[iNum].FileData[i].csData[2] = (((Prom_Write_Address+lOffset)>>24)&0xffff);
				filelist[iNum].FileData[i].csData[3] = (((Prom_Write_Address+lOffset)>>16)&0xffff);
				filelist[iNum].FileData[i].csData[4] = (((Prom_Write_Address+lOffset)>>8)&0xffff);
				filelist[iNum].FileData[i].csData[5] = ((Prom_Write_Address+lOffset)&0xffff);
				filelist[iNum].FileData[i].csData[6] = (((i)>>24)&0xffff);
				filelist[iNum].FileData[i].csData[7] = (((i)>>16)&0xffff);
				filelist[iNum].FileData[i].csData[8] = (((i)>>8)&0xffff);
				filelist[iNum].FileData[i].csData[9] = ((i)&0xffff);
			//一个包的封装
				if((fileLen % SEND_MAX_SIZE) != 0 && ( i == filelist[iNum].iMaxPack - 1))		//是否为最后一个可能非send_max_length的包
				{
					filelist[iNum].FileData[i].nSize = fileLen % SEND_MAX_SIZE;
					filelist[iNum].FileData[i].csData[10] = ((filelist[iNum].FileData[i].nSize>>8)&0xff);
					filelist[iNum].FileData[i].csData[11] = ((filelist[iNum].FileData[i].nSize)&0xff);
					memcpy(filelist[iNum].FileData[i].csData + 12,filePtr + lOffset,filelist[iNum].FileData[i].nSize);
					int j = 12;
					char checksum = 0;
					while(j < filelist[iNum].FileData[i].nSize + 12)
					{
						checksum += filelist[iNum].FileData[i].csData[j];
						filelist[iNum].FileData[i].csData[filelist[iNum].FileData[i].nSize + 12]=(~checksum) + 1;
						j++;
					}
					filelist[iNum].FileData[i].nSize = filelist[iNum].FileData[i].nSize + 12 + 1;
				}
				else
				{
					filelist[iNum].FileData[i].nSize = SEND_MAX_SIZE;
					filelist[iNum].FileData[i].csData[10] = ((filelist[iNum].FileData[i].nSize>>8)&0xff);
					filelist[iNum].FileData[i].csData[11] = ((filelist[iNum].FileData[i].nSize)&0xff);
					memcpy(filelist[iNum].FileData[i].csData + 12,filePtr+lOffset,SEND_MAX_SIZE);
					int j = 12;
					char checksum = 0;
					while(j < SEND_MAX_SIZE + 12)
					{
						checksum += filelist[iNum].FileData[i].csData[j];
						filelist[iNum].FileData[i].csData[SEND_MAX_SIZE + 12]=(~checksum)+1;
						j++;
					}
					filelist[iNum].FileData[i].nSize = filelist[iNum].FileData[i].nSize + 12 + 1;
				}
			}
			//CheckSum
			int iFileLen = fileLen % 4;
			if(iFileLen == 0)
			{
				iFileLen = fileLen / 4;
			}
			else
			{
				iFileLen = fileLen / 4 + 1;
			}
			filelist[iNum].CheckSum = MyPF.GenerateCRC32(filePtr,iFileLen);
			fclose(fp);
			return 0;
		}
		else
		{
			//分配失败
			printf("memorry aplly failed!\n");
			fclose(fp);
			return -1;
		}
	}
	else
	{
		printf("file open failed!\n");
		return -1;
	}
}

/************************************************
函数名称:init_file_info
函数功能:初始化文件信息
输入参数:
输出参数:
返回说明:true/false
*************************************************/
bool CDatabase::init_file_info()
{
	int fileLen = 0;
	char *readPtr = NULL;
	string sql = "select * from t_file_info where chk = '1'";
	if(mysql_query(connection, sql.c_str()))  
    {  
        cout << "Query Error:" << mysql_error(connection);  
        return false; 
    }  
    else  
    {  
        result = mysql_use_result(connection); // 获取结果集  
        // mysql_field_count()返回connection查询的列数  
        for(int i=1; i <= mysql_field_count(connection); ++i)  
        {  
            // 获取下一行  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()返回结果集中的字段数  
            for(int j=0; j < mysql_num_fields(result); ++j)  
            {  
//            	cout << row[j] << " ";  
            	if(j == 0)
        		{
        			fileLen = strlen(row[j]);
					memcpy(filelist[i].nFileID,row[j],fileLen);
        		}
				else if(j == 1)
				{
					filelist[i].cCheck = *row[j];	//由sql查询语句知道cCheck为1
				}
				else if(j == 2)
				{
					fileLen = strlen(row[j]);
					memcpy(filelist[i].Version,row[j],fileLen);
				}
				else if(j == 3)
				{
					fileLen = strlen(row[j]);
					memcpy(filelist[i].filePath,row[j],fileLen);
				}
				else if(j == 4)
				{
					filelist[i].nSoftType = *row[j];
				}
			}
			get_file_info(i,filelist[i].filePath);
        }  
        // 释放结果集的内存  
        mysql_free_result(result);
    }
	return true;
}

/************************************************
函数名称:sn_bin_crc
函数功能:根据SN号查询对应BIN文件的CRC值和ID
输入参数:string sql
输出参数:unsigned char *softCrc		文件的CRC
		int *file_id	文件的ID
返回说明:true/false
*************************************************/
bool CDatabase::sn_bin_crc(string sql,unsigned char *softCrc,int *file_id)
{	
	//string sql = "select softCrc,nsoftverid from t_update_snlist where sn = 'S980114-05020701'";
    // mysql_query()执行成功返回0，失败返回非0值
    unsigned char szSoftCrc[9] = {0};
	unsigned char szfile_id[3] = {0};
	int crcLen = 0,cfile_id_len = 0;
	unsigned int pfile_id = 0;	
    if(mysql_query(connection, sql.c_str()))  
    {  
        cout << "Query Error:" << mysql_error(connection);  
        return false;  
    }  
    //获取结果集
    result = mysql_use_result(connection);  
    // mysql_field_count()返回connection查询的列数  
    for(int i = 0; i < mysql_field_count(connection); ++i){  
        // 获取下一行  
        row = mysql_fetch_row(result);  
        if(row <= 0)  
        {  
            break;  
        }  
        // mysql_num_fields()返回结果集中的字段数  
        for(int j=0; j < mysql_num_fields(result); ++j){
			if(j == 0){
				crcLen = strlen(row[j]);
				memcpy(szSoftCrc,row[j],crcLen);
			}
			else if(j == 1){
				cfile_id_len = strlen(row[j]);
				memcpy(szfile_id,row[j],cfile_id_len);
				*file_id = atoi((const char*)szfile_id);
			}
        }  
    }  
    // 释放结果集的内存  
    mysql_free_result(result);  

	MyPF.AsciiStrToBcd(softCrc,szSoftCrc,8);
    return true;
}
