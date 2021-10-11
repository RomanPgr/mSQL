#include <iostream>
#include <cctype>
#include <stdexcept>
#include <cstring>
#include <stack>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h> 
#include "socket.h"
#include "manager.h"
#include "class_table.h"

Manager *m;
Socket_p *socket1;

namespace lexer {
    int cur_lex_line{ 0 };
    int cur_lex_pos{ 0 };
    int cur_lex_end_pos{ 0 };
    enum lex_type_t cur_lex_type;
    std::string cur_lex_text;

    int c;
    void init()
    {
        c = socket1->get();
    }

    void next()
    {
        cur_lex_text.clear();
        cur_lex_pos = cur_lex_end_pos;
        enum state_t {
            H,

            H_ALL,
            H_AND,
            H_CREATE,
            H_DELETE,
            H_DROP,
            H_FROM,
            H_IN,
            H_INSERT,
            H_INTO,
            H_LIKE,
            H_LONG,
            H_NOT,
            H_OR,
            H_SELECT,
            H_SET,
            H_TABLE,
            H_TEXT,
            H_UPDATE,
            H_WHERE,

            H_NOT_EQUALLY,
            H_PERCENT,
            H_OPEN,
            H_CLOSE,
            H_STAR,
            H_PLUS,
            H_COMMA,
            H_SUBTRACT,
            H_DIVIDE,
            H_LESS,
            H_LESS_EQUALLY,
            H_EQUALLY,
            H_MORE,
            H_MORE_EQUALLY,

            H_LONG_TYPE,
            H_LINEP,
            H_LINE,
            H_NAME,

            H_OK,

           // H_X///////////////////
        } state = H;
        while (state != H_OK) {
            switch (state) {
            case H:
                if (std::isspace(c)) {
                    // stay in H
                } else if (c == '!') {
                    state = H_NOT_EQUALLY;
               // } else if (c == '@') {////////////////////////
                   // state = H_X; ////////////////
                } else if (c == '%') {
                    state = H_PERCENT;
                } else if (c == '(') {
                    state = H_OPEN;
                } else if (c == ')') {
                    state = H_CLOSE;
                } else if (c == '*') {
                    state = H_STAR;
                } else if (c == '+') {
                    state = H_PLUS;
                } else if (c == ',') {
                    state = H_COMMA;
                } else if (c == '-') {
                    state = H_SUBTRACT;
                } else if (c == '/') {
                    state = H_DIVIDE;
                } else if (c == '<') {
                    state = H_LESS;
                } else if (c == '=') {
                    state = H_EQUALLY;
                } else if (c == '>') {
                    state = H_MORE;
                } else if (std::isdigit(c)) {
                    state = H_LONG_TYPE;
                } else if (c == '_' || std::isalpha(c)) {
                    state = H_NAME;
                } else if (c == '\'') {
                    state = H_LINE;
                } else if (c == EOF) {
                    cur_lex_type = END;
                    state = H_OK;
                } else {
                    throw std::logic_error(
                        std::to_string(cur_lex_line) + ": " +
                        std::to_string(cur_lex_end_pos) + ": "
                        "unexpected character with code " + std::to_string(c));
                }
                break;

                /*************************/
            case H_NOT_EQUALLY:
                if (c == '='){
                    cur_lex_type = NOT_EQUALLY;
                    state = H_OK;
                }
                else {
                    throw std::logic_error(
                        std::to_string(cur_lex_line) + ": " +
                        std::to_string(cur_lex_end_pos) + ": "
                        "unexpected character with code " + std::to_string(c));
                }
                break;

            case H_PERCENT:
                cur_lex_type = PERCENT;
                state = H_OK;
                break;

        //    case H_X://///////////////////////
        //        cur_lex_type = END;///////////////////////
        //        state = H_OK;////////////////////////
        //        break;///////////////////////

            case H_OPEN:
                cur_lex_type = OPEN;
                state = H_OK;
                break;

            case H_CLOSE:
                cur_lex_type = CLOSE;
                state = H_OK;
                break;

            case H_STAR:
                cur_lex_type = STAR;
                state = H_OK;
                break;

            case H_COMMA:
                cur_lex_type = COMMA;
                state = H_OK;
                break;

            case H_PLUS:
                cur_lex_type = PLUS;
                state = H_OK;
                break;

            case H_SUBTRACT:
                cur_lex_type = SUBTRACT;
                state = H_OK;
                break;

            case H_DIVIDE:
                cur_lex_type = DIVIDE;
                state = H_OK;
                break;

            case H_LESS:
                if (c == '='){
                    state = H_LESS_EQUALLY;
                } else {
                    cur_lex_type = LESS;
                    state = H_OK;
                }
                break;

            case H_LESS_EQUALLY:
                cur_lex_type = LESS_EQUALLY;
                state = H_OK;
                break;

            case H_EQUALLY:
                cur_lex_type = EQUALLY;
                state = H_OK;
                break;

            case H_MORE:
                if (c == '='){
                    state = H_MORE_EQUALLY;
                } else {
                    cur_lex_type = MORE;
                    state = H_OK;
                }
                break;

            case H_MORE_EQUALLY:
                cur_lex_type = MORE_EQUALLY;
                state = H_OK;
                break;
            
            case H_LONG_TYPE:
                if (std::isdigit(c)) {
                    // stay in H_LONG_TYPE
                } else if (std::isspace(c) || c == EOF || c == '!' ||
                    c == '(' || c == ')' || c == '*' || c == '+' ||
                    c == ',' || c == '-' || c == '<' || c == '>'){
                    cur_lex_type = LONG_TYPE;
                    state = H_OK;
                }
                else {
                    throw std::logic_error(
                        std::to_string(cur_lex_line) + ": " +
                        std::to_string(cur_lex_end_pos) + ": "
                        "unexpected character with code " + std::to_string(c));
                }
                break;

            case H_LINE:
                if (std::iscntrl(c)){
                    throw std::logic_error(
                        std::to_string(cur_lex_line) + ": " +
                        std::to_string(cur_lex_end_pos) + ": "
                        "u1nexpected character with code " + std::to_string(c));
                }
                if (c == '\''){
                    state = H_LINEP;
                }
                break;

            case H_NAME:
                if (c == '_' || std::isalnum(c)){
                    // stay in H_NAME
                } else if (std::isspace(c) || c == EOF || c == '!' ||
                    c == '(' || c == ')' || c == '*' || c == '+' ||
                    c == ',' || c == '-' || c == '<' || c == '>'){
                    cur_lex_type = NAME;
                    state = H_OK;
                }
                else {
                    throw std::logic_error(
                        std::to_string(cur_lex_line) + ": " +
                        std::to_string(cur_lex_end_pos) + ": "
                        "unexpected character with code " + std::to_string(c));
                }
                break;

            case H_LINEP:
                cur_lex_type = LINE;
                cur_lex_text.pop_back();
                cur_lex_text.erase(0, 1);
                state = H_OK;
                break;

            case H_OK:
                break;
            }

            if (state != H_OK) {
                if (c == '\n') {
                    ++cur_lex_line;
                    cur_lex_pos = cur_lex_end_pos = 0;
                }
                else if (std::isspace(c)) {
                    ++cur_lex_pos;
                    ++cur_lex_end_pos;
                }
                else {
                    ++cur_lex_end_pos;
                }

                if (!std::isspace(c) && cur_lex_type != END) {
                    cur_lex_text.push_back(c);
                }

                c = socket1->get();
            }
            else if (cur_lex_type == NAME){
                if (cur_lex_text == "ALL"){
                    cur_lex_type = ALL;
                }
                else if (cur_lex_text == "AND"){
                    cur_lex_type = AND;
                }
                else if (cur_lex_text == "CREATE"){
                    cur_lex_type = CREATE;
                }
                else if (cur_lex_text == "DELETE"){
                    cur_lex_type = DELETE;
                }
                else if (cur_lex_text == "DROP"){
                    cur_lex_type = DROP;
                }
                else if (cur_lex_text == "FROM"){
                    cur_lex_type = FROM;
                }
                else if (cur_lex_text == "IN"){
                    cur_lex_type = IN;
                }
                else if (cur_lex_text == "INSERT"){
                    cur_lex_type = INSERT;
                }
                else if (cur_lex_text == "INTO"){
                    cur_lex_type = INTO;
                }
                else if (cur_lex_text == "LIKE"){
                    cur_lex_type = LIKE;
                }
                else if (cur_lex_text == "LONG"){
                    cur_lex_type = LONG;
                }
                else if (cur_lex_text == "NOT"){
                    cur_lex_type = NOT;
                }
                else if (cur_lex_text == "OR"){
                    cur_lex_type = OR;
                }
                else if (cur_lex_text == "SELECT"){
                    cur_lex_type = SELECT;
                }
                else if (cur_lex_text == "SET"){
                    cur_lex_type = SET;
                }
                else if (cur_lex_text == "TABLE"){
                    cur_lex_type = TABLE;
                }
                else if (cur_lex_text == "TEXT"){
                    cur_lex_type = TEXT;
                }
                else if (cur_lex_text == "UPDATE"){
                    cur_lex_type = UPDATE;
                }
                else if (cur_lex_text == "WHERE"){
                    cur_lex_type = WHERE;
                }
            }
        }

    }
}

