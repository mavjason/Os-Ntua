void writetomyfile(char *buff ) {
	size_t len, idx;
	ssize_t wcnt;
	idx = 0;
	len = strlen(buff);
	do {
		wcnt = write(fdwrite,buff + idx, len - idx);
		if (wcnt == -1){ /* error */
			perror("write");
			return ;
		}
		idx += wcnt;
	} while (idx < len);
	return ;
}
