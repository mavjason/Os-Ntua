#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
void writetomyfile(char *buff);
void readmyfile(int fdread);
int fdwrite; // global arxeio p grafw gia na min exw themata
int main(int argc, char **argv) {
	int f, g;
	int oflags, mode;
	char novalidname[10];
	strcpy( novalidname, "fconc.out" );
	oflags = O_CREAT | O_WRONLY | O_TRUNC;
	mode = S_IRUSR | S_IWUSR;
	if ( argc < 3 ) {
		printf("Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
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
	if (argc==4) {
		if (strcmp(argv[1],argv[3])==0 || strcmp(argv[2],argv[3])==0  ){
			printf("Name from file which i read has the same with the file where i write . Try again with different output file name.\n");
			close(f);
			close(g);
			close(fdwrite);
			return 1;
		}
		fdwrite = open(argv[3],oflags, mode);
	}
	if (argc==3) {
		if ( (strcmp(argv[1],novalidname)==0) || (strcmp(argv[2],novalidname)==0) ){
			printf("Name from file which i read has the same with the file where i write . Try again with different output file name.\n");
			close(f);
			close(g);
			close(fdwrite);
			return 1;
		}
		else fdwrite = open("fconc.out",oflags, mode);
	}
	if (fdwrite == -1){
		perror("open");
		exit(1);
	}
	readmyfile(f);
	readmyfile(g);
	close(f);
	close(g);
	close(fdwrite);
	return 1;
}

