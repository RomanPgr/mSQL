#include "class_table.h"

Table::Table(Manager *m, Socket_p *s){
		socket = s;
        manag = m;
        error = false;
        err_types = false;
        temp_file = NULL;
    }

Table::~Table(){
	if (temp_file != NULL){
		fclose(temp_file);
	}
}

int Table::drop(){
	if (std::remove(manag->name_table.c_str())){
		throw std::logic_error("The table is not found");
	} 
	return 0;
}

int Table::print(){
	if (temp_file != NULL){
		std::fseek(temp_file, 0, SEEK_SET);
		
		int len;
		int num;
		fread(&len, sizeof(len), 1, temp_file);
		fread(&num, sizeof(num), 1, temp_file);
		
		int *type_fields = new int[len];
		int *size_fields = new int[len];
		fread(type_fields, sizeof(int), len, temp_file);
		fread(size_fields, sizeof(int), len, temp_file);

		for (int i = 0; i < len; ++i){
			char ch[NAME_LENGTH + 1];
			ch[NAME_LENGTH] = '\0';
			std::fread(ch, sizeof(char), NAME_LENGTH, temp_file);
			socket->write(ch, NAME_LENGTH);
			socket->write('\t');
			std::cout << ch << "\t";
			//ѕ≈чать в какой-то поток
		}
		std::cout << '\n';
		socket->write('\n');
		for (int i = 0; i < num; ++i){
			for (int j = 0; j < len; ++j){
				if (type_fields[j] == LONG_TYPE){
					unsigned long l;
					std::fread(&l, sizeof(l), 1, temp_file);
					std::cout << l << "\t";
					socket->write(std::to_string(l));
					socket->write('\t');
					//ѕечать в какой-то поток
				}
				else {
					char *ch = new char[size_fields[j] + 1];
					ch[size_fields[j]] = '\0';
					std::fread(ch, sizeof(char), size_fields[j], temp_file);
					//ѕечать в какой-то поток
					socket->write(ch, size_fields[j]);
					socket->write('\t');
					std::cout << ch << "\t";
					delete[] ch;
				}
			}std::cout << "\n";
			//ѕеревод строки
			 socket->write('\n');
		}
			
		delete[] type_fields;
		delete[] size_fields;
		std::fclose(temp_file);
		temp_file = NULL;
	}
	else {	
		FILE *f = std::fopen(manag->name_table.c_str(), "rb");
		int len;
		int num;
		fread(&len, sizeof(len), 1, f);
		fread(&num, sizeof(num), 1, f);
		
		int *type_fields = new int[len];
		int *size_fields = new int[len];            
		fread(type_fields, sizeof(int), len, f);
		fread(size_fields, sizeof(int), len, f);		
		
		for (int i = 0; i < len; ++i){
			char ch[NAME_LENGTH + 1];
			ch[NAME_LENGTH] = '\0';
			std::fread(ch, sizeof(char), NAME_LENGTH, f);
			std::cout << ch << "\t";
			socket->write(ch, NAME_LENGTH);
			socket->write('\t');
			//ѕ≈чать в какой-то поток
		}
		std::cout << std::endl;
		socket->write('\n');
		for (int i = 0; i < num; ++i){
			for (int j = 0; j < len; ++j){
				if (type_fields[j] == LONG_TYPE){
					unsigned long l;
					std::fread(&l, sizeof(l), 1, f);
					std::cout << l << "\t";
					socket->write(std::to_string(l));
					socket->write('\t');
					//ѕечать в какой-то поток
				}
				else {
					char *ch = new char[size_fields[j] + 1];
					ch[size_fields[j]] = '\0';
					std::fread(ch, sizeof(char), size_fields[j], f);
					std::cout << ch << "\t";
					socket->write(ch, size_fields[j]);
					socket->write('\t');
					//ѕечать в какой-то поток
					delete[] ch;
				}
			}
			std::cout << std::endl;
			socket->write('\n');
			//ѕеревод строки
		}
		delete[] type_fields;
		delete[] size_fields;
		std::fclose(f);
	}
	return 0;
}
    