namespace loop_parser{
    void init(){
        lexer::init();
        lexer::next();
    }
    lex_type_t sql_pred();

    void select_prod();
    void select_prod2();
    void select_prod3();

    void insert_prod();
    void insert_prod2();
    void insert_prod3();

    void update_prod();
    void update_prod2();
    void update_prod3();
    void update_prod4();
    void update_prod5();

    void delete_prod();
    void delete_prod2();

    void create_prod();
    void create_prod2();
    void create_prod3();
    void create_prod4();
    void create_prod5();
    void create_prod6();
    void create_prod7();
    void create_prod8();
    void create_prod9();

    void drop_prod();

    void wirazh();
    void slag();
    void dop_slag();
    void mnozh();
    void dop_mnozh();

    void where_klausa();
    void where_klausa2();
    void where_klausa3();
    void where_klausa4();
    void where_klausa5();

    void log_wirazh();
    void log_slag();
    void log_dop_slag();
    void log_mnozh();
    void log_dop_mnozh();

    void otnosh();
    void spis_const();
    void spis_const2();
    void spis_const3();

    lex_type_t sql_pred(){
        if (lexer::cur_lex_type == SELECT){
            lexer::next();
            select_prod();
            return(SELECT);
        }
        else if (lexer::cur_lex_type == INSERT){
            lexer::next();
            insert_prod();
            return(INSERT);
        }
        else if (lexer::cur_lex_type == UPDATE){
            lexer::next();
            update_prod();
            return(UPDATE);
        }
        else if (lexer::cur_lex_type == DELETE){
            lexer::next();
            delete_prod();
            return(DELETE);
        }
        else if (lexer::cur_lex_type == CREATE){
            lexer::next();
            create_prod();
            return(CREATE);
        }
        else if (lexer::cur_lex_type == DROP){
            lexer::next();
            drop_prod();
            return(DROP);
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void select_prod(){
        if (lexer::cur_lex_type == NAME){
            m->names_select_field.push_back(lexer::cur_lex_text);
            lexer::next();
            select_prod2();
        } else if (lexer::cur_lex_type == STAR){
            m->select_all = true;
            lexer::next();
            if (lexer::cur_lex_type == FROM){
                lexer::next();
                select_prod3();
            } else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void select_prod2(){
        if (lexer::cur_lex_type == COMMA){
            lexer::next();
            select_prod();
        }
        else if (lexer::cur_lex_type == FROM){
            lexer::next();
            select_prod3();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void select_prod3(){
        if (lexer::cur_lex_type == NAME){
            m->name_table = lexer::cur_lex_text;
            lexer::next();
            where_klausa();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void insert_prod(){
        if (lexer::cur_lex_type == INTO){
            lexer::next();
            insert_prod2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void insert_prod2(){
        if (lexer::cur_lex_type == NAME){
            m->name_table = lexer::cur_lex_text;
            lexer::next();
            insert_prod3();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void insert_prod3(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            spis_const();
            if (lexer::cur_lex_type == CLOSE){
                lexer::next();
            } else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void update_prod(){
        if (lexer::cur_lex_type == NAME){
            m->name_table = lexer::cur_lex_text;
            lexer::next();
            update_prod2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void update_prod2(){
        if (lexer::cur_lex_type == SET){
            lexer::next();
            update_prod3();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void update_prod3(){
        if (lexer::cur_lex_type == NAME){
            m->field_name = lexer::cur_lex_text;
            lexer::next();
            update_prod4();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void update_prod4(){
        if (lexer::cur_lex_type == EQUALLY){
            lexer::next();
            wirazh();
            while (!m->log_expression_stack_r.empty()){
                m->expression_for_update.push(m->log_expression_stack_r.top());
                m->log_expression_stack_r.pop();
                m->expression_types_for_update.push(m->log_expression_types_r.top());
                m->log_expression_types_r.pop();
            }
            where_klausa();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void delete_prod(){
		if (lexer::cur_lex_type == FROM){
            lexer::next();
            delete_prod2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void delete_prod2(){
		if (lexer::cur_lex_type == NAME){
            m->name_table = lexer::cur_lex_text;
            lexer::next();
            where_klausa();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod(){
        if (lexer::cur_lex_type == TABLE){
            lexer::next();
            create_prod2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod2(){
        if (lexer::cur_lex_type == NAME){
            m->name_table = lexer::cur_lex_text;
            lexer::next();
            create_prod3();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod3(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            create_prod4();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod4(){
        if (lexer::cur_lex_type == NAME){
            m->descr_fields_names.push_back(lexer::cur_lex_text);
            lexer::next();
            create_prod5();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod5(){
        if (lexer::cur_lex_type == TEXT){
            m->descr_fields_types.push_back(LINE);
            lexer::next();
            create_prod6();
        }
        else if (lexer::cur_lex_type == LONG){
            m->descr_fields_types.push_back(LONG_TYPE);
            m->descr_fields_sizes.push_back(sizeof(unsigned long));
            lexer::next();
            create_prod9();
        }
        else{
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod6(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            create_prod7();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod7(){
        if (lexer::cur_lex_type == LONG_TYPE){
            m->descr_fields_sizes.push_back(std::stoul(lexer::cur_lex_text));
            lexer::next();
            create_prod8();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod8(){
        if (lexer::cur_lex_type == CLOSE){
            lexer::next();
            create_prod9();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void create_prod9(){
        if (lexer::cur_lex_type == COMMA){
            lexer::next();
            create_prod4();
        }
        else if (lexer::cur_lex_type == CLOSE){
            lexer::next();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void drop_prod(){
        if (lexer::cur_lex_type == TABLE){
            lexer::next();
            if (lexer::cur_lex_type == NAME){
                m->name_table = lexer::cur_lex_text;
                lexer::next();
            }
            else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void wirazh(){
        slag();
        dop_slag();
    }

    void dop_slag(){
        if (lexer::cur_lex_type == PLUS){
			std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            slag();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == SUBTRACT){
			std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            slag();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
    }

    void slag(){
        mnozh();
        dop_mnozh();
    }

    void dop_mnozh(){
        if (lexer::cur_lex_type == STAR){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            mnozh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == DIVIDE){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            mnozh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == PERCENT){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            mnozh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
    }

    void mnozh(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            wirazh();
            if (lexer::cur_lex_type == CLOSE){
                lexer::next();
            } else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        } else if (lexer::cur_lex_type == LONG_TYPE){
            m->log_expression_stack_r.push(lexer::cur_lex_text);
            m->log_expression_types_r.push(lexer::cur_lex_type);
            lexer::next();
        } else if (lexer::cur_lex_type == LINE){
            m->log_expression_stack_r.push(lexer::cur_lex_text);
            m->log_expression_types_r.push(lexer::cur_lex_type);
            lexer::next();
        } else if (lexer::cur_lex_type == NAME){
            m->log_expression_stack_r.push(lexer::cur_lex_text);
            m->log_expression_types_r.push(lexer::cur_lex_type);
            lexer::next();
        } else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void log_wirazh(){
        log_slag();
        log_dop_slag();
    }

    void log_dop_slag(){
        if (lexer::cur_lex_type == OR){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            log_slag();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
    }

    void log_slag(){
        log_mnozh();
        log_dop_mnozh();
    }

    void log_dop_mnozh(){
        if (lexer::cur_lex_type == AND){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            log_mnozh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
    }

    void log_mnozh(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            log_wirazh();
            if (lexer::cur_lex_type == CLOSE){
                lexer::next();
            }
            else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else if (lexer::cur_lex_type == NOT){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            log_mnozh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);

        } else {
            otnosh();
        }
    }

    void otnosh(){
        wirazh();
        if (lexer::cur_lex_type == NOT_EQUALLY){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == EQUALLY){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == LESS){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == LESS_EQUALLY){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == MORE){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        else if (lexer::cur_lex_type == MORE_EQUALLY){
            std::string s = lexer::cur_lex_text;
            lex_type_t a = lexer::cur_lex_type;
            lexer::next();
            wirazh();
            m->log_expression_stack_r.push(s);
            m->log_expression_types_r.push(a);
        }
        
    }
   
    void where_klausa(){
		if (lexer::cur_lex_type == WHERE){
            lexer::next();
            where_klausa2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void where_klausa2(){
        if (lexer::cur_lex_type == ALL){
			m->where_all = true;
            lexer::next();
        }
        else {
            log_wirazh();
            where_klausa3();
        }
    }

    void where_klausa3(){
        if (lexer::cur_lex_type == NOT){
			m->where_not = true;
            lexer::next();
            where_klausa4();
        }
        else if (lexer::cur_lex_type == IN){
			m->where_in = true;
            lexer::next();
            where_klausa5();
        }
        else if (lexer::cur_lex_type == LIKE){
			m->where_like = true;
            lexer::next();
            if (lexer::cur_lex_type == LINE){
				m->like_string = lexer::cur_lex_text;
                lexer::next();
            }
            else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else if (lexer::cur_lex_type == END){
			m->where_logic = true;
		} else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void where_klausa4(){
        if (lexer::cur_lex_type == IN){
			m->where_in = true;
            lexer::next();
            where_klausa5();
        }
        else if (lexer::cur_lex_type == LIKE){
			m->where_like = true;
            lexer::next();
            if (lexer::cur_lex_type == LINE){
				m->like_string = lexer::cur_lex_text;
                lexer::next();
            } else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void where_klausa5(){
        if (lexer::cur_lex_type == OPEN){
            lexer::next();
            spis_const();
            if (lexer::cur_lex_type == CLOSE){
                lexer::next();
            } else {
                throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
            }
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void spis_const(){
        m->fields_types.push_back(lexer::cur_lex_type);
        m->fields_value.push_back(lexer::cur_lex_text);
        if (lexer::cur_lex_type == LONG_TYPE){
            lexer::next();
            spis_const2();
        }
        else if (lexer::cur_lex_type == LINE){
            lexer::next();
            spis_const2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

    void spis_const2(){
        if (lexer::cur_lex_type == COMMA){
            lexer::next();
            spis_const3();
        }
        else if (lexer::cur_lex_type == CLOSE){
            //lexer::next();
        } else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }

  void spis_const3(){
        m->fields_types.push_back(lexer::cur_lex_type);
        m->fields_value.push_back(lexer::cur_lex_text);
        if (lexer::cur_lex_type == LONG_TYPE){
            lexer::next();
            spis_const2();
        }
        else if (lexer::cur_lex_type == LINE){
            lexer::next();
            spis_const2();
        }
        else {
            throw std::logic_error(
                        std::to_string(lexer::cur_lex_line) + ": " +
                        std::to_string(lexer::cur_lex_pos) + ": "
                        "unexpected token got <<" + lexer::cur_lex_text + ">>");
        }
    }


}


int main()
{
	Socket_p skt('s');
	socket1 = &skt;
	
	m = new Manager();
	lex_type_t  tp;
	try{
		loop_parser::init();
		tp = loop_parser::sql_pred();
		Table t(m, &skt);
		switch (tp){
		case (SELECT) :
			t.select();
			break;

		case (INSERT) :
			t.insert();
			break;

		case (UPDATE) :
			t.update();
			break;

		case (CREATE) :
			t.create();
			break;

		case (DROP) :
			t.drop();
			break;

		case (DELETE) :
			t.delete_t();
			break;
		}
		t.print();
		
	}
	catch (std::exception &e){
		while (skt.get() != EOF){;}
		std::cout << e.what();
		skt.write(e.what());
	}
	skt.write(EOF);
	delete m;
}
