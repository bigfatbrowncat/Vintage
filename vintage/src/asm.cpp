#include <string.h>
#include <stdio.h>
#include "instructions.h"

#include "asm.h"
#include "chain.h"

#define OP_ADD			0
#define OP_SUB			1

#define SYMBOL_TEXT						0
#define SYMBOL_IDENTIFIER				1
#define SYMBOL_WHITESPACE				2
#define SYMBOL_CONTROL					3

#define BRACE_NOT_A_BRACE				0
#define BRACE_CURLY						1
#define BRACE_SQUARE					2
#define BRACE_ROUND						3

#define PARSECONST_OK					0
#define PARSECONST_INVALID_TOKEN		1
#define PARSECONST_DIV_BY_ZERO			2

#define ARG_INVALID						0
#define ARG_NULL						0	// never meets ARG_INVALID
#define ARG_MEMORY_STACK				1
#define ARG_STACK						2
#define ARG_CONST						4

#define MAX_ARGUMENTS_NUMBER			8
#define MAX_ARG_TYPE_VARIATIONS			32

struct instr_argument_type_variation
{
	instr_t operation_code;
	int optional_argument_type;
	int optional_argument_default_value;
	int argument_type_flags[MAX_ARGUMENTS_NUMBER];		// A combination of ARG_* flags
};

instr_argument_type_variation INSTR_ARGUMENT_TYPE_VARIATION_NULL = { -1, ARG_NULL, 0, {} };

struct instruction_descriptor
{
	char* operator_string;
	instr_argument_type_variation argument_variations[MAX_ARG_TYPE_VARIATIONS];		// A combination of ARG_* flags
};

instruction_descriptor INSTRUCTION_DESCRIPTOR_NULL = { NULL, {} };

