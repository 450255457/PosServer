//============================================================================
// Name        : thread_pool.cpp
// Author      : linden
// Version     :
// Copyright   : Your copyright notice
// Description : thread_pool in C++, Ansi-style
//============================================================================

#include "thread_pool.h"

extern file_info filelist[FILEMAX];		//升级文件的个数
extern long Prom_Write_Address;

CPosServer PS;

int Erase_APP(char *Buff,unsigned int nFileLen);

threadpool::threadpool(){
	
}

threadpool::~threadpool(){
	
}

/************************************************
函数名称:threadpool_init
函数功能:线程池的初始化
输入参数:int thread_number,线程池中线程的数量
		int max_requests,请求队列中最多允许的,等待处理的请求的数量
输出参数:
返回说明:0
*************************************************/
int threadpool::threadpool_init(int thread_number, int max_requests){
	m_stop = false;
	m_threads = NULL;
	m_max_requests = max_requests;
    if( ( thread_number <= 0 ) || ( max_requests <= 0 ) ){
        throw std::exception();
    }

    m_threads = new pthread_t[ thread_number ];			//动态生成对象数组
    if( ! m_threads )
    {
        throw std::exception();
    }
	//创建thread_number个线程,并将它们都设置为脱离线程
    for ( int i = 0; i < thread_number; ++i )
    {
        //printf( "create the %dth thread\n", i );
        if( pthread_create( m_threads + i, NULL, worker,this ) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        if( pthread_detach( m_threads[i] ) )
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
	return 0;
}

void threadpool::threadpool_destroy(){
	delete [] m_threads;
    m_stop = true;
}

/************************************************
函数名称:append
函数功能:线程池中任务队列的添加
输入参数:void* request,传进来的参数指针
输出参数:
返回说明:true/false
*************************************************/
bool threadpool::append( void* request )
{
	//操作工作队列时一定要加锁,因为它被所有线程共享
    m_queuelocker.lock();
    if ( m_workqueue.size() > m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back( request );
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

//工作线程运行的函数,它不断从工作队列中取出任务并执行之
void* threadpool::worker( void* arg )
{
	threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}

void threadpool::run()
{
	CPublicFunction MyPF;
	//printf("pthread_self = %x\n",pthread_self());
    while ( ! m_stop )
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if ( m_workqueue.empty() )
        {
            m_queuelocker.unlock();
            continue;
        }
        void* request = m_workqueue.front();	//返回第一个元素的引用
        m_workqueue.pop_front();				//删除链表头的一元素
        m_queuelocker.unlock();
        if ( ! request )
        {
            continue;
        }
        //request->process();
		thread_para *thread_arg = (thread_para *)request;
		int sockfd = thread_arg->sockfd;
		int epollfd = thread_arg->epollfd;
		//printf( "start new thread to receive data on sockfd: %d\n", sockfd );
		char recv_buf[BUFMAXSIZE];  
		memset( recv_buf, '\0', BUFMAXSIZE );
		//循环读取socket上的数据,直到遇到EAGAIN错误.
		while( 1 ){
			int ret = recv( sockfd, recv_buf, BUFMAXSIZE - 1, 0 ); 
			if( ret == 0 ){  
				close( sockfd );   
				//printf( "foreiner closed the connection\n" );   
				break;  
			}   
			else if( ret < 0 ){
				//对于非阻塞IO,下面的条件成立表示数据已经全部读取完毕。此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
				if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
					PS.reset_oneshot( epollfd, sockfd );
					//printf( "read later\n" );
					break;            
				}        
			}
			char pbuf[BUFMAXSIZE] =  {0};				//接收的数据buf
			char data_len[2] = {0};						//数据大小
			char version_no[16] = {0};					//版本号
			char crc_32[4] = {0};						//CRC32
			char packet_no[2] = {0};					//包号
			char sn_no[16] = {0};						//SN号
			int file_id = 0;							//软件ID
			int i = 0,j = 0,ilen = 0,ipacket_no = 0,isend = 0;			
			unsigned char softCrc[4] = {0};				//bin文件CRC32
			unsigned char send_buf[BUFMAXSIZE] = {0};	//发送的数据
			unsigned char Erase_Data[64] = {0};
			int Erase_len = 0;
			PACK_SEND_DATA Send_Data;
			bzero(&Send_Data,sizeof(PACK_SEND_DATA));
			PACK_SEND_REVICE_DATA send_revice_data;
			memcpy(pbuf,recv_buf,BUFMAXSIZE);
			if(pbuf[0] != 0X54 || pbuf[1] != 0X00/* || pbuf[45] != 0X5B*/){		//54，00为包头
				cout << "the data head or tail error!" << endl;
				return;
			}
			data_len[0] = pbuf[2];
			data_len[1] = pbuf[3];
			
			for(i = 0; i < 16; i++){
				version_no[i] = pbuf[6 + i];
			}
			for(i = 0; i < 4; i++){
				crc_32[i] = pbuf[22 + i];
			}
			//客户端发来的包号
			for(i = 0; i < 2; i++){		
				packet_no[i] = pbuf[26 + i];		
			}
			ipacket_no = ((unsigned char)packet_no[0] << 8)  + (unsigned char)packet_no[1];
			//if(pbuf[28] != 0X10){
				//cout << "the sn len error!" << endl;
				//pthread_exit((void *)-1);
			//}
			for(i = 0; i < 16; i++){
				sn_no[i] = pbuf[29 + i];
			}
			//根据sn号查对应的bin文件ID,然后检查CRC32是否相同
			//string sql = "select softCrc,nsoftverid from t_update_snlist where sn = 'S980114-05020701'";
			string sql = "select softCrc,nsoftverid from t_update_snlist where sn = ";
			string ssn_no = sn_no;
			sql = sql + "'" + ssn_no + "'";
			CDatabase MyDB;
			MyDB.initDB("localhost", "root", "123456", "InfoData");
			MyDB.sn_bin_crc(sql,softCrc,&file_id);
			//如果SN号不在数据库里呢???
			if((file_id == 0) || (softCrc[0] == '\0')){
				cout << "the SN may be not in the sql!" << endl;
				send_revice_data.Head[0] = 0X45;
				send_revice_data.Datalen[0]	= 0x00;
				send_revice_data.Datalen[1]	= 0x09;
				send_revice_data.Cmd[0] = 0x01;
				send_revice_data.Cmd[1] = 0X00;			//回复终端可以更新
				memcpy(send_revice_data.CrcCmd,softCrc,4);
				char pack_no[2] = {0};
				MyPF.DecTo2Hex(filelist[file_id].iMaxPack,pack_no);
				memcpy(send_revice_data.nPackNo,pack_no,2);
				send_revice_data.nCheck[0] = 0X03;
				ilen = sizeof(PACK_SEND_REVICE_DATA);
				memcpy(send_buf+2,&send_revice_data,ilen);
				send_buf[ilen+2-1] = MyPF.get_bcc_data(send_buf + 6,ilen - 2);
				send_buf[0] = (ilen >> 8) & 0xff;
				send_buf[1] = ilen & 0xff;
				isend = send(sockfd,send_buf,ilen+2,0);
				if(isend < 0){
					perror("send error");
					return;
				}
			}
			if(pbuf[4] == 0X00 && pbuf[5] == 0X01){		//检查是否更新
				send_revice_data.Head[0] = 0X45;
				send_revice_data.Datalen[0]	= 0x00;
				send_revice_data.Datalen[1]	= 0x09;
				send_revice_data.Cmd[0] = 0x01;
				send_revice_data.Cmd[1] = 0X00;			//CRC32相同，回复终端不需更新
				if(memcmp(crc_32,softCrc,4) != 0){
					send_revice_data.Cmd[1] = 0X01;		//CRC32不相同，回复终端可以更新
				}
				else{
					cout << sn_no
						<< ",it doesn't need the update." <<  endl;
				}
				//CRC32
				memcpy(send_revice_data.CrcCmd,softCrc,4);
				//发送的包号,文件最大包号
				//send_revice_data.nPackNo[0] = (((filelist[file_id].iMaxPack) >> 8)&0xffff);
				//send_revice_data.nPackNo[1]	= ((filelist[file_id].iMaxPack)&0xffff);
				char pack_no[2] = {0};
				MyPF.DecTo2Hex(filelist[file_id].iMaxPack,pack_no);
				memcpy(send_revice_data.nPackNo,pack_no,2);
				//CHECKSUM
				send_revice_data.nCheck[0] = 0X03;
				ilen = sizeof(PACK_SEND_REVICE_DATA);
				memcpy(send_buf+2,&send_revice_data,ilen);
				send_buf[ilen+2-1] = MyPF.get_bcc_data(send_buf + 6,ilen - 2);
				send_buf[0] = (ilen >> 8) & 0xff;
				send_buf[1] = ilen & 0xff;
				//发送数据
				isend = send(sockfd,send_buf,ilen+2,0);
				if(isend < 0){
					perror("send error");
					return;
				}
				//printf("send_buf is checking update.\n");
			}
			else if(pbuf[4] == 0X00 && pbuf[5] == 0X02){		//更新
				if(ipacket_no == 0){
					Erase_len = Erase_APP((char *)Erase_Data,filelist[file_id].lFileLen);
					memcpy(send_buf+2,Erase_Data,Erase_len);
					send_buf[0] = (Erase_len >> 8) & 0xff;
					send_buf[1] = Erase_len & 0xff;
					//发送数据
					isend = send(sockfd,send_buf,Erase_len+2,0);
					if(isend < 0){
						perror("send error");
						return;
					}
					cout << "Test:run here-->ipacket_no == 0!" << endl;
				}
				else if(ipacket_no <= filelist[file_id].iMaxPack){
					Send_Data.Head[0] = 0X45;
					Send_Data.Cmd[0] = 0X02;
					int isend_len = filelist[file_id].FileData[ipacket_no-1].nSize;
					if((isend_len > SEND_MAX_LENGTH) || (isend_len <= 0)){
						return;
					}
					Send_Data.Datalen[0] = (((isend_len + 4 - 1) >> 8) & 0xff);   //去掉有效数据中的最后一位CK 
					Send_Data.Datalen[1] = ((isend_len + 4 - 1)) & 0xff;    	 //去掉有效数据中的最后一位CK     
					Send_Data.nPackNo[0] = ((ipacket_no >> 8) & 0xff);
					Send_Data.nPackNo[1] = ipacket_no & 0xff;
					memcpy(Send_Data.Data,filelist[file_id].FileData[ipacket_no-1].csData,isend_len);
					ilen = 8 + isend_len;
					memcpy(send_buf+2,&Send_Data,ilen);
					send_buf[0] = (ilen >> 8) & 0xff;
					send_buf[1] = ilen & 0xff;
					//发送数据
					isend = send(sockfd,send_buf,ilen+2,0);
					if(isend < 0){
						perror("send error");
						return;
					}
					printf("SN:%s-->PacketNo:%d <= MaxPack:%d\n",sn_no,ipacket_no,filelist[file_id].iMaxPack);
				}
				else{
					cout << "PacketNo > MaxPack!" << endl;
					send_revice_data.Head[0] = 0X45;
					send_revice_data.Datalen[0]	= 0x00;
					send_revice_data.Datalen[1]	= 0x9;
					send_revice_data.Cmd[0] = 0X02;
					send_revice_data.Cmd[1] =0x01; 	//更新完成
					send_revice_data.CrcCmd[0] = (((filelist[file_id].CheckSum)>>24)&0xff);
					send_revice_data.CrcCmd[1] = (((filelist[file_id].CheckSum)>>16)&0xff);
					send_revice_data.CrcCmd[2] = (((filelist[file_id].CheckSum)>>8)&0xff);
					send_revice_data.CrcCmd[3] = ((filelist[file_id].CheckSum) & 0xff);
					send_revice_data.nCheck[0] = 0x03;
					ilen = 0X0d;
					memcpy(send_buf + 2,&send_revice_data,ilen);
					send_buf[ilen-1] = MyPF.get_bcc_data(send_buf + 6,ilen - 2);
					send_buf[0] = (ilen >> 8) & 0xff;
					send_buf[1] = ilen & 0xff;
					//发送数据
					isend = send(sockfd,send_buf,ilen+2,0);
					if(isend < 0){
						perror("send error");
						return;
					}
				}
			}
			else if(pbuf[4] >= 0X02 && pbuf[5] == 0X02){	////升级成功或者失败
				send_revice_data.Head[0] = 0X45;
				send_revice_data.Datalen[0]	= 0x00;
				send_revice_data.Datalen[1]	= 0x9;
				send_revice_data.Cmd[0] = 0X02;
				send_revice_data.Cmd[1] = 0X02;
				send_revice_data.CrcCmd[0] = (((filelist[file_id].CheckSum)>>24)&0xff);
				send_revice_data.CrcCmd[1] = (((filelist[file_id].CheckSum)>>16)&0xff);
				send_revice_data.CrcCmd[2] = (((filelist[file_id].CheckSum)>>8)&0xff);
				send_revice_data.CrcCmd[3] = ((filelist[file_id].CheckSum) & 0xff);
				send_revice_data.nCheck[0] = 0x03;
				ilen = 0X0d;
				memcpy(send_buf + 2,&send_revice_data,ilen);
				send_buf[ilen-1] = MyPF.get_bcc_data(send_buf + 6,ilen - 2);
				send_buf[0] = (ilen >> 8) & 0xff;
				send_buf[1] = ilen & 0xff;
				//发送数据
				isend = send(sockfd,send_buf,ilen+2,0);
				if(isend < 0){
					perror("send error");
					return;
				}
			}
			else{
				cout << "the data error!" << endl;
				return;
			}
		}
		//printf( "end thread receiving data on fd: %d\n", sockfd );
		//printf("Test:sockfd = %d,epoll = %d\n",sockfd,epollfd);
		//printf("pthread_self = %x\n",pthread_self());
    }
	return;
}

//返回发送擦除指令
int Erase_APP(char *Buff,unsigned int nFileLen){
   unsigned char SendBuf[256];
   bzero(SendBuf,256);

   int iNum = nFileLen % SEND_MAX_SIZE;
   unsigned int Prom_Flash_Max;
   if (iNum == 0)
   {
		Prom_Flash_Max = nFileLen / SEND_MAX_SIZE;
   } else
   {
		Prom_Flash_Max = nFileLen / SEND_MAX_SIZE + 1;
   }

   SendBuf[0] =0x45;
   SendBuf[1] =0x00;
   SendBuf[2] =0x00;
   SendBuf[3] =0x0C;
   SendBuf[4] =0x1A;
   SendBuf[5] =0x00;
   SendBuf[6] = (((Prom_Write_Address)>>24)&0xffff);
   SendBuf[7] = (((Prom_Write_Address)>>16)&0xffff);
   SendBuf[8] = (((Prom_Write_Address)>>8)&0xffff);
   SendBuf[9]	= ((Prom_Write_Address)&0xffff);
   SendBuf[10] = (((Prom_Flash_Max)>>8)&0xff);
   SendBuf[11]	= ((Prom_Flash_Max)&0xff);
   SendBuf[12] = (((nFileLen)>>24)&0xffff);
   SendBuf[13] = (((nFileLen)>>16)&0xffff);
   SendBuf[14] = (((nFileLen)>>8)&0xffff);
   SendBuf[15]	= ((nFileLen)&0xffff);
   unsigned int checksum=0;
   for(int i=4;i<16;i++)
   {
	   checksum+=SendBuf[i];
	}
   SendBuf[16] =checksum;
   memcpy(Buff,SendBuf,0x11);
   return 0x11;
}

