#include <string.h>
#include <stdio.h>
#include "instructions.h"

#include "asm.h"
#include "chain.h"

#define SYMBOL_TEXT						0
#define SYMBOL_IDENTIFIER				1
#define SYMBOL_WHITESPACE				2
#define SYMBOL_CONTROL					3

#define BRACE_NOT_A_BRACE				0
#define BRACE_CURLY						1
#define BRACE_SQUARE					2
#define BRACE_ROUND						3

#define ARG_INVALID						0
#define ARG_CONST						1
#define ARG_STACK						2
#define ARG_MEMORY						3
#define ARG_MEMORY_STACK				4

bool isLetter(char);
bool isDigit(char);
bool toDefint(char* str, int4& res);

typedef chain<int4> id_pair;

int parseArg(int pass, char* arg, int4& value, id_pair* labels)
{
	char argval[MAX_TOKEN_LENGTH];
	int tpe = ARG_INVALID;
	int braces = 0;

	if (arg == NULL || *arg == 0) return ARG_INVALID;

	int len = 0;
	while (arg[len] != 0)
		len++;

	if (len > 2 && arg[0] == '[' && arg[1] == '{' && arg[len - 2] == '}' && arg[len - 1] == ']')
	{
		braces = 2;
		tpe = ARG_MEMORY_STACK;
	}
	else if (len > 1 && arg[0] == '[' && arg[len - 1] == ']')
	{
		braces = 1;
		tpe = ARG_MEMORY;

	}
	else if (len > 1 && arg[0] == '{' && arg[len - 1] == '}')
	{
		braces = 1;
		tpe = ARG_STACK;
	}
	else
	{
		braces = 0;
		tpe = ARG_CONST;
	}

	// Extracting the inner value
	for (int i = 0; i < len - 2 * braces; i++)
	{
		argval[i] = arg[i + braces];
	}
	argval[len - 2 * braces] = 0;

	if (pass == 1)	// On first pass assuming the argument is valid
	{
		return tpe;
	}
	else //if (pass == 2)	 on the second step we should parse the label
	{
		// Checking if the inner value is an identifier
		if (labels->findId(argval, value))
		{
			// Identifier found and resolved
			return tpe;
		}
		else if (toDefint(argval, value))
		{
			// Constant parsed
			return tpe;
		}
		else
		{
			return ARG_INVALID;
		}
	}
}

/*
bool validateInstruction(char* token)
{
	char* cur = token;
	while (*cur != 0)
	{
		if (*cur < 'a' || *cur > 'z' )
		{
			return false;
		}
		cur ++;
	}
	if (cur == token) return false;
	return true;
}

bool validateIdentifier(char* token)
{
	char* cur = token;
	while (*cur != 0)
	{
		if (cur == token && !isLetter(*cur) && *cur != '_') return false;

		if (isLetter(*cur) || isDigit(*cur) || *cur == '_')
		{
			return false;
		}
		cur ++;
	}
	if (cur == token) return false;
	return true;
}
*/

bool toDefint(char* str, int4& res)
{
	int4 sign = 1;
	int4 num = 0;
	char* pos = str;
	if (str == NULL || *pos == 0) return false;
	if (*pos == '-')
	{
		sign = -1;
		pos++;
	}
	for (; *pos != 0; pos++)
	{
		int digit = (int)(*pos) - '0';
		if (digit >= 0 && digit <= 9)
			num = num * 10 + digit;
		else
			return false;
	}
	res = num * sign;
	return true;
}

bool isLetter(char ch)
{
	return ((ch >= 'a' && ch <= 'z') ||
			(ch >= 'A' && ch <= 'Z'));
}

bool isDigit(char ch)
{
	return ch >= '0' && ch <= '9';
}

bool isWhitespace(char ch)
{
	return ch == ' ' || ch == '\t';
}

