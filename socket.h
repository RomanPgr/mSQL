#ifndef SOCKET
#define SOCKET
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h> 
#include <stdexcept>
#include <unistd.h>
#include <cstring>
#define ADDRESS "mysocket"

class Socket_p{
private:
	FILE *takef;
	int sendf;
public:
	Socket_p(char mode);
	~Socket_p();
	int write(const std::string str);
	inline int write(const char *ch, const int len){ return send(sendf, ch, len, 0);}
	inline int write(const char ch){ return send(sendf, &ch, 1, 0);}
	inline char get(){ return fgetc(takef);}
};
#endif
