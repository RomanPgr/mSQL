#ifndef MANAGER
#define MANAGER

enum lex_type_t{
    ALL,
    AND,
    CREATE,
    DELETE,
    DROP,
    FROM,
    IN,
    INSERT,
    INTO,
    LIKE,
    LONG,
    NOT,
    OR,
    SELECT,
    SET,
    TABLE,
    TEXT,
    UPDATE,
    WHERE,

    NOT_EQUALLY,
    PERCENT,
    OPEN,
    CLOSE,
    STAR,
    PLUS,
    COMMA,
    SUBTRACT,
    DIVIDE,
    LESS,
    LESS_EQUALLY,
    EQUALLY,
    MORE,
    MORE_EQUALLY,

    LONG_TYPE,
    LINE,
    NAME,
    BOOLEAN,

    END
};



struct Manager{
    //for SQL +
    std::string name_table;

    //for CREATE +
    std::vector <lex_type_t> descr_fields_types;
    std::vector <int> descr_fields_sizes;
    std::vector <std::string> descr_fields_names;
    
    //for INSERT+
    std::vector <lex_type_t> fields_types;
    std::vector <std::string> fields_value;

    //for SELECT
    bool select_all;
    std::vector <std::string> names_select_field;

    //for UPDATE
    std::string field_name;
    std::stack <std::string> expression_for_update;
    std::stack <lex_type_t> expression_types_for_update;

    //for WHERE
    bool where_all;
    bool where_logic;
    bool where_in;
    bool where_like;
    bool where_not;
    std::string like_string;

    std::stack <std::string> log_expression_stack_r; //не изменяется
    std::stack <lex_type_t> log_expression_types_r;

    std::stack <std::string> log_expression_stack;
    std::stack <lex_type_t> log_expression_types;

    std::vector <std::string> log_expression_stack_temp;
    std::vector <lex_type_t> log_expression_types_temp;

    std::vector <std::string> expression_for_update_temp;
    std::vector <lex_type_t> expression_types_for_update_temp;

    std::vector <lex_type_t> types; //для текущей строки
    std::vector <std::string> names;
    std::vector <std::string> fields; 

    Manager(){
        select_all = false;
        bool where_all = false;
        bool where_logic = false;
        bool where_in = false;
        bool where_like = false;
        bool where_not = false;
    }
};

#endif