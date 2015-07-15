
#ifndef _PUBLICFUNCTION_H
#define _PUBLICFUNCTION_H

#include <iostream>
#include <string.h>

#define BUFMAXSIZE	1300

using namespace std;

class CPublicFunction
{  
public:  
    CPublicFunction();  
    ~CPublicFunction();

	unsigned char get_bcc_data(unsigned char *data,int len);
	unsigned long GenerateCRC32(unsigned char *DataBuf,unsigned long len);
	void	AsciiStrToBcd(unsigned char *outbuf,unsigned char *b,int len);
	void	DecTo2Hex(int dec,char *data);
	
private:  
}; 

#endif
