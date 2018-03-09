#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <pwd.h>
#include <grp.h>
#include <time.h>




int main(int argc, char* argv[]){
	
	
	char path[] = "/home/ic17b049/myfind/.git/branches";
	struct stat buf;
	int statRes = lstat(path, &buf);
	
	struct passwd *userInfo;
	struct group *groupInfo;
	
	if(statRes != 0){
		printf("ERROR lstat");
		exit(1);
	}
	
	// Inode
	printf("%lu",buf.st_ino); 
	printf("\t");
	
	
	//
	
	printf("%lu ", buf.st_blocks/2);
	
	// File type
	char fileType = '?';
	if(S_ISREG(buf.st_mode)) fileType = '-';
	else if(S_ISDIR(buf.st_mode)) fileType = 'd';
	else if(S_ISBLK(buf.st_mode)) fileType = 'b';
	else if(S_ISCHR(buf.st_mode)) fileType = 'c';
	else if(S_ISFIFO(buf.st_mode)) fileType = 'p';
	else if(S_ISLNK(buf.st_mode)) fileType = 'l';

	
	printf("%c", fileType);
	
	// Permision
	
	printf("%c", (buf.st_mode & S_IRUSR)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWUSR)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXUSR)? 'x' : '-');

	printf("%c", (buf.st_mode & S_IRGRP)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWGRP)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXGRP)? 'x' : '-');

	printf("%c", (buf.st_mode & S_IROTH)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWOTH)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXOTH)? 'x' : '-');	
	
	printf("\t");	
	
	//Hardlinks
	
	printf("%i\t",buf.st_nlink); 
	
	
	
	//Username
	
	userInfo =  getpwuid(buf.st_uid);
	
	printf("%s", userInfo->pw_name);		
	printf("\t");		

	//Groupname

	groupInfo = getgrgid(buf.st_gid);
	printf("%s", groupInfo->gr_name);		
	printf("\t");		
	
	// Size in Bytes
	printf("%lu",buf.st_size); 
	printf("\t");	

	// Time
	
	char buff[20];
	time_t time = buf.st_mtime;
	strftime(buff, 20, "%b %e %H:%M", localtime(&time));
	printf("%s",buff);
	printf("\t");
	//Path
	printf("%s",path);
	printf("\n");
}