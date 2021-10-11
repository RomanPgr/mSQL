#include "socket.h"

Socket_p::Socket_p(char mode){
	
	if (mode == 's'){
		int d, len, ca_len;
		struct sockaddr_un sa, ca;
		if((d = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) { 
			throw std::logic_error("The socket is not opened");
		}
		sa.sun_family = AF_UNIX;
		std::strcpy(sa.sun_path, ADDRESS);
		unlink(ADDRESS);
		len = sizeof(sa.sun_family) + strlen(sa.sun_path);
		if (bind(d, (struct sockaddr *)&sa, len) < 0 ) {
			throw std::logic_error("The socket is not opened");
		} 
		if (listen (d, 1) < 0 ) {
			throw std::logic_error("The socket is not opened");
		}
		ca_len = sizeof ca; 
		if ((sendf = accept(d, (struct sockaddr *)&ca, (socklen_t *) &ca_len)) < 0 ) {
			throw std::logic_error("The socket is not opened");
		} 
		takef = fdopen(sendf, "r"); 
	} else {
		 int len;
		 struct sockaddr_un sa;
		
		 if ((sendf = socket (AF_UNIX, SOCK_STREAM, 0)) < 0){
			 throw std::logic_error("Failed to connect to the socket");
		 }
		 
		 sa.sun_family = AF_UNIX;
		 strcpy(sa.sun_path, ADDRESS);
		 len = sizeof(sa.sun_family) + strlen(sa.sun_path); 
		
		 if (connect(sendf, (struct sockaddr *)&sa, len) < 0 ){ 
			throw std::logic_error("Failed to connect to the socket");
		 } 
		 takef = fdopen (sendf, "r");		
	}
}

Socket_p::~Socket_p(){
	fclose(takef);
	close(sendf);
	unlink(ADDRESS);
}
int Socket_p::write(const std::string str){
	char ch[str.length()];
	std::strncpy(ch, str.c_str(), str.length());
	return send(sendf, ch, str.length(), 0);
}
