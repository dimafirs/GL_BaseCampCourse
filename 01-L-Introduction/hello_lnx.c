#include <stdio.h>

int main(int argc, char *argv[])
{
	if (1 == argc) {
		printf("Hello, world!\n");
	}

	for (int i = 1; i < argc; i++) {
		printf("%-4d Hello, %s!\n", i, argv[i]);
	}

	return 0;
}