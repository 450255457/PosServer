#include "DataBase.h"
#include "public_function.h"

using namespace std;

file_info filelist[FILEMAX];		//�����ļ��ĸ���
long Prom_Write_Address = 0x08020000;

CPublicFunction MyPF;

CDatabase::CDatabase()  
{  
    connection = mysql_init(NULL); // ��ʼ�����ݿ����ӱ���  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);
    }  
}  
  
CDatabase::~CDatabase()  
{  
    if(connection != NULL)  // �ر����ݿ�����  
    {  
        mysql_close(connection);  
    }  
}  
  
bool CDatabase::initDB(string host, string user, string pwd, string db_name)  
{  
    // ����mysql_real_connect����һ�����ݿ�����  
    // �ɹ�����MYSQL*���Ӿ����ʧ�ܷ���NULL  
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
    // mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ����PHP�в�һ��  
    if(mysql_query(connection, sql.c_str()))
    {  
        cout << "Query Error:" << mysql_error(connection);  
        exit(1);  
    }  
    else  
    {  
        result = mysql_use_result(connection); // ��ȡ�����  
        // mysql_field_count()����connection��ѯ������  
        for(int i=0; i < mysql_field_count(connection); ++i)  
        {  
            // ��ȡ��һ��  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()���ؽ�����е��ֶ���  
            for(int j=0; j < mysql_num_fields(result); ++j)  
            {  
                cout << row[j] << " ";  
            }  
            cout << endl;  
        }  
        // �ͷŽ�������ڴ�  
        mysql_free_result(result);  
    }  
    return true;  
}

/************************************************
��������:get_file_info
��������:��ȡ�ļ���Ϣ
�������:const int iNum
		const char *cFilePath
�������:
����˵��:0/-1
*************************************************/
int CDatabase::get_file_info(const int iNum,const char *cFilePath)
{
	FILE *fp;
	long fileLen = 0;
	unsigned char *filePtr = NULL;
	int i = 0;
	
	if((fp = fopen(cFilePath, "rb")) != NULL)
	{
		fseek(fp,0L, SEEK_END);		//��λ���ļ�ĩβ
		fileLen = ftell(fp);		//�õ��ļ��ܴ�С
		filelist[iNum].lFileLen = fileLen;
		//�Ѷ�̬�����ڴ�ռ�
		if(filePtr = (unsigned char *)malloc(fileLen + 14))
		{
			//����ɹ�
			memset(filePtr,0x00,fileLen);
			fseek(fp,0L,SEEK_SET);				//��λ���ļ���ͷ
			fread(filePtr,fileLen,1,fp); 		//һ���Զ�ȡȫ���ļ�����
//			*(filePtr+fileLen)=0; 				//�ַ���������־ 
			//���ݴ���
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
			//һ�����ķ�װ
				if((fileLen % SEND_MAX_SIZE) != 0 && ( i == filelist[iNum].iMaxPack - 1))		//�Ƿ�Ϊ���һ�����ܷ�send_max_length�İ�
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
			//����ʧ��
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
��������:init_file_info
��������:��ʼ���ļ���Ϣ
�������:
�������:
����˵��:true/false
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
        result = mysql_use_result(connection); // ��ȡ�����  
        // mysql_field_count()����connection��ѯ������  
        for(int i=1; i <= mysql_field_count(connection); ++i)  
        {  
            // ��ȡ��һ��  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()���ؽ�����е��ֶ���  
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
					filelist[i].cCheck = *row[j];	//��sql��ѯ���֪��cCheckΪ1
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
        // �ͷŽ�������ڴ�  
        mysql_free_result(result);
    }
	return true;
}

/************************************************
��������:sn_bin_crc
��������:����SN�Ų�ѯ��ӦBIN�ļ���CRCֵ��ID
�������:string sql
�������:unsigned char *softCrc		�ļ���CRC
		int *file_id	�ļ���ID
����˵��:true/false
*************************************************/
bool CDatabase::sn_bin_crc(string sql,unsigned char *softCrc,int *file_id)
{	
	//string sql = "select softCrc,nsoftverid from t_update_snlist where sn = 'S980114-05020701'";
    // mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
    unsigned char szSoftCrc[9] = {0};
	unsigned char szfile_id[3] = {0};
	int crcLen = 0,cfile_id_len = 0;
	unsigned int pfile_id = 0;	
    if(mysql_query(connection, sql.c_str()))  
    {  
        cout << "Query Error:" << mysql_error(connection);  
        return false;  
    }  
    //��ȡ�����
    result = mysql_use_result(connection);  
    // mysql_field_count()����connection��ѯ������  
    for(int i = 0; i < mysql_field_count(connection); ++i){  
        // ��ȡ��һ��  
        row = mysql_fetch_row(result);  
        if(row <= 0)  
        {  
            break;  
        }  
        // mysql_num_fields()���ؽ�����е��ֶ���  
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
    // �ͷŽ�������ڴ�  
    mysql_free_result(result);  

	MyPF.AsciiStrToBcd(softCrc,szSoftCrc,8);
    return true;
}