bool Table::where(){
	if (manag->where_all){
		return true;
	}
	if (manag->where_like && (manag->log_expression_types.size() != 1 || manag->log_expression_types.top() != NAME)){
		throw std::invalid_argument("Invalid like expression");
	}

	if (manag->where_like){
		for (unsigned int i = 0; i < manag->types.size(); ++i){
				if (manag->names[i] == manag->log_expression_stack.top()){
					if (manag->types[i] != LINE){
							throw std::invalid_argument("This field is not a string");
						}
					return(manag->where_not ^ like_comparison(manag->fields[i], manag->like_string));
				}
		}
		throw std::invalid_argument("Field not found");
		
	}
	calculate_expression();
	if (manag->where_in){
		for (unsigned int i = 0; i < manag->fields_types.size(); ++i){
			if (manag->log_expression_types.top() == manag->fields_types[i] &&
				manag->log_expression_stack.top() == manag->fields_value[i]){
				return (manag->where_not ^ true);
			}
		}
		return (manag->where_not ^ false);
	}
	if (manag->log_expression_types.top() == BOOLEAN){
		return std::stoi(manag->log_expression_stack.top());
	}
	throw std::invalid_argument("Invalid expression");
}

int Table::calculate_expression(){
	/*for (unsigned int i = 0; ! manag->log_expression_stack.empty(); ++i){
		std::cout << "size = " << manag->log_expression_stack.top() << std::endl;
		manag->log_expression_stack.pop() ;
	}
	throw std::logic_error(" ");*/
	
	if (manag->log_expression_stack.size() == 1){
		for (unsigned int i = 0; i < manag->fields.size(); ++i){
			if (manag->log_expression_stack.top() == manag->names[i]){
				manag->log_expression_stack.pop();
				manag->log_expression_types.pop();
				manag->log_expression_stack.push(manag->fields[i]);
				manag->log_expression_types.push(manag->types[i]);
				return 0;
			}
		}
		throw std::invalid_argument("Field not found");
	}
	while (!err_types && (manag->log_expression_stack.size() > 1)){
		std::string arg1 = manag->log_expression_stack.top();
		manag->log_expression_stack.pop();
		std::string arg2 = manag->log_expression_stack.top();
		manag->log_expression_stack.pop();
		manag->log_expression_stack.pop(); //“ак и надо 2 раза

		lex_type_t targ1 = manag->log_expression_types.top();
		manag->log_expression_types.pop();
		lex_type_t targ2 = manag->log_expression_types.top();
		manag->log_expression_types.pop();
		lex_type_t targ3 = manag->log_expression_types.top();
		manag->log_expression_types.pop();
		if (targ1 == NAME){
			for (unsigned int i = 0; i < manag->fields.size(); ++i){
				if (arg1 == manag->names[i]){
					arg1 = manag->fields[i];
					targ1 = manag->types[i];
					break;
				}
			}
			if (targ1 == NAME){
				throw std::invalid_argument("Field not found");
			}
		}
		if (targ2 == NAME){
			for (unsigned int i = 0; i < manag->fields.size(); ++i){
				if (arg2 == manag->names[i]){
					arg2 = manag->fields[i];
					targ2 = manag->types[i];
					break;
				}
			}
			if (targ2 == NAME){
				throw std::invalid_argument("Field not found");
			}
		}

		switch (targ3){
		case NOT_EQUALLY:
			manag->log_expression_stack.push(std::to_string((targ1 != targ2) || (arg1 != arg2)));
			manag->log_expression_types.push(BOOLEAN);
			break;
		case PERCENT:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) % std::stoul(arg2)));
				manag->log_expression_types.push(LONG_TYPE);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");;
			}
			break;
		case STAR:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) * std::stoul(arg2)));
				manag->log_expression_types.push(LONG_TYPE);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case PLUS:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) + std::stoul(arg2)));
				manag->log_expression_types.push(LONG_TYPE);
			}
			else if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(arg1 + arg2);
				manag->log_expression_types.push(LINE);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case SUBTRACT:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) - std::stoul(arg2)));
				manag->log_expression_types.push(LONG_TYPE);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case DIVIDE:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) / std::stoul(arg2)));
				manag->log_expression_types.push(LONG_TYPE);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case LESS:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) < std::stoul(arg2)));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case LESS_EQUALLY:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) <= std::stoul(arg2)));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case EQUALLY:
			manag->log_expression_stack.push(std::to_string((targ1 == targ2) && (arg1 == arg2)));
			manag->log_expression_types.push(BOOLEAN);
			break;
		case MORE:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) > std::stoul(arg2)));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case MORE_EQUALLY:
			if (targ1 == LONG_TYPE && targ2 == LONG_TYPE){
				manag->log_expression_stack.push(std::to_string(std::stoul(arg1) >= std::stoul(arg2)));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case AND:
			if (targ1 == BOOLEAN && targ2 == BOOLEAN){
				manag->log_expression_stack.push(std::to_string(1 & std::stoul(arg1) & std::stoul(arg2)));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case OR:
			if (targ1 == BOOLEAN && targ2 == BOOLEAN){
				manag->log_expression_stack.push(std::to_string(1 & (std::stoul(arg1) | std::stoul(arg2))));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		case NOT:
			if (targ1 == BOOLEAN){
				manag->log_expression_stack.push(std::to_string(!(1 & std::stoul(arg1))));
				manag->log_expression_types.push(BOOLEAN);
			}
			else {
				throw std::domain_error("The expression contains incompatible data types");
			}
			break;
		}
	}
	return false;
}


