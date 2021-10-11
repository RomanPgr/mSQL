#include "socket.h"
#include <iostream>
#include <stdexcept>

int main(){
	char c;
	try{
		Socket_p s('c');
		while (true){
			c = std::cin.get();
			s.write(&c, 1);
			if (c == EOF){
				c = s.get();
				while (c != EOF){
					std::cout << c;
					c = s.get();
				}
                break;
			}
		}
	}
	catch (std::exception &e){
		std::cout << e.what() << std::endl;
	}
}
