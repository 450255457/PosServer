//============================================================================
// Name        : PosServer.cpp
// Author      : linden
// Version     :
// Copyright   : Your copyright notice
// Description : PosServer in C++, Ansi-style
//============================================================================

#include "PosServer.h"
#include "public_function.h"

using namespace std;

extern file_info filelist[FILEMAX];		//�����ļ��ĸ���
extern long Prom_Write_Address;

threadpool my_threadpool;

CPosServer::CPosServer()  
{  

}  
  
CPosServer::~CPosServer()  
{  
 
} 

/************************************************
��������:socket_bind
��������:�����׽��ֲ����а�
�������:const char *ip
		int nPort	�˿�
�������:
����˵��:socketfd/-1
*************************************************/
 int CPosServer::socket_bind(const char *ip,int nPort){
	int socketfd;
	struct sockaddr_in serveraddr;
	socketfd = socket(AF_INET,SOCK_STREAM,0);	//protocolͨ��Ϊ0����ʾ������������׽�������ѡ��Ĭ��Э��
	if(socketfd == -1){
		cout << "Error->socket:" << strerror(errno) << endl;
		return -1;
	}
	bzero(&serveraddr,sizeof(serveraddr));		//��һ���ڴ���������ȫ������Ϊ0
	serveraddr.sin_family = AF_INET;
	//inet_pton�����ʮ����IPת��Ϊ����
	inet_pton(AF_INET,ip,&serveraddr.sin_addr);
	serveraddr.sin_port = htons(nPort);
	
	//���÷���ʱ�޺ͽ���ʱ��
	struct timeval timeout = {100,0};	//100s
	setsockopt(socketfd,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));
	setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	//�����ַ����������	
	int opt = 1;
	if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0){
		printf("setsockopt:%s\n",strerror(errno));
		return -1;
	}
	
	if(bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr)) == -1){
		perror("bind error:");
		return -1;
	}
	return socketfd;
}

