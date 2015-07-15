/*************************************************************************
	> File Name: main.cpp
	> Author: linden
	> Mail: 450255457@qq.com 
	> Created Time: 2015年03月29日 星期四 10时30分31秒
 ************************************************************************/

#include <unistd.h>
#include <iostream>
#include <mysql/mysql.h> 

#include "PosServer.h"
#include "DataBase.h"
#include "public_function.h"

using namespace std;

int main(int argc,char *argv[]){
	CPublicFunction MyPF;
	CDatabase MyDB;
	MyDB.initDB("localhost", "root", "123456", "InfoData");
	//MyDB.executeSQL("select * from t_file_info");
	MyDB.init_file_info();
	
	cout << "PosServer is running..." << endl;
	CPosServer PS;
	int socketfd;
	socketfd = PS.socket_bind(IPADDRESS,PORT);
	if(listen(socketfd,LISTENQ) < 0){
		cout << "Error->listen:" << strerror(errno) << endl;
		return -1;
	}
//	PS.ev_select(socketfd);
	PS.do_epoll(socketfd);
	return 0;
}
	
