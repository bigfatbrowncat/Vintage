#include <stdlib.h>
#include <stdio.h>

int main()
{
	try
	{

		printf("\nAll tests have passed successfully.\n");
		return EXIT_SUCCESS;
	}
	catch (int i)
	{
		if (i == -1)
		{
			printf("\nTest falure has occured.\n");
		}
		else
		{
			printf("\nException number %d occured during the tests.\n", i);
		}
	}
	return EXIT_FAILURE;
}
