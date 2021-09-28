#include <stdio.h>
#include <unistd.h>

void zing(void) {
	char *A = getlogin();
	printf("Bye bye, %s\n", A);
	return ;
}
