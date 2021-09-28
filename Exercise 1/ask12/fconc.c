#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void readwrite(int fdread, int fdwrite);

int main(int argc, char **argv) {
	int f, g, fdwrite;
	int oflags, mode;
	oflags = O_CREAT | O_WRONLY | O_TRUNC;
	mode = S_IRUSR | S_IWUSR;

	if (argc==4) {		//3 files given
		if (strcmp(argv[1],argv[3])==0 || strcmp(argv[2],argv[3])==0){
			printf("Try again with a different file names.\n");
			exit(1);
		}
		fdwrite = open(argv[3], oflags, mode);
	}
	else if (argc==3) {	//2 files given
		if ( (strcmp(argv[1],"fconc.out")==0) || (strcmp(argv[2],"fconc.out")==0) ){
			printf("Try again with different file names.\n");
			exit(1);
		}
		fdwrite = open("fconc.out", oflags, mode);
	}
	else {			//wrong number of files given
		printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
		exit(1);
	}
	if (fdwrite == -1){
		perror("open");
		exit(1);
	}


	f = open(argv[1], O_RDONLY);
	if (f == -1){
		perror(argv[1]);
		exit(1);
	}
	g = open(argv[2], O_RDONLY);
	if (g == -1){
		perror(argv[2]);
		exit(1);
	}

	readwrite(f, fdwrite);
	readwrite(g, fdwrite);

	close(f);
	close(g);
	close(fdwrite);
	return 0;
}
