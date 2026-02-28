#include <stdio.h>
#include <string.h>

void func(char *input) {
	char buffer[256];
	strcpy(buffer, input);
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		func(argv[1]);
	}
}