char braceType(char ch, bool& is_open)
{
	switch (ch)
	{
		case '(':
			is_open = true;
			return BRACE_ROUND;

		case '{':
			is_open = true;
			return BRACE_CURLY;

		case '[':
			is_open = true;
			return BRACE_SQUARE;

		case ')':
			is_open = false;
			return BRACE_ROUND;

		case '}':
			is_open = false;
			return BRACE_CURLY;

		case ']':
			is_open = false;
			return BRACE_SQUARE;
	}

	return BRACE_NOT_A_BRACE;
}

bool addInstr(int1* target, int4 target_size, int4& mem_pos, instr_t instr, int4 args[], int arg_num)
{
	if (mem_pos + 1 + arg_num * sizeof(int4) < target_size)
	{
		*(instr_t*)(&target[mem_pos]) = instr;
		mem_pos += sizeof(instr_t);
		for (int i = 0; i < arg_num; i++)
		{
			*(int4*)(&target[mem_pos]) = args[i];
			mem_pos += sizeof(int4);
		}
		return true;
	}
	else
		return false;
}

bool addData(int1* target, int4 target_size, int4& mem_pos, wchar_t* data, int4 data_length)
{
	if (mem_pos + /*1 + sizeof(defint) +*/ data_length * sizeof(wchar_t) < target_size)
	{
		/*target[mem_pos] = data_data;
		mem_pos ++;

		*(defint*)(&target[mem_pos]) = data_length * sizeof(wchar_t);
		mem_pos += sizeof(defint);*/

		for (int i = 0; i < data_length; i++)
		{
			*(wchar_t*)(&target[mem_pos]) = data[i];
			mem_pos += sizeof(wchar_t);
		}
		return true;
	}
	else
		return false;
}

