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

extern file_info filelist[FILEMAX];		//升级文件的个数
extern long Prom_Write_Address;

threadpool my_threadpool;

CPosServer::CPosServer()  
{  

}  
  
CPosServer::~CPosServer()  
{  
 
} 

/************************************************
函数名称:socket_bind
函数功能:创建套接字并进行绑定
输入参数:const char *ip
		int nPort	端口
输出参数:
返回说明:socketfd/-1
*************************************************/
 int CPosServer::socket_bind(const char *ip,int nPort){
	int socketfd;
	struct sockaddr_in serveraddr;
	socketfd = socket(AF_INET,SOCK_STREAM,0);	//protocol通常为0，表示按给定的域和套接字类型选择默认协议
	if(socketfd == -1){
		cout << "Error->socket:" << strerror(errno) << endl;
		return -1;
	}
	bzero(&serveraddr,sizeof(serveraddr));		//把一段内存区的内容全部设置为0
	serveraddr.sin_family = AF_INET;
	//inet_pton将点分十进制IP转换为整数
	inet_pton(AF_INET,ip,&serveraddr.sin_addr);
	serveraddr.sin_port = htons(nPort);
	
	//设置发送时限和接收时限
	struct timeval timeout = {100,0};	//100s
	setsockopt(socketfd,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(struct timeval));
	setsockopt(socketfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	//允许地址的立即重用	
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
函数名称:ev_select
函数功能:IO多路复用select
输入参数:int socketfd
输出参数:
返回说明:0/-1
*************************************************/
int CPosServer::ev_select(int socketfd){
	int i = 0;
	int connfd;
	struct sockaddr_in client_addr;
	socklen_t client_size;
	client_size = sizeof(client_addr);
	int sockfd[BACKLOG] = {0};	//accepted connectionfd
	int conn_amount = 0;		//current connection amount
	fd_set rfds;	//文件描述符集的定义
	int maxsock;
	struct timeval tv;
	maxsock = socketfd;
	while(1){
		FD_ZERO(&rfds);				//将rfds清零,使集合中不含任何fd	
		FD_SET(socketfd,&rfds);		//将listenfd加入rfds集合
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
			//表示探测失败
			perror("select");
			break;
		}
		else if(ret == 0){
			//表示超时，下一轮循环
			printf("timeout!\n");
			continue;
		}
		for(i = 0; i < conn_amount; i++){
			if(FD_ISSET(sockfd[i],&rfds)){
				//测试其是否在rfds集合中
				char recvBuf[BUFMAXSIZE] = {0};
				char sendBuf[BUFMAXSIZE] = {0};
				ret = recv(sockfd[i],recvBuf,sizeof(recvBuf),0);
				if(ret <= 0){
					//表明客户端关闭连接，服务器也关闭相应连接,并把连接套接字从文件描述符集中清除
					printf("client[%d] close\n",i);
					close(sockfd[i]);
					FD_CLR(sockfd[i],&rfds);
					sockfd[i] = 0;
					conn_amount--;
				}
				else{
					//客户端有数据发送过来，作相应接受处理
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
					//线程处理
					int err = 0;
					char *arg = recvBuf;
					pthread_t ntid;
					err = pthread_create(&ntid,NULL,recv_thr_fn,(void *)arg);
					if(err != 0){
						printf("can't create thread:%s\n",strerror(err));
					}
					err = pthread_join(ntid,(void *)&arg);
					if(err != 0){
						printf("线程返回值出现错误:%s\n",strerror(err));
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
函数名称:do_epoll
函数功能:IO多路复用epoll
输入参数:int socketfd
输出参数:
返回说明:0/-1
*************************************************/
int CPosServer::do_epoll(int socketfd){
	int epollfd;
	int nfds;
	struct epoll_event events[EPOLLEVENTS];		//内核需要监听的事
	char buf[BUFMAXSIZE];
	memset(buf,0,BUFMAXSIZE);
	epollfd = epoll_create(FDSIZE);
	addfd(epollfd,socketfd,false);
	my_threadpool.threadpool_init();
//	add_event(epollfd,socketfd,EPOLLIN);
	while(1){
		nfds = epoll_wait(epollfd,events,EPOLLEVENTS,-1);	//等待I/O事件
		if(nfds == 0){
			cout << "PosServer is running..." << endl;
			continue;
		}
		else if((nfds < 0) && (errno != EINTR)){			//epoll遭遇EINTR(Interrupted system call)
			cout << "Error->epoll_wait:" << strerror(errno) << endl;
			break;
		}
		edge_trigger( events, nfds, epollfd, socketfd );	//使用ET模式
	}
	my_threadpool.threadpool_destroy();
	close(socketfd);
	close(epollfd);
	return 0;
}

//将文件描述符设置成非阻塞的
int CPosServer::setnonblocking(int fd){
	int old_option = fcntl( fd, F_GETFL );
	int new_option = old_option | O_NONBLOCK; 
	fcntl( fd, F_SETFL, new_option );  
	return old_option;
}
 


//将fd上的EPOLLIN和EPOLLET事件注册到epollfd指示的epoll内核事件表中，参数oneshot指定是否注册fd上的EPOLLONESHOT事件
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

//重置fd上的事件，这样操作之后，尽管fd上的EPOLLONESHOT事件被注册，但是操作系统仍然会触发fd上的EPOLLIN事件，且只触发一次
void CPosServer::reset_oneshot(int epollfd,int fd){
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}

//ET模式的工作流程
void CPosServer::edge_trigger( epoll_event* events, int nfds, int epollfd, int socketfd ){
	char buf[ BUFMAXSIZE ];   
	for ( int i = 0; i < nfds; i++ ){  
		int sockfd = events[i].data.fd;    
		if ( sockfd == socketfd ){      
			struct sockaddr_in client_address;   
			socklen_t client_addrlength = sizeof( client_address );   
			int connfd = accept( socketfd, (struct sockaddr*)&client_address, &client_addrlength );
			addfd(epollfd, connfd, true);
			//add_event( epollfd, connfd, true );	//对connfd开启ET模式
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
			//初始化属性值，均设为默认值
            pthread_attr_init(&attr); 
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
			//设置线程为分离属性
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

//将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中,参数enable_et指定是否对fd启用ET模式
void CPosServer::add_event( int epollfd, int fd, bool enable_et ){
	epoll_event event;  
	event.data.fd = fd;   
	event.events = EPOLLIN;  
	if( enable_et ){ 
		event.events |= EPOLLET;
	}  
	epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );		//注册新的fd到epfd中
	setnonblocking( fd );
}