#if 0
/************************************************
��������:ev_select
��������:IO��·����select
�������:int socketfd
�������:
����˵��:0/-1
*************************************************/
int CPosServer::ev_select(int socketfd){
	int i = 0;
	int connfd;
	struct sockaddr_in client_addr;
	socklen_t client_size;
	client_size = sizeof(client_addr);
	int sockfd[BACKLOG] = {0};	//accepted connectionfd
	int conn_amount = 0;		//current connection amount
	fd_set rfds;	//�ļ����������Ķ���
	int maxsock;
	struct timeval tv;
	maxsock = socketfd;
	while(1){
		FD_ZERO(&rfds);				//��rfds����,ʹ�����в����κ�fd	
		FD_SET(socketfd,&rfds);		//��listenfd����rfds����
		//timeout setting
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		for(i = 0; i < BACKLOG; i++){
			if(sockfd[i] != 0){
				FD_SET(sockfd[i],&rfds);
			}
		}
		int ret;
		ret = select(maxsock + 1,&rfds,NULL,NULL,&tv);
		if(ret < 0){
			//��ʾ̽��ʧ��
			perror("select");
			break;
		}
		else if(ret == 0){
			//��ʾ��ʱ����һ��ѭ��
			printf("timeout!\n");
			continue;
		}
		for(i = 0; i < conn_amount; i++){
			if(FD_ISSET(sockfd[i],&rfds)){
				//�������Ƿ���rfds������
				char recvBuf[BUFMAXSIZE] = {0};
				char sendBuf[BUFMAXSIZE] = {0};
				ret = recv(sockfd[i],recvBuf,sizeof(recvBuf),0);
				if(ret <= 0){
					//�����ͻ��˹ر����ӣ�������Ҳ�ر���Ӧ����,���������׽��ִ��ļ��������������
					printf("client[%d] close\n",i);
					close(sockfd[i]);
					FD_CLR(sockfd[i],&rfds);
					sockfd[i] = 0;
					conn_amount--;
				}
				else{
					//�ͻ��������ݷ��͹���������Ӧ���ܴ���
					if(ret < BUFMAXSIZE-1){
						memset(&recvBuf[ret],'\0',1);
					}
					printf("client[%d] send %s\n",i,recvBuf);
					pthread_t ntid;
					thread_para arg;
					arg.fd = sockfd[i];
					memcpy(arg.buf,recvBuf,BUFMAXSIZE);
					int iret = pthread_create(&ntid,NULL,recv_thr_fn,(void *)&arg);
					if(iret != 0){
						cout << "can't create thread:" << strerror(errno) << endl;
					}
					#if 0
					//�̴߳���
					int err = 0;
					char *arg = recvBuf;
					pthread_t ntid;
					err = pthread_create(&ntid,NULL,recv_thr_fn,(void *)arg);
					if(err != 0){
						printf("can't create thread:%s\n",strerror(err));
					}
					err = pthread_join(ntid,(void *)&arg);
					if(err != 0){
						printf("�̷߳���ֵ���ִ���:%s\n",strerror(err));
					}
					char *pSendBuf = NULL;
					pSendBuf = arg;
					//printf("pSendBuf = %s\n",pSendBuf);
					int j = 0;
					while(j != 2048){
						sendBuf[j] = *pSendBuf;
						j++;
						pSendBuf++;
					}
					sendBuf[j] = '\0';
					ret = send(sockfd[i],sendBuf,2048,0);
					printf("ret = %d,i = %d\n",ret,i);
					#endif
				}
			}
		}
		//check whether a new connection comes
		if(FD_ISSET(socketfd,&rfds)){
			connfd = accept(socketfd,(struct sockaddr *)&client_addr,&client_size);
			if(connfd <= 0){
				perror("accept");
				continue;
			}
			if(conn_amount < BACKLOG){
				//sockfd[conn_amount++] = connfd;
				for(i = 0; i < BACKLOG; i++){
					if(sockfd[i] == 0){
						sockfd[i] = connfd;
						break;
					}
				}
				conn_amount++;
				printf("new connection client[%d] %s:%d\n",conn_amount,inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
				if(connfd > maxsock){
					maxsock = connfd;
				}		
			}		
			else{		
				printf("max connections arrive,exit\n");	
				send(connfd,"bye",4,0);			
				close(connfd);		
				break;		
			}
		}
	}
	return 0;
}
#endif

/************************************************
��������:do_epoll
��������:IO��·����epoll
�������:int socketfd
�������:
����˵��:0/-1
*************************************************/
int CPosServer::do_epoll(int socketfd){
	int epollfd;
	int nfds;
	struct epoll_event events[EPOLLEVENTS];		//�ں���Ҫ��������
	char buf[BUFMAXSIZE];
	memset(buf,0,BUFMAXSIZE);
	epollfd = epoll_create(FDSIZE);
	addfd(epollfd,socketfd,false);
	my_threadpool.threadpool_init();
//	add_event(epollfd,socketfd,EPOLLIN);
	while(1){
		nfds = epoll_wait(epollfd,events,EPOLLEVENTS,-1);	//�ȴ�I/O�¼�
		if(nfds == 0){
			cout << "PosServer is running..." << endl;
			continue;
		}
		else if((nfds < 0) && (errno != EINTR)){			//epoll����EINTR(Interrupted system call)
			cout << "Error->epoll_wait:" << strerror(errno) << endl;
			break;
		}
		edge_trigger( events, nfds, epollfd, socketfd );	//ʹ��ETģʽ
	}
	my_threadpool.threadpool_destroy();
	close(socketfd);
	close(epollfd);
	return 0;
}

//���ļ����������óɷ�������
int CPosServer::setnonblocking(int fd){
	int old_option = fcntl( fd, F_GETFL );
	int new_option = old_option | O_NONBLOCK; 
	fcntl( fd, F_SETFL, new_option );  
	return old_option;
}
 


//��fd�ϵ�EPOLLIN��EPOLLET�¼�ע�ᵽepollfdָʾ��epoll�ں��¼����У�����oneshotָ���Ƿ�ע��fd�ϵ�EPOLLONESHOT�¼�
void CPosServer::addfd(int epollfd,int fd,bool oneshot){
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if(oneshot){
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

//����fd�ϵ��¼�����������֮�󣬾���fd�ϵ�EPOLLONESHOT�¼���ע�ᣬ���ǲ���ϵͳ��Ȼ�ᴥ��fd�ϵ�EPOLLIN�¼�����ֻ����һ��
void CPosServer::reset_oneshot(int epollfd,int fd){
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

//ETģʽ�Ĺ�������
void CPosServer::edge_trigger( epoll_event* events, int nfds, int epollfd, int socketfd ){
	char buf[ BUFMAXSIZE ];   
	for ( int i = 0; i < nfds; i++ ){  
		int sockfd = events[i].data.fd;    
		if ( sockfd == socketfd ){      
			struct sockaddr_in client_address;   
			socklen_t client_addrlength = sizeof( client_address );   
			int connfd = accept( socketfd, (struct sockaddr*)&client_address, &client_addrlength );
			addfd(epollfd, connfd, true);
			//add_event( epollfd, connfd, true );	//��connfd����ETģʽ
			//cout << "accept a new sockfd:" << connfd << endl;
		}  
		else if ( events[i].events & EPOLLIN ){
			m_arglocker.lock();
			thread_para arg;
			arg.epollfd = epollfd;
			arg.sockfd = sockfd;
			m_arglocker.unlock();
			my_threadpool.append(&arg);
			
			#if 0
			pthread_t ntid;
			pthread_attr_t attr;
			thread_para arg;
			arg.epollfd = epollfd;
			arg.fd = sockfd;
			//��ʼ������ֵ������ΪĬ��ֵ
            pthread_attr_init(&attr); 
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
			//�����߳�Ϊ��������
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			int iret = pthread_create(&ntid,&attr,recv_thr_fn,(void *)&arg);
			if(iret != 0){
				cout << "can't create thread:" << strerror(errno) << endl;
			}
			#endif
		}       
		else{
			printf( "Error:something else happened \n" );     
		}
	}
}

//���ļ�������fd�ϵ�EPOLLINע�ᵽepollfdָʾ��epoll�ں��¼�����,����enable_etָ���Ƿ��fd����ETģʽ
void CPosServer::add_event( int epollfd, int fd, bool enable_et ){
	epoll_event event;  
	event.data.fd = fd;   
	event.events = EPOLLIN;  
	if( enable_et ){ 
		event.events |= EPOLLET;
	}  
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );		//ע���µ�fd��epfd��
	setnonblocking( fd );
}
