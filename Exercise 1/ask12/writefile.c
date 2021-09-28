#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void writefile(char *buff, int fdwrite) {
	size_t len, idx;
	ssize_t wcnt;
	idx = 0;
	len = strlen(buff);
	do {
		wcnt = write(fdwrite,buff + idx, len - idx);
		if (wcnt == -1){ /* error */
			perror("write");
			exit(1) ;
		}
		idx += wcnt;
	} while (idx < len);
	return ;
}
