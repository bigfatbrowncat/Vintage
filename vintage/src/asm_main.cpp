#include <time.h>
#include <stdio.h>

#include "instructions.h"
#include "asm.h"

static const char* error_messages[] =
{
		/* ASSEMBLED_SUCCESSFULLY */			NULL,
		/* INCORRECT_SYMBOL */			 		"incorrect symbol found",
		/* INCORRECT_CLOSING_BRACE */			"incorrect closing brace",
		/* EXTRA_CLOSING_BRACE */				"extra closing brace",
		/* UNCLOSED_BRACE */					"not enough closing braces",
		/* UNCLOSED_QUOTE */				 	"open quote isn't closed",
		/* DUPLICATED_LABEL */			 		"the label is already defined",
		/* UNKNOWN_INSTRUCTION */			 	"unknown instruction",
		/* UNEXPECTED_IDENTIFIER */		 		"unexpected identifier",
		/* NO_MEMORY */					 		"not enough memory",
		/* INCORRECT_ARG_TYPE_CONST_WANTED */	"incorrect argument type, should be a numerical constant",
		/* MISSING_COMMA */				 		"missing comma between arguments",
		/* INCORRECT_ARGUMENT */			 	"incorrect argument",
		/* INCORRECT_ARGUMENT_TYPE */		 	"incorrect argument type",
		/* INCORRECT_ARGUMENT_VALUE */		 	"incorrect argument value",
		/* INCORRECT_ARGUMENTS_NUMBER */		"incorrect arguments number"
};

#define		RESULT_OK			0
#define		RESULT_ERROR		1

static bool debug_mode = false;
static char* debug_symbols_name = NULL;

int main(int argc, char *argv[])
{
	int res = RESULT_OK;

	if (argc < 3)
	{
		printf("Using:\n\tvintage-asm <source> <target> [<debug-symbols>]\n");
		return RESULT_ERROR;
	}

	char* source_name = argv[1];
	printf("Source code %s\n", source_name);
	char* target_name = argv[2];
	printf("Target binary %s\n", target_name);

	if (argc > 3)
	{
		debug_mode = true;
		debug_symbols_name = argv[3];
		printf("Writing debug symbols to %s\n", debug_symbols_name);
	}

	const int4 mem_size = 4096; // 4 Kb should be enough for everyone :)
	int1 mem[mem_size];
	char code[MAX_CODE_LENGTH];

	FILE *src = NULL, *dest = NULL, *debug_symbols_dest = NULL;
	int asmres = -1;

	src = fopen(source_name, "r");
	if (src == NULL)
	{
		printf("Assembly error: can't open the source file\n");
		res = RESULT_ERROR;
		goto exit;
	}
	dest = fopen(target_name, "wb");
	if (dest == NULL)
	{
		printf("Assembly error: can't open the destination file\n");
		res = RESULT_ERROR;
		goto exit;
	}
	if (debug_symbols_name != NULL)
	{
		debug_symbols_dest = fopen(debug_symbols_name, "wb");
		if (debug_symbols_dest == NULL)
		{
			printf("Assembly error: can't open the debug symbols destination file\n");
			res = RESULT_ERROR;
			goto exit;
		}
	}

	code[fread(code, 1, MAX_CODE_LENGTH, src)] = 0;			// Reading the code

	int ln, cl;
	int4 target_size;
	asmres = assemble(code, mem, mem_size, ln, cl, target_size, debug_symbols_dest);
	if (asmres == ASM_ASSEMBLED_SUCCESSFULLY)
	{
		printf("Assembled successfully. %d bytes of code written.\n", target_size);
	}
	else
	{
		if (ln == -1)
		{
			printf("Assembly error: %s\n", error_messages[asmres]);
			res = RESULT_ERROR;
			goto exit;
		}
		else if (cl == -1)
		{
			printf("Syntax error at line %d: %s.\n", ln + 1, error_messages[asmres]);
			res = RESULT_ERROR;
			goto exit;
		}
		else
		{
			printf("Syntax error at line %d, col %d: %s.\n", ln + 1, cl + 1, error_messages[asmres]);
			res = RESULT_ERROR;
			goto exit;
		}

	}

	fwrite(mem, 1, target_size, dest);

exit:
	if (src != NULL) fclose(src);
	if (dest != NULL) fclose(dest);
	if (debug_symbols_dest != NULL) fclose(debug_symbols_dest);

	return res;
}