int assemble(char* code, int1* target, int4 max_target_size, int& error_line, int& error_col, int4& actual_target_size, FILE* debug_symbols_destination)
{
#	define RAISE_LC_ERROR(err)			\
		{								\
			error_line = line_index;	\
			error_col = col_index;		\
			res = err;					\
			goto exit;					\
		}

#	define RAISE_L_ERROR(err)			\
		{								\
			error_line = line_index;	\
			error_col = -1;				\
			res = err;					\
			goto exit;					\
		}

#	define RAISE_ERROR(err)				\
		{								\
			error_line = line_index;	\
			error_col = -1;				\
			res = err;					\
			goto exit;					\
		}

	int res = ASM_ASSEMBLED_SUCCESSFULLY;
	char tokens[MAX_TOKENS_NUM][MAX_TOKEN_LENGTH];

	id_pair* labels = new id_pair;

	labels->value = 0;
	labels->id_name = new char[2];
	labels->id_name[0] = '@';
	labels->id_name[1] = 0;

	labels->next = NULL;

	for (int pass = 1; pass <= 2; pass ++)
	{
		char* cur = code;
		int4 line_index = 0;
		int4 mem_pos = 0;

		while (*cur != '\x0')
		{
			// Writing the current memory position to debug symbols
			if (pass == 1) fwrite(&mem_pos, sizeof(mem_pos), 1, debug_symbols_destination);

			// Dividing the line into tokens
			bool first_newline = true;
			while (*cur == '\n')
			{
				if (pass == 1 && !first_newline)
				{
					char zero = 0;
					if (pass == 1) fwrite(&zero, 1, 1, debug_symbols_destination);
					fwrite(&mem_pos, sizeof(mem_pos), 1, debug_symbols_destination);
				}
				first_newline = false;
				cur++;
				line_index++;
			}

			// Parsing the current line
			char* lin_cur = NULL;// = tokens[0];
			bool quotes_on = false;
			bool in_comment = false;

			int prev_symbol = SYMBOL_WHITESPACE;
			int cur_symbol = SYMBOL_WHITESPACE;

			int tokens_num = 0;

			int col_index = 0;
			int braces_num = 0;

			while (*cur != '\n' && *cur != '\x0')
			{
				char braces[MAX_BRACES_NUM];	// braces stack
				bool is_brace_open = false;
				char brace_type = braceType(*cur, is_brace_open);

				char symbol_to_add = 0;		// 0 means 'don't add'
				bool force_new_token = false;

				prev_symbol = cur_symbol;

				if (in_comment)
				{
					// Do nothing
				}
				else if (braces_num == 0 && quotes_on && (*cur == '"') && (*(cur + 1) == '"'))
				{
					symbol_to_add = *cur;
					if (pass == 1) fwrite(cur, 1, 1, debug_symbols_destination);
					cur ++;		// Not an error - we pass thru 2 characters
				}
				else if (*cur == '"')
				{
					quotes_on = !quotes_on;
					force_new_token = quotes_on;
					symbol_to_add = *cur;
					cur_symbol = SYMBOL_TEXT;
				}
				else if (!quotes_on && brace_type != BRACE_NOT_A_BRACE)
				{
					symbol_to_add = *cur;
					cur_symbol = SYMBOL_TEXT;
					if (is_brace_open)
					{
						braces[braces_num] = brace_type;
						force_new_token = braces_num == 0;
						braces_num ++;
					}
					else
					{
						if (braces_num == 0)
						{
							RAISE_LC_ERROR(ASM_EXTRA_CLOSING_BRACE)
						}
						else if (braces[braces_num - 1] == brace_type)
						{
							braces_num --;
						}
						else
						{
							RAISE_LC_ERROR(ASM_INCORRECT_CLOSING_BRACE)
						}
					}
				}
				else if (quotes_on || braces_num > 0)
				{
					symbol_to_add = *cur;
					cur_symbol = SYMBOL_TEXT;
				}
				else if (isLetter(*cur) || isDigit(*cur) || *cur == '_')
				{
					symbol_to_add = *cur;
					cur_symbol = SYMBOL_IDENTIFIER;
				}
				else if (isWhitespace(*cur))
				{
					cur_symbol = SYMBOL_WHITESPACE;
				}
				else if (*cur == ',' || *cur == ':')
				{
					// Adding a single-character token
					symbol_to_add = *cur;

					cur_symbol = SYMBOL_CONTROL;
				}
				else if (*cur == ';')
				{
					in_comment = true;
				}
				else
				{
					RAISE_LC_ERROR(ASM_INCORRECT_SYMBOL)
				}

				if (force_new_token || (cur_symbol != SYMBOL_WHITESPACE && prev_symbol != cur_symbol))
				{
					// Close the token if it's not the very first one
					if (tokens_num > 0)
						*lin_cur = 0;
					// Open the next
					lin_cur = tokens[tokens_num];
					tokens_num++;
				}

				if (symbol_to_add > 0)
				{
					*lin_cur = *cur;
					lin_cur ++;
				}

				if (pass == 1) fwrite(cur, 1, 1, debug_symbols_destination);
				cur ++;
				col_index ++;
			}

			if (quotes_on)
			{
				RAISE_LC_ERROR(ASM_UNCLOSED_QUOTE)
			}
			if (braces_num > 0)
			{
				RAISE_LC_ERROR(ASM_UNCLOSED_BRACE)
			}

			if (lin_cur != NULL) *lin_cur = 0;	// Closing the last token

			// Appending the zero-closing char to the debug symbols
			char zero = 0;
			if (pass == 1) fwrite(&zero, 1, 1, debug_symbols_destination);

			// Analyzing tokens
			int instr_start = 0;
			if (tokens_num >= 2 && areEqual(tokens[1], ":"))	// If the label exists
			{
				instr_start = 2;
				if (pass == 1  && !labels->addId(tokens[0], mem_pos))
				{
					RAISE_L_ERROR(ASM_DUPLICATED_LABEL)
				}
			}

			// Generating the code

			if (tokens_num > instr_start)	// If there is an instruction
			{
				if (areEqual(tokens[instr_start], "nop"))
				{
					if (tokens_num > instr_start + 1)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}
					if (!addInstr(target, max_target_size, mem_pos, nop, NULL, 0))
					{
						RAISE_L_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "alloc"))
				{
					if (tokens_num > instr_start + 2)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}
					int4 alloc_arg = 0;
					if (!toDefint(tokens[instr_start + 1], alloc_arg))
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
					}

					if (!addInstr(target, max_target_size, mem_pos, alloc_const, &alloc_arg, 1))
					{
						RAISE_L_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "free"))
				{
					if (tokens_num > instr_start + 2)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}
					int4 free_arg = 0;
					if (!toDefint(tokens[instr_start + 1], free_arg))
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
					}

					if (!addInstr(target, max_target_size, mem_pos, free_const, &free_arg, 1))
					{
						RAISE_L_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "mov"))
				{
					int4 arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, mov_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, mov_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_MEMORY_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, mov_m_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_MEMORY_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, mov_stp_m_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}

				}
				else if (areEqual(tokens[instr_start], "add"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}

					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, add_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, add_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "sub"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, sub_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, sub_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "mul"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, mul_stp_stp, arg_vals, 2))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, mul_stp_const, arg_vals, 2))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "div"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, div_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, div_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "mod"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, mod_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, mod_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "not"))
				{
					int arg_vals[2];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int argnum;

					if (tokens_num == instr_start + 4)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 2;
					}
					else if (tokens_num == instr_start + 2)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);

						argnum = 1;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, not_stp, arg_vals, 2))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "and"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, and_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, and_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "or"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, or_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, or_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "xor"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (arg1_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, xor_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, xor_stp_const, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "if"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}

					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, if_stp_flow, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "ifp"))
				{
					int arg_vals[3];
					arg_vals[0] = 4;		// default size in bytes

					int arg1_type;
					int arg2_type;
					int argnum;

					if (tokens_num == instr_start + 6)
					{
						int arg0_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg1_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 5], arg_vals[2], labels);

						if (arg0_type != ARG_CONST)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARG_TYPE_CONST_WANTED)
						}

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						if (!areEqual(tokens[instr_start + 4], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

						argnum = 3;
					}
					else if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[1], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[2], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
						argnum = 2;
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK && arg2_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, ifp_stp_flow, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "regin"))
				{
					int arg_vals[2];

					int arg1_type;
					int arg2_type;

					if (tokens_num == instr_start + 4)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
						arg2_type = parseArg(pass, tokens[instr_start + 3], arg_vals[1], labels);

						if (!areEqual(tokens[instr_start + 2], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}

					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg1_type == ARG_INVALID || arg2_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_CONST && arg2_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, regin_const_stp, arg_vals, 2))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "uregin"))
				{
					if (tokens_num != instr_start + 2)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					int arg_vals[1];

					int arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);

					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, uregin_const, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "call"))
				{
					if (tokens_num > instr_start + 2)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					int arg_vals[1];

					int arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);

					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, call_stp, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, call_flow, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "ret"))
				{
					if (tokens_num > instr_start + 1)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					if (!addInstr(target, max_target_size, mem_pos, ret_stp, NULL, 0))
					{
						RAISE_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "hret"))
				{
					if (tokens_num > instr_start + 1)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					if (!addInstr(target, max_target_size, mem_pos, hret_stp, NULL, 0))
					{
						RAISE_ERROR(ASM_NO_MEMORY)
					}
				}

				else if (areEqual(tokens[instr_start], "jmp"))
				{
					if (tokens_num > instr_start + 2)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					int arg_vals[1];

					int arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);

					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_CONST)
					{
						if (!addInstr(target, max_target_size, mem_pos, jmp_flow, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "out"))
				{
					int arg_vals[3];
					int arg_types[3];

					if (tokens_num == instr_start + 4)
					{
						for (int i = 0; i < 3; i ++)
						{
							arg_types[i] = parseArg(pass, tokens[instr_start + 1 + 2 * i], arg_vals[i], labels);
						}

						for (int i = 0; i < 2; i ++)
						{
							if (!areEqual(tokens[instr_start + 2 + 2 * i], ","))
							{
								RAISE_L_ERROR(ASM_MISSING_COMMA)
							}
						}

					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}


					if (arg_types[0] == ARG_INVALID || arg_types[1] == ARG_INVALID || arg_types[2] == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg_types[0] == ARG_CONST && arg_types[1] == ARG_STACK && arg_types[2] == ARG_MEMORY_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, out_const_stp_m_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg_types[0] == ARG_CONST && arg_types[1] == ARG_STACK && arg_types[2] == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, out_const_stp_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg_types[0] == ARG_CONST && arg_types[1] == ARG_CONST && arg_types[2] == ARG_MEMORY_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, out_const_const_m_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg_types[0] == ARG_CONST && arg_types[1] == ARG_CONST && arg_types[2] == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, out_const_const_stp, arg_vals, 3))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}

				}
				else if (areEqual(tokens[instr_start], "halt"))
				{
					if (tokens_num > instr_start + 1)
					{
						RAISE_L_ERROR(ASM_UNEXPECTED_IDENTIFIER)
					}

					if (!addInstr(target, max_target_size, mem_pos, halt, NULL, 0))
					{
						RAISE_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "setcont"))
				{
					int arg_vals[1];

					int arg1_type;

					if (tokens_num == instr_start + 2)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}

					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, setcont_stp, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_MEMORY_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, setcont_m_stp, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "getcont"))
				{
					int arg_vals[1];

					int arg1_type;

					if (tokens_num == instr_start + 2)
					{
						arg1_type = parseArg(pass, tokens[instr_start + 1], arg_vals[0], labels);
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENTS_NUMBER)
					}

					if (arg1_type == ARG_INVALID)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}
					else if (arg1_type == ARG_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, getcont_stp, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else if (arg1_type == ARG_MEMORY_STACK)
					{
						if (!addInstr(target, max_target_size, mem_pos, getcont_m_stp, arg_vals, 1))
						{
							RAISE_ERROR(ASM_NO_MEMORY)
						}
					}
					else
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
					}
				}
				else if (areEqual(tokens[instr_start], "text"))
				{
					wchar_t txt_res[MAX_DATA_BUFFER_LENGTH];
					wchar_t *res_cur = txt_res;

					for (int i = instr_start + 1; i < tokens_num; i++)
					{
						if (tokens[i][0] == '"')
						{
							int len = 0;
							while (tokens[i][len] != 0) len++;

							for (int j = 1; j < len - 1; j++)
							{
								*res_cur = tokens[i][j];
								res_cur ++;
							}
						}
						else
						{
							int4 arg_val;
							int arg_type = parseArg(pass, tokens[i], arg_val, labels);

							if (arg_type == ARG_INVALID)
							{
								RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
							}
							else if (arg_type == ARG_CONST)
							{
								*res_cur = arg_val;
								res_cur ++;
							}
							else
							{
								RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
							}
						}
					}

					if (!addData(target, max_target_size, mem_pos, txt_res, (int4)(res_cur - txt_res)))
					{
						RAISE_ERROR(ASM_NO_MEMORY)
					}
				}
				else if (areEqual(tokens[instr_start], "int1"))
				{
					int1 int_res[MAX_DATA_BUFFER_LENGTH];
					int1 *res_cur = int_res;

					for (int i = instr_start + 1; i < tokens_num; i++)
					{
						int4 arg_val;
						int arg_type = parseArg(pass, tokens[i], arg_val, labels);

						if (arg_type == ARG_INVALID)
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
						}
						else if (arg_type == ARG_CONST)
						{
							printf("%d, %d\n", arg_val, arg_val % 256);
							if (arg_val == arg_val % 256)
							{
								*res_cur = arg_val % 256;
								res_cur ++;
							}
							else
							{
								RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_VALUE)
							}
						}
						else
						{
							RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT_TYPE)
						}
					}
				}
				else
				{
					RAISE_L_ERROR(ASM_UNKNOWN_INSTRUCTION)
				}
			}

		}
		actual_target_size = mem_pos;
	}
exit:
	labels->freeIds();
	return res;

}
