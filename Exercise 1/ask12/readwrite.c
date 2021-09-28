#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void writefile(void *buff, int fdwrite);

void readwrite(int fdread, int fdwrite){
	char buff[1024];
	ssize_t rcnt;
	for (;;){
		rcnt = read(fdread,buff,sizeof(buff)-1);
		if (rcnt == 0) /* end-of-file */
			return ;
		if (rcnt == -1){ /* error */
			perror("read");
			exit(1);
		}
		buff[rcnt] = '\0';
		writefile(buff, fdwrite);
	}
	return ;
}
