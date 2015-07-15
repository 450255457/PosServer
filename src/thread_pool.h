#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <unistd.h>
#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "locker.h"
#include "PosServer.h"
#include "public_function.h"
#include "DataBase.h"

typedef struct _thread_para
{
	int epollfd;
	int sockfd;
	char buf[BUFMAXSIZE];
}thread_para;

//�̳߳���
class threadpool{
public:
	
    threadpool( );
    ~threadpool();

	int threadpool_init(int thread_number = 10, int max_requests = 10000 );
	void threadpool_destroy();
	//������������������
    bool append( void* request );
private:
	static void* worker( void* arg );
	void run();

    //int m_thread_number;			//�̳߳��е��߳���
    int m_max_requests;				//�����������������������
    pthread_t* m_threads;			//�����̳߳ص�����,���СΪm_thread_number
    std::list< void* > m_workqueue;	//�������
    locker m_queuelocker;			//����������еĻ�����
    sem m_queuestat;				//�Ƿ���������Ҫ����
    bool m_stop;					//�Ƿ�����߳�
};

#endif
