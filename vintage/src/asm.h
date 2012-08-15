#ifndef ASM_H_
#define ASM_H_

#include "instructions.h"

#define ASM_ASSEMBLED_SUCCESSFULLY				0
#define ASM_INCORRECT_SYMBOL					1
#define ASM_INCORRECT_CLOSING_BRACE				2
#define ASM_EXTRA_CLOSING_BRACE					3
#define ASM_UNCLOSED_BRACE						4
#define ASM_UNCLOSED_QUOTE						5
#define ASM_DUPLICATED_LABEL					6
#define ASM_UNKNOWN_INSTRUCTION					7
#define ASM_UNEXPECTED_IDENTIFIER				8
#define ASM_NO_MEMORY							9
#define ASM_INCORRECT_ARG_TYPE_CONST_WANTED		10
#define ASM_MISSING_COMMA						11
#define ASM_INCORRECT_ARGUMENT					12
#define ASM_INCORRECT_ARGUMENT_TYPE				13
#define ASM_INCORRECT_ARGUMENT_VALUE			14
#define ASM_INCORRECT_ARGUMENTS_NUMBER			15

#define MAX_CODE_LENGTH				1024 * 1024
#define MAX_TOKEN_LENGTH			256
#define MAX_BRACES_NUM				32
#define MAX_TOKENS_NUM				256
#define MAX_DATA_BUFFER_LENGTH		65536

int assemble(char* code, int1* target, int4 max_target_size, int& error_line, int& error_col, int4& actual_target_size, FILE* debug_symbols_destination);


#endif /* ASM_H_ */
