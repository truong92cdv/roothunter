#include <stdio.h>

int main(int argc, char *argv[]) {
	int secret;
	char name[100];
	printf("Enter your name: ");
	gets(name);
	if (secret == 0x746f6f72) {
		printf("You win!\n");
	}
}
