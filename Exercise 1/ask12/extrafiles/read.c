void readmyfile(int fdread){
	char buff[1024];
	ssize_t rcnt;
	for (;;){
		rcnt = read(fdread,buff,sizeof(buff)-1);
		if (rcnt == 0) /* end-of-file */
			return ;
		if (rcnt == -1){ /* error */
			perror("read");
			return ;
		}
		buff[rcnt] = '\0';
		writetomyfile(buff);
	}
}