int Table::insert(){
	FILE *f = std::fopen(manag->name_table.c_str(), "rb+");
	if (f == NULL){
		throw std::logic_error("The table is not create");
	}
	int len;
	int num;
	fread(&len, sizeof(len), 1, f);
	if ((long)len != (long)manag->fields_types.size()){
		fclose(f);
		throw std::logic_error("In a row of " + std::to_string(len) + " fields, in the table " + std::to_string(manag->fields_types.size()));
	}
		
	fread(&num, sizeof(num), 1, f);
	if (num >= NUMBER_RECORD){
		fclose(f);
		throw std::logic_error("The table is complete and contains " + std::to_string(num) + " records");
	}
	int *type_field = new int[len];
	int *size_field = new int[len];
	fread(type_field, sizeof(int), len, f);
	fread(size_field, sizeof(int), len, f);

	for (int i = 0; i < len; ++i){
		if (manag->fields_types[i] != type_field[i]){
			fclose(f);
			delete[] type_field;
			delete[] size_field;
			throw std::logic_error("Mismatch of field types: " + manag->fields_value[i]);
		}
		if (type_field[i] == LINE && ((long)size_field[i] < (long)(manag->fields_value[i].length()))){
			fclose(f);
			delete[] type_field;
			delete[] size_field;
			throw std::logic_error("The size of field " + manag->fields_value[i] + " exceeds the allowed size");
		}
		if (type_field[i] == LONG_TYPE){
			try{
				std::stoul(manag->fields_value[i]);
			}
			catch (...){
				fclose(f);
				delete[] type_field;
				delete[] size_field;
				throw std::logic_error("Cannot be converted to unsigned long: " + manag->fields_value[i]);
			}
		}
	}
	std::fseek(f, 0, SEEK_END);

	for (int i = 0; i < len; i++){
		if (type_field[i] == LINE){
			char *ch = new char[size_field[i] + 1];
			ch[size_field[i]] = '\0';
			strcncpy(ch, manag->fields_value[i], size_field[i]);
			std::fwrite(ch, sizeof(char), size_field[i], f);
			delete[] ch;
		}
		if (type_field[i] == LONG_TYPE){
			unsigned long  q = std::stoul(manag->fields_value[i]);
			std::fwrite(&q, sizeof(q), 1, f);
		}
	}
	std::fseek(f, sizeof(int), SEEK_SET);
	++num;
	std::fwrite(&num, sizeof(num), 1, f);
	std::fclose(f);
	return 0;
}

int Table::create(){
	int len = manag->descr_fields_types.size();
	if (len > NUMBER_FIELD){
		error = true;
		throw std::logic_error("Too many fields: " + std::to_string(len));
		return -1;
	}
	for (int i = 0; i < len; ++i){
		if (manag->descr_fields_sizes[i] > LENGTH_FIELD){
			error = true;
			throw std::logic_error("Too long fields " + manag->descr_fields_names[i] + ": " + std::to_string(manag->descr_fields_sizes[i]) + " characters");
			return -1;
		}
		if (manag->descr_fields_names[i].length() > NAME_LENGTH){
			error = true;
			throw std::logic_error("Too long name " + manag->descr_fields_names[i] + ": " + manag->descr_fields_names[i] + " characters");
			return -1;
		}
	} 
	FILE *f = std::fopen(manag->name_table.c_str(), "wb");
	if (f == NULL){
		error = true;
		throw std::logic_error("The table is not create");
		return -1;
	} else {
		int num = 0;
		std::fwrite(&len, sizeof(len), 1, f);
		std::fwrite(&num, sizeof(num), 1, f);
		
		for (int i = 0; i < len; ++i){
			std::fwrite(&(manag->descr_fields_types[i]), sizeof(int), 1, f);
		}
		for (int i = 0; i < len; ++i){
			std::fwrite(&(manag->descr_fields_sizes[i]), sizeof(int), 1, f);
		}
		
		for (int i = 0; i < len; ++i){
			char ch[NAME_LENGTH + 1];
			ch[NAME_LENGTH] = '\0';
			strcncpy(ch, manag->descr_fields_names[i], NAME_LENGTH);
			std::fwrite(ch, sizeof(char), NAME_LENGTH, f);                
		}
		std::fclose(f);
	}
	return 0;
}


