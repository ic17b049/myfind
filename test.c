#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]){
	struct stat buf;
	int retVal = lstat(argv[1], &buf);
	if(retVal == 0)	printf("%s\n",(S_ISDIR(buf.st_mode))? "DIR" : "FILE");
	else printf("ERROR\n");
}