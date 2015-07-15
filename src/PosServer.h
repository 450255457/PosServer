#ifndef _POSSERVER_H
#define _POSSERVER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>

#include "DataBase.h"
#include "locker.h"
#include "thread_pool.h"

#define IPADDRESS	"0.0.0.0"
#define PORT		8090		//12346
#define LISTENQ		5
#define FDSIZE		1000
#define EPOLLEVENTS	1000
#define BACKLOG 5		//���������

struct PACK_SEND_DATA {
	char Head[2];				//Ҫ���͵�����
	char Datalen[2];			//���ݴ�С
	char Cmd[2];				//״̬��
	char nPackNo[2];			//�����ֽڴ�С����
    char Data[SEND_MAX_LENGTH]; //���ݴ�С
};

struct PACK_SEND_REVICE_DATA {
	
	char Head[2];				//��ͷ
	char Datalen[2];			//���ݴ�С
	char Cmd[2]; 				//״̬��
	char CrcCmd[4]; 			//CRC32
	char nPackNo[2]; 			//�����ֽڴ�С����
	char nCheck[1]; 			//CHECKSUM
};

class CPosServer
{  
public:  
    CPosServer();  
    ~CPosServer();
	
	int socket_bind(const char *ip,int nPort);
	int do_epoll(int socketfd);
	void add_event( int epollfd, int fd, bool enable_et);
	void edge_trigger( epoll_event* events, int nfds, int epollfd, int socketfd );
	int setnonblocking( int fd );
	void addfd(int epollfd,int fd,bool oneshot);
	int ev_select(int socketfd);
	void reset_oneshot(int epollfd,int fd);
		
private:
	locker m_arglocker;			//������е���
};  

#endif