bool Table::like_comparison(std::string arg1, std::string arg2) {
	if (arg1.length() == 0){
		return arg2.length() == 0;
	}
	switch (arg1[0]){
	case '%':
		arg1.erase(0, 1);
		while (true){
			if (like_comparison(arg1, arg2)){
				return true;
			}
			if (arg2.length() == 0)
			{
				break;
			}
			arg2.erase(0, 1);
		}
		return false;
		break;

	case '_':
		if (arg2.length() > 0){
			arg1.erase(0, 1);
			arg2.erase(0, 1);
			return like_comparison(arg1, arg2);
		}
		else {
			return false;
		}
		break;
	case '[':
		if (arg1.length() > 1)
		{
			if (arg1[1] != '^'){
				if (arg1.length() < 4){
					throw std::invalid_argument("Invalid like string");
				}
				for (unsigned int i = 1; i < arg1.length(); ++i){
					if (arg1[i] == ']'){
						if (i == 2){
							throw std::invalid_argument("Invalid like string");
						} else {
							arg1.erase(0, i + 1);
							arg2.erase(0, 1);
							return like_comparison(arg1, arg2);
						}
					} else {
						if (arg1[i] == arg2[0]){
							return false;
						}
					}
				}
			}
			else {
				bool b = false;
				if (arg1.length() < 3){
					throw std::invalid_argument("Invalid like string");
				}
				for (unsigned int i = 1; i < arg1.length(); ++i){
					if (arg1[i] == ']'){
						if (i == 1){
							throw std::invalid_argument("Invalid like string");
						}
						else {
							if (b == false){
								return false;
							}
							arg1.erase(0, i + 1);
							arg2.erase(0, 1);
							return like_comparison(arg1, arg2);
						}
					}
					else {
						if (arg1[i] == arg2[0]){
							b = true;
						}
					}
				}
			}
		}
		break;

	default:
		if (arg2.length() > 0 && arg1[0] == arg2[0]){
			arg1.erase(0, 1);
			arg2.erase(0, 1);
			return like_comparison(arg1, arg2);
		} else {
			return false;
		}
		break;

	}
	return false;
}