instruction_descriptor INSTR_DESCS[] =
{
	{
		"nop",
		{
			{ nop, ARG_NULL, 0, { ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"halt",
		{
			{ halt, ARG_NULL, 0, { ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"alloc",
		{
			{ alloc_const, ARG_NULL, 0, { ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"free",
		{
			{ free_const, ARG_NULL, 0, { ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"mov",
		{
			{ mov_stp_const, ARG_CONST, 4,   { ARG_STACK,        ARG_CONST,        ARG_NULL } },
			{ mov_stp_stp,   ARG_CONST, 4,   { ARG_STACK,        ARG_STACK,        ARG_NULL } },
			{ mov_m_stp_stp, ARG_CONST, 4,   { ARG_MEMORY_STACK, ARG_STACK,        ARG_NULL } },
			{ mov_stp_m_stp, ARG_CONST, 4,   { ARG_STACK,        ARG_MEMORY_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"add",
		{
			{ add_stp_const, ARG_CONST, 4,   { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ add_stp_stp,   ARG_CONST, 4,   { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"sub",
		{
			{ sub_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ sub_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"mul",
		{
			{ mul_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ mul_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"div",
		{
			{ div_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ div_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"mod",
		{
			{ mod_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ mod_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"not",
		{
			{ not_stp,       ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"and",
		{
			{ and_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ and_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"or",
		{
			{ or_stp_const,  ARG_CONST, 4,    { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ or_stp_stp,    ARG_CONST, 4,    { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"xor",
		{
			{ xor_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ xor_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"gr",
		{
			{ gr_stp_const,  ARG_CONST, 4,    { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ gr_stp_stp,    ARG_CONST, 4,    { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"greq",
		{
			{ greq_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ greq_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"lweq",
		{
			{ lweq_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ lweq_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"eq",
		{
			{ eq_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ eq_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"neq",
		{
			{ neq_stp_const, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			{ neq_stp_stp,   ARG_CONST, 4,     { ARG_STACK,        ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jz",
		{
			{ jz_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jnz",
		{
			{ jnz_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jp",
		{
			{ jp_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jnp",
		{
			{ jnp_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jn",
		{
			{ jn_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jnn",
		{
			{ jnn_stp_flow, ARG_CONST, 4,     { ARG_STACK,        ARG_CONST, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"jmp",
		{
			{ jmp_flow,    ARG_NULL, 0,     { ARG_CONST,      ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"call",
		{
			{ call_stp,    ARG_NULL, 0,      { ARG_STACK,      ARG_NULL } },
			{ call_flow,   ARG_NULL, 0,      { ARG_CONST,      ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"ret",
		{
			{ ret,         ARG_NULL, 0,  { ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"hret",
		{
			{ hret,        ARG_NULL, 0,   { ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"setcont",
		{
			{ setcont_stp,   ARG_NULL, 0,        { ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"setcont",
		{
			{ setcont_m_stp, ARG_NULL, 0,          { ARG_MEMORY_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"getcont",
		{
			{ getcont_stp,   ARG_NULL, 0,        { ARG_STACK, ARG_NULL } },
			{ getcont_m_stp, ARG_NULL, 0,          { ARG_MEMORY_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	{
		"out",
		{
			{ out_const,     ARG_NULL, 0,      { ARG_CONST, ARG_NULL } },
			{ out_stp,       ARG_NULL, 0,    { ARG_STACK, ARG_NULL } },
			INSTR_ARGUMENT_TYPE_VARIATION_NULL
		}
	},
	INSTRUCTION_DESCRIPTOR_NULL
};

bool isLetter(char);
bool isDigit(char);
bool toDefint(char* str, int4& res);

typedef chain<int4> id_pair;

int find_instruction_descriptor(char* operator_string)
{
	int instruction_descriptor_index = -1;
	for (int i = 0; INSTR_DESCS[i].operator_string != NULL; i++)
	{
		if (areEqual(operator_string, INSTR_DESCS[i].operator_string))
		{
			instruction_descriptor_index = i;
			break;
		}
	}
	return instruction_descriptor_index;
}

int count_arguments_in_variation_no_optional(instr_argument_type_variation var)
{
	int cnt = 0;
	while (var.argument_type_flags[cnt] != ARG_NULL)
	{
		cnt++;
	}
	return cnt;
}

int parseConst(char* arg, int4& value, id_pair* labels)
{
	int cur_op = OP_ADD;
	char cur_const[MAX_TOKEN_LENGTH];
	int cur_const_len = 0;
	int4 cur_val = 0;

	int l = strlen(arg);
	if (l == 0)
	{
		return PARSECONST_INVALID_TOKEN;
	}

	int i = 0;

	// Checking if the first char is an operator
	if (arg[i] == '+')
	{
		cur_op = OP_ADD;
		i++;
	}
	if (arg[i] == '-')
	{
		cur_op = OP_SUB;
		i++;
	}

	for (; i <= l; i++)
	{
		if (i == l || (arg[i] == '+' || arg[i] == '-' || arg[i] == '*' || arg[i] == '/'))
		{
			int4 tmp_value;
			cur_const[cur_const_len] = 0;

			if (labels->findId(cur_const, tmp_value))
			{
				// Identifier found and resolved
			}
			else if (toDefint(cur_const, tmp_value))
			{
				// Constant parsed
			}
			else
			{
				return PARSECONST_INVALID_TOKEN;
			}

			if (cur_op == OP_ADD)
			{
				cur_val += tmp_value;
			}
			else if (cur_op == OP_SUB)
			{
				cur_val -= tmp_value;
			}
			cur_const_len = 0;

		}

		if (i < l)
		{
			if (arg[i] == '+')
			{
				cur_op = OP_ADD;
			}
			else if (arg[i] == '-')
			{
				cur_op = OP_SUB;
			}
			else
			{
				cur_const[cur_const_len] = arg[i];
				cur_const_len ++;
			}
		}
	}

	value = cur_val;
	return PARSECONST_OK;
}

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
		if (parseConst(argval, value, labels) == PARSECONST_OK)
		{
			// Identifier found and resolved
			return tpe;
		}
		else
		{
			return ARG_INVALID;
		}
	}
}

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
	if (mem_pos + data_length * sizeof(wchar_t) < target_size)
	{
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
				int instruction_descriptor_index = find_instruction_descriptor(tokens[instr_start]);
				if (instruction_descriptor_index > -1)
				{
					// Checking for commas
					for (int i = instr_start + 2; i < tokens_num; i += 2)
					{
						if (!areEqual(tokens[i], ","))
						{
							RAISE_L_ERROR(ASM_MISSING_COMMA)
						}
					}

					if (tokens_num - instr_start != 1 && ((tokens_num - 1) - instr_start) % 2 != 1)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}

					int param_tokens_number;
					if (tokens_num - instr_start == 1)
						param_tokens_number = 0;
					else
						param_tokens_number = ((tokens_num - 1) - instr_start) / 2 + 1;

					// Getting the actual parameter types and values
					int4 arg_vals[param_tokens_number + 1];		// this "1" is for the optional argument
					int4 arg_types[param_tokens_number + 1];

					for (int k = 0; k < param_tokens_number; k++)
					{
						arg_types[k] = parseArg(pass, tokens[instr_start + 1 + 2 * k], arg_vals[k], labels);
					}

					// Trying to map this into any operator variation
					int applied_variation_index = -1;

					for (int var_index = 0;
					     INSTR_DESCS[instruction_descriptor_index].argument_variations[var_index].operation_code != -1;
					     var_index++)
					{
						instr_argument_type_variation cur_var = INSTR_DESCS[instruction_descriptor_index].argument_variations[var_index];
						bool applicable = false;
						bool with_optional = false;

						if (count_arguments_in_variation_no_optional(cur_var) == param_tokens_number)
						{
							applicable = true;
							// Checking the types
							for (int i = 0; i < param_tokens_number; i++)
							{
								if (cur_var.argument_type_flags[i] != arg_types[i])
								{
									applicable = false;
									break;
								}
							}
						}

						if (!applicable &&
						    count_arguments_in_variation_no_optional(cur_var) + 1 == param_tokens_number &&
						    cur_var.optional_argument_type == arg_types[0])
						{
							// Trying again including the optional argument
							applicable = true;
							with_optional = true;

							// Checking the types
							for (int i = 1; i < param_tokens_number; i++)
							{
								if (cur_var.argument_type_flags[i - 1] != arg_types[i])
								{
									applicable = false;
									break;
								}
							}
						}

						if (applicable)
						{
							// We've found it! Now let's generate the code.
							if (cur_var.optional_argument_type != ARG_NULL && !with_optional)
							{
								// Adding the default value as the first argument
								for (int i = param_tokens_number; i > 0; i--)
								{
									arg_vals[i] = arg_vals[i - 1];
									arg_types[i] = arg_types[i - 1];
								}
								arg_types[0] = cur_var.optional_argument_type;
								arg_vals[0] = cur_var.optional_argument_default_value;
								param_tokens_number++;	// one extra argument added
							}

							addInstr(target, max_target_size, mem_pos, cur_var.operation_code, arg_vals, param_tokens_number);

							applied_variation_index = var_index;
							break;
						}

					}

					if (applied_variation_index == -1)
					{
						RAISE_L_ERROR(ASM_INCORRECT_ARGUMENT)
					}

				}

				/*if (areEqual(tokens[instr_start], "add"))
				{
					int4 arg_vals[3];
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
				}*/
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
							if (arg_val == arg_val % 256)
							{
								*res_cur = arg_val % 256;
								res_cur ++;
							}
							else if (pass > 1)
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
