#ifndef CLASS_TABLE
#define CLASS_TABLE
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <stack>
#include "manager.h"
#include "socket.h"

#define NAME_LENGTH 32
#define NUMBER_RECORD 1000
#define NUMBER_FIELD 100
#define LENGTH_FIELD 1024

class Table{
private:
    Manager *manag;
    bool error;
    bool err_types = false;
    FILE *temp_file;
    Socket_p *socket; 
    
    bool where();
    int calculate_expression();
    bool like_comparison(std::string arg1, std::string arg2);
    
    static char* strcncpy(char ch[], const std::string str, const unsigned int len){
        for (unsigned int i = 0; i < len && i < str.length(); ++i){
            ch[i] = str[i];
        }
        for (unsigned int i = str.length(); i < len; ++i){
            ch[i] = '\0';
        }
        return ch;
    }
public:

    Table(Manager *m, Socket_p *s);
    ~Table();

    int print();
    int select();
    int delete_t();
    int update();
    int create();
    int insert();
    int drop();
};
#endif