int Table::select(){
	FILE *f = std::fopen(manag->name_table.c_str(), "rb");
	if (f == NULL){
		throw std::logic_error("The table is not exists");
		return -1;
	}
	int len;
	int num;
	int len_temp = 0;
	int num_temp = 0;
	fread(&len, sizeof(len), 1, f);
	fread(&num, sizeof(num), 1, f);
	
	int *type_fields = new int[len];
	int *size_fields = new int[len];
	fread(type_fields, sizeof(int), len, f);
	fread(size_fields, sizeof(int), len, f);
	
	for (int i = 0; i < len; ++i){
		manag->types.push_back((lex_type_t)type_fields[i]);
	}

	bool *is_appropriate = new bool[len];
	

	for (int i = 0; i < len; ++i){
		char ch[NAME_LENGTH + 1];
		ch[NAME_LENGTH] = '\0';
		std::fread(ch, sizeof(char), NAME_LENGTH, f);
		manag->names.push_back(ch);
		is_appropriate[i] = false;
	}
	for (int i = 0; i < len; ++i){
		if (manag->select_all){
			is_appropriate[i] = true;
			++len_temp;
		}
		else {
			for (int j = 0; (long) j < (long) manag->names_select_field.size(); ++j){
				if (!is_appropriate[i] && manag->names[i] == manag->names_select_field[j]){
					is_appropriate[i] = true;
					++len_temp;
				}
			}
		}
	}

	temp_file = tmpfile();
	fwrite(&len_temp, sizeof(len_temp), 1, temp_file); 
	fwrite(&num_temp, sizeof(num_temp), 1, temp_file);
	for (int i = 0; i < len; ++i){
		if (is_appropriate[i]){
			fwrite(&type_fields[i], sizeof(int), 1, temp_file);
		}
	}
	for (int i = 0; i < len; ++i){
		if (is_appropriate[i]){
			fwrite(&size_fields[i], sizeof(int), 1, temp_file);
		}
	}
	for (int i = 0; i < len; ++i){
		if (is_appropriate[i]){
			char ch[NAME_LENGTH + 1];
			ch[NAME_LENGTH] = '\0';
			strcncpy(ch, manag->names[i], NAME_LENGTH);
			std::fwrite(ch, sizeof(char), NAME_LENGTH, temp_file);
		}
	}
	{
		int i = manag->log_expression_stack_r.size() - 1;
		manag->log_expression_stack_temp.resize(i + 1);
		manag->log_expression_types_temp.resize(i + 1);
		for (; i >= 0; --i){
			manag->log_expression_stack_temp[i] = manag->log_expression_stack_r.top();
			manag->log_expression_stack_r.pop();
			manag->log_expression_types_temp[i] = manag->log_expression_types_r.top();
			manag->log_expression_types_r.pop();
		}
	}

	for (int i = 0; i < num; ++i){
		manag->fields.clear();
		for (int j = 0; j < len; ++j){
			if (manag->types[j] == LONG_TYPE){
				unsigned long l;
				std::fread(&l, sizeof(l), 1, f);
				manag->fields.push_back(std::to_string(l));
			} else {
				char *ch = new char[size_fields[j] + 1];
				ch[size_fields[j]] = '\0';
				std::fread(ch, sizeof(char), size_fields[j], f);
				manag->fields.push_back(ch);
				delete[] ch;
			}
		}

		while (!manag->log_expression_types.empty()){
			manag->log_expression_types.pop();
		}
		while (!manag->log_expression_stack.empty()){
			manag->log_expression_stack.pop();
		}
		for (int j = manag->log_expression_stack_temp.size() - 1; j >= 0; --j){
			manag->log_expression_stack.push(manag->log_expression_stack_temp[j]);
			manag->log_expression_types.push(manag->log_expression_types_temp[j]);
		}
		try {
			if (where()){
				for (int j = 0; j < len; ++j){
					if (is_appropriate[j]){
						if (type_fields[j] == LINE){
							char *ch = new char[size_fields[j] + 1];
							ch[size_fields[j]] = '\0';
							strcncpy(ch, manag->fields[j], size_fields[j]);
							std::fwrite(ch, sizeof(char), size_fields[j], temp_file);
							std::cout << ch <<'\n';
							delete[] ch;
						} else {
							
							unsigned long l = std::stoul(manag->fields[j].c_str());
							std::cout << l <<'\n';
							std::fwrite(&l, sizeof(l), 1, temp_file);
						}
					}
				}
				++num_temp;
			}
		}
		catch (...){
			delete[] type_fields;
			delete[] size_fields;
			delete[] is_appropriate;
			std::fclose(f);
			std::fclose(temp_file);
			temp_file = NULL;
			throw;
		}

	}
	delete[] type_fields;
	delete[] size_fields;
	delete[] is_appropriate;
	std::fseek(temp_file, sizeof(int), SEEK_SET);
	std::fwrite(&num_temp, sizeof(num_temp), 1, temp_file);
	std::fclose(f);
	return 0;
}



int Table::delete_t(){
	FILE *f = std::fopen(manag->name_table.c_str(), "rb");
	if (f == NULL){
		throw std::logic_error("The table is not exists");
	}
	int len;
	int num;
	int num_temp = 0;
	fread(&len, sizeof(len), 1, f);
	fread(&num, sizeof(num), 1, f);
	
	int *type_fields = new int[len];
	int *size_fields = new int[len];
	fread(type_fields, sizeof(int), len, f);
	fread(size_fields, sizeof(int), len, f);
	for (int i = 0; i < len; ++i){
		manag->types.push_back((lex_type_t)type_fields[i]);
	}

	for (int i = 0; i < len; ++i){
		char ch[NAME_LENGTH + 1];
		ch[NAME_LENGTH] = '\0';
		std::fread(ch, sizeof(char), NAME_LENGTH, f);
		manag->names.push_back(ch);
	}
	
	temp_file = tmpfile();
	fwrite(&len, sizeof(len), 1, temp_file);
	fwrite(&num_temp, sizeof(num_temp), 1, temp_file);
	fwrite(type_fields, sizeof(int), len, temp_file);
	fwrite(size_fields, sizeof(int), len, temp_file);

	for (int i = 0; i < len; ++i){
		char ch[NAME_LENGTH + 1];
		ch[NAME_LENGTH] = '\0';
		strcncpy(ch, manag->names[i], NAME_LENGTH);
		std::fwrite(ch, sizeof(char), NAME_LENGTH, temp_file);
	}
	
	{
		int i = manag->log_expression_stack_r.size() - 1;
		manag->log_expression_stack_temp.resize(i + 1);
		manag->log_expression_types_temp.resize(i + 1);
		for (; i >= 0; --i){
			manag->log_expression_stack_temp[i] = manag->log_expression_stack_r.top();
			manag->log_expression_stack_r.pop();
			manag->log_expression_types_temp[i] = manag->log_expression_types_r.top();
			manag->log_expression_types_r.pop();
		}
	}
	for (int i = 0; i < num; ++i){
		manag->fields.clear();
		for (int j = 0; j < len; ++j){
			if (manag->types[j] == LONG_TYPE){
				unsigned long l;
				std::fread(&l, sizeof(l), 1, f);
				manag->fields.push_back(std::to_string(l));
			}
			else {
				char *ch = new char[size_fields[j] + 1];
				ch[size_fields[j]] = '\0';
				std::fread(ch, sizeof(char), size_fields[j], f);
				manag->fields.push_back(ch);
				delete[] ch;
			}
		}

		while (!manag->log_expression_types.empty()){
			manag->log_expression_types.pop();
		}
		while (!manag->log_expression_stack.empty()){
			manag->log_expression_stack.pop();
		}
		for (int j = manag->log_expression_stack_temp.size() - 1; j >= 0; --j){
			manag->log_expression_stack.push(manag->log_expression_stack_temp[j]);
			manag->log_expression_types.push(manag->log_expression_types_temp[j]);
		}

		try{
			if (!where()){
				for (int j = 0; j < len; ++j){
					if (type_fields[j] == LINE){
						char *ch = new char[size_fields[j] + 1];
						ch[size_fields[j]] = '\0';
						strcncpy(ch, manag->fields[j], size_fields[j]);
						std::fwrite(ch, sizeof(char), size_fields[j], temp_file);
						delete[] ch;
					}
					else {
						unsigned long l = std::stoul(manag->fields[j].c_str());
						std::fwrite(&l, sizeof(l), 1, temp_file);
					}
				}
				++num_temp;
			}
		}
		catch (...){
			delete[] type_fields;
			delete[] size_fields;
			std::fclose(f);
			std::fclose(temp_file);
			temp_file = NULL;
			throw;
		}

	}
	delete[] type_fields;
	delete[] size_fields;
	
	std::fclose(f);
	char byte;
	f = std::fopen(manag->name_table.c_str(), "wb");
	std::fseek(temp_file, 0, SEEK_SET);
	
	std::fread(&byte, sizeof(byte), 1, temp_file);
	while(!feof(temp_file)){
		std::fwrite(&byte, sizeof(byte), 1, f);
		std::fread(&byte, sizeof(byte), 1, temp_file);
	}
	
	std::fclose(temp_file);
	std::fclose(f);
	temp_file = NULL;
	return 0;
}




int Table::update(){
	manag->types.clear();
	manag->names.clear();
	manag->fields.clear();

	FILE *f = std::fopen(manag->name_table.c_str(), "rb+");
	if (f == NULL){
			throw std::logic_error("The table is not exists");
	}
	int len;
	int num;  
	fread(&len, sizeof(len), 1, f);
	fread(&num, sizeof(num), 1, f);
	
	int *type_fields = new int[len];
	int *size_fields = new int[len];
	fread(type_fields, sizeof(int), len, f);
	fread(size_fields, sizeof(int), len, f);
	for (int i = 0; i < len; ++i){
		manag->types.push_back((lex_type_t) type_fields[i]);
	}
	
	int num_ret = 0;
	int size_upd_field = 0;
	
	
	for (int i = 0; i < len; ++i){
		char ch[NAME_LENGTH + 1];
		ch[NAME_LENGTH] = '\0';
		std::fread(ch, sizeof(char), NAME_LENGTH, f);
		manag->names.push_back(ch);
		std::cout << ch <<std::endl;
		if (num_ret != 0 || manag->names[manag->names.size() - 1] == manag->field_name){
			if (size_upd_field == 0){
				size_upd_field = size_fields[i];
			}
			num_ret += size_fields[i];
		}
	}
	
	{
		int i = manag->log_expression_stack_r.size() - 1;
		manag->log_expression_stack_temp.resize(i + 1);
		manag->log_expression_types_temp.resize(i + 1);
		for (; i >= 0; --i){
			manag->log_expression_stack_temp[i] = manag->log_expression_stack_r.top();
			manag->log_expression_stack_r.pop();
			manag->log_expression_types_temp[i] = manag->log_expression_types_r.top();
			manag->log_expression_types_r.pop();
		}
	}
  

	{
		int j = manag->expression_for_update.size();
		manag->expression_for_update_temp.resize(j);
		manag->expression_types_for_update_temp.resize(j);
		for (int i = 0; !manag->expression_for_update.empty(); ++i){
			manag->expression_for_update_temp[i] = manag->expression_for_update.top();
			manag->expression_for_update.pop();
			manag->expression_types_for_update_temp[i] = manag->expression_types_for_update.top();
			manag->expression_types_for_update.pop();
		}
	}

	for (int i = 0; i < num; ++i){
		manag->fields.clear();
		for (int j = 0; j < len; ++j){
			if (manag->types[j] == LONG_TYPE){
				long l;
				std::fread(&l, sizeof(l), 1, f);
				manag->fields.push_back(std::to_string(l));
			}
			else {
				char *ch = new char[size_fields[j] + 1];
				ch[size_fields[j]] = '\0';
				std::fread(ch, sizeof(char), size_fields[j], f);
				manag->fields.push_back(ch);
				delete[] ch;
			}
		}
	 
		while (!manag->log_expression_types.empty()){
			manag->log_expression_types.pop();
		}
		while (!manag->log_expression_stack.empty()){
			manag->log_expression_stack.pop();
		}
		for (int j = manag->log_expression_stack_temp.size() - 1; j >= 0; --j){
			manag->log_expression_stack.push(manag->log_expression_stack_temp[j]);
			manag->log_expression_types.push(manag->log_expression_types_temp[j]);
		}
		bool bt;
		try{
			bt = where();
		}
		catch (...){
			delete[] type_fields;
			delete[] size_fields;
			std::fclose(f);
			throw;
		}
		if (bt){
			while (!manag->log_expression_types.empty()){
				manag->log_expression_types.pop();
			}
			while (!manag->log_expression_stack.empty()){
				manag->log_expression_stack.pop();
			}
			for (int j = manag->expression_for_update_temp.size() - 1; j >= 0; --j){
				manag->log_expression_stack.push(manag->expression_for_update_temp[j]);
				manag->log_expression_types.push(manag->expression_types_for_update_temp[j]);
			}
			try {
				calculate_expression();
			}
			catch  (...) {
				delete[] type_fields;
				delete[] size_fields;
				std::fclose(f);
				throw;
			}
			if (manag->log_expression_types.top() == LINE){
				char *ch = new char[size_upd_field + 1];
				ch[size_upd_field] = '\0';
				strcncpy(ch, manag->log_expression_stack.top(), size_upd_field);
				std::fseek(f, -num_ret, SEEK_CUR);
				std::fwrite(ch, sizeof(char), size_upd_field, f);
				std::fseek(f, num_ret - size_upd_field, SEEK_CUR);
				delete[] ch;
			} else if(manag->log_expression_types.top() == LONG_TYPE){
				unsigned long  l = std::stoul(manag->log_expression_stack.top());
				std::fseek(f, -num_ret, SEEK_CUR);
				std::fwrite(&l, sizeof(l), 1, f);
				std::fseek(f, num_ret - size_upd_field, SEEK_CUR);
			} else {
				delete[] type_fields;
				delete[] size_fields;
				std::fclose(f);
				throw std::logic_error("Unknown field type");
			}
		}

	}
	delete[] type_fields;
	delete[] size_fields;
	std::fclose(f);
	return 0;
}
