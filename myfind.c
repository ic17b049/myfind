//why the user 160 exists??? first char shouldn't be a number
//test-find.sh row 491 User hugo do not exist;
//*
// @file myfind.c
// Betriebssysteme My Find File
// Beispiel 1
//
// @author Dominic Schebeck, Dominik Marcel Rychly, Thomas Neugschwandtner
// @date 03/13/2018
//
// @version 1.0
//

// ------------------------------------------------------includes----------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <grp.h>
#include <time.h>

struct optionItem {
	char *name;
	int params;
};

char *option[] = {
		"-user",
		"-name",
		"-type",
		"-print",
		"-ls",
		"-nouser",
		"-path"
	
		//"-group",
		//"-nogroup"
	};


void errorMsg(int i);
void do_dir(const char * dir_name,  char * parms[]);
uid_t getUidFromString(char *id);


int isValidOption(char *option);
int in_array ( char *needle , char *haystack[], int arraySize);
struct optionItem *searchOption(char* optName);

void do_file(char * dir_name,  char * parms[]);

char **cmdLine;
void lsprint(char* path);
void spclPrint(char *str);

//*
// \brief The MyFind C program
//
//This is the main entry point for any C program
//
// \param argc the number of arguments
// \param argv the arguments itselves (including the program name in argv[0])
//
// \return always "success"
// \retval 0 always
//
int main(int argc, char* argv[])
{	
	cmdLine = argv;


	int expParam = 0;	
	struct optionItem *optItem = NULL;
	int optionPos = 0;

	for(int i = 0;i<argc;i++){
		switch(i) {
			case 0:
				//expect Program name
				break;
			case 1:
				//expect Path
				break;
			default:
				//expect Option/Parameter
				if(argv[i][0] == '-'){
					//Option
					if(expParam > 0) errorMsg(2); // expect Parameter
					
					optItem = searchOption(argv[i]);
					if(optItem == NULL) errorMsg(3); // unknown Option
					
					optionPos = i; // Store Option Position
					expParam = optItem->params;
					
				}else{
					//Parameter
					if(expParam == 0) errorMsg(4); // Expect Option
					expParam--;
			
				}
				break;	
		}	

		if(optItem != NULL && optionPos != 0 && expParam == 0){
			if(strcmp(optItem->name,"-user") == 0){
				if(getUidFromString(argv[( optionPos + 1 )]) == 0) errorMsg(5);  // unknown User
			}
			
			if(strcmp(optItem->name,"-type") == 0){
				char allowedFileTypes[] = "fdbcpls";
				char *validType = memchr(allowedFileTypes, argv[( optionPos + 1 )][0], strlen(allowedFileTypes));
				
				if(validType == NULL) errorMsg(6);  // unknown file type
			}
		}
	}
	
	if(expParam > 0) errorMsg(7);;  // expect additional option at the end;
	

	do_file(argv[1], &argv[2]);	
	
	
	return 0;
}

void errorMsg(int i){

	fprintf( stderr, "%s: ",cmdLine[0]);	
	
	switch(i) {
		case 1: fprintf( stderr, "Error: Can not open directory\n"); exit(i); break;
		case 2: fprintf( stderr, "Error: expect parameter\n"); exit(i); break;
		case 3: fprintf( stderr, "Error: unknown option\n"); exit(i); break;
		case 4: fprintf( stderr, "Error: Expect Option\n"); exit(i); break;
		case 5: fprintf( stderr, "Error: unknown User\n"); exit(i); break;
		case 6: fprintf( stderr, "Error: unknown file type\n"); exit(i); break;		
		case 7: fprintf( stderr, "Error: expect additional option at the end\n"); exit(i); break;
		case 8: fprintf( stderr, "Error: Cannt Open directory\n"); exit(i); break;
		case 9: fprintf( stderr, "Error\n"); exit(i); break;		
		default: printf("Another Error\n"); exit(i); break;
	}
}



void do_dir(const char * dir_name,  char * parms[]){
	DIR *directory = opendir(dir_name);
	if (directory == NULL) errorMsg(8);
	struct dirent *direntry; ;
	while((direntry = readdir(directory))){
		//char newFile[10024] = {'\0'};
		
		size_t newFileLen = strlen(dir_name)+ 1 +strlen(direntry->d_name) + 1 ;

		char *newFile = (char *) malloc(newFileLen * sizeof(char));
		
		if( strcmp(direntry->d_name,".") == 0 ) continue;
		if( strcmp(direntry->d_name,"..") == 0 ) continue;
		
		strcpy(newFile, dir_name);
		strcat(newFile, "/");
		strcat(newFile, direntry->d_name);
		do_file(newFile,parms);
		
		free(newFile);
	}
}

void do_file(char * dir_name,  char *parms[]){

	int print = 1;
	int smthPrinted = 0;
	int currentParam = 0;
	int cParam = 0;
	struct stat newStatBuffer;
	char *fileName = basename(dir_name);
	lstat(dir_name, &newStatBuffer);
	
	while(parms[currentParam] != NULL  && print == 1){
		cParam = 0;
		if( strcmp(parms[currentParam],"-user") == 0){
			uid_t userid = getUidFromString(parms[currentParam + 1]);
			if(newStatBuffer.st_uid != userid)	print = 0;
			cParam += 2;
		}
		
		if( strcmp(parms[currentParam],"-nouser") == 0){
			if(getpwuid(newStatBuffer.st_uid) != NULL ) print = 0;
			cParam += 1;
		}			
		
		if( strcmp(parms[currentParam],"-name") == 0){
			if(fnmatch(parms[currentParam + 1], fileName, 0) != 0) print = 0;
			cParam += 2;
		}
		if( strcmp(parms[currentParam],"-path") == 0){
			if(fnmatch(parms[currentParam + 1], dir_name, 0) != 0) print = 0;
			cParam += 2;
		}
		
		if( strcmp(parms[currentParam],"-type") == 0){
			char currFileType = '\0';

			if(S_ISREG(newStatBuffer.st_mode)) currFileType = 'f';
			if(S_ISDIR(newStatBuffer.st_mode)) currFileType = 'd';
			if(S_ISBLK(newStatBuffer.st_mode)) currFileType = 'b';
			if(S_ISCHR(newStatBuffer.st_mode)) currFileType = 'c';
			if(S_ISFIFO(newStatBuffer.st_mode)) currFileType = 'p';
			if(S_ISLNK(newStatBuffer.st_mode)) currFileType = 'l';
			if(S_ISSOCK(newStatBuffer.st_mode)) currFileType = 's';
		
			if(currFileType != parms[currentParam + 1][0]) print = 0;
			cParam += 2;
		}
		
		if( strcmp(parms[currentParam],"-print") == 0){
			printf("%s\n",dir_name);
			smthPrinted = 1;
			cParam += 1;
		}		
		
		if( strcmp(parms[currentParam],"-ls") == 0){
			lsprint(dir_name);
			smthPrinted = 1;
			cParam += 1;
		}
		
		currentParam += cParam;
	}

	if(print == 1 && smthPrinted == 0) {
		printf("%s\n",dir_name);
	}
	
	if(S_ISDIR(newStatBuffer.st_mode)){
		do_dir(dir_name, parms);	
	}
	

}


int in_array ( char *needle , char *haystack[], int arraySize){
	int inArray = 0;
	for(int i = 0; i<arraySize; i++){
		int cmpVal = strcmp(haystack[i],needle);
		if(cmpVal == 0) inArray = 1;
	}
	return !inArray;
}

struct optionItem *searchOption(char* optName){
	static struct optionItem optArray[] = {
		{.name="-user",.params=1},
		{.name="-name",.params=1},
		{.name="-type",.params=1},
		{.name="-print",.params=0},
		{.name="-ls",.params=0},
		{.name="-nouser",.params=0},
		{.name="-path",.params=1},
		//only for groups with 4 Persons
		//{.name="-group",.minParams=1,.maxParams=1),
		//{.name="-nogroup",.minParams=1,.maxParams=1)
	};
	
	int optionItemLen = (sizeof(optArray)/sizeof(optArray[0]));
	
	for(int i = 0;i<optionItemLen;i++){
		if(strcmp(optArray[i].name,optName) == 0) return &optArray[i];
	}
	
	return NULL;

}

int isValidOption(char *chkOption){

	struct optionItem *optItem = searchOption(chkOption);
	if(optItem != NULL)	return 1;
	
	return 0;
}

uid_t getUidFromString(char *id){
	struct passwd *userinfo;
	
	userinfo = getpwnam(id);
	
	if(userinfo != NULL)	return userinfo->pw_uid;
	if(isdigit(id[0]) == 0) return 0;
	
	long int userid = strtol(id, NULL, 10);
	userinfo =  getpwuid(userid);
	
	if(userinfo != NULL)	return userinfo->pw_uid;
	
	return 0;
}

void lsprint(char* path){
	struct stat buf;
	int statRes = lstat(path, &buf);
	
	struct passwd *userInfo;
	struct group *groupInfo;
	
	char *linkname;
	ssize_t r;
	
	if(statRes != 0){
		printf("ERROR lstat");
		exit(1);
	}
	
	
	// File type
	char fileType = '?';
	if(S_ISREG(buf.st_mode)) fileType = '-';
	else if(S_ISDIR(buf.st_mode)) fileType = 'd';
	else if(S_ISBLK(buf.st_mode)) fileType = 'b';
	else if(S_ISCHR(buf.st_mode)) fileType = 'c';
	else if(S_ISFIFO(buf.st_mode)) fileType = 'p';
	else if(S_ISLNK(buf.st_mode)) fileType = 'l';
	else if(S_ISSOCK(buf.st_mode)) fileType = 's';
	
	
	
	// Inode
	printf("%6lu ",buf.st_ino); 
	printf(" ");
	
	
	//
	
	printf("%3lu", buf.st_blocks/2);

	printf(" ");

	printf("%c", fileType);
	
	// Permision
	
	printf("%c", (buf.st_mode & S_IRUSR)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWUSR)? 'w' : '-');
	printf("%c", 
             (buf.st_mode & S_ISUID) ? 
				(buf.st_mode & S_IXUSR) ? 's' : 'S':
				(buf.st_mode & S_IXUSR) ? 'x' : '-'
	);
			
	
	printf("%c", (buf.st_mode & S_IRGRP)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWGRP)? 'w' : '-');	
	printf("%c", 
             (buf.st_mode & S_ISGID) ?
				(buf.st_mode & S_IXGRP) ? 's' : 'S':
				(buf.st_mode & S_IXGRP) ? 'x' : '-'
	);	

	printf("%c", (buf.st_mode & S_IROTH)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWOTH)? 'w' : '-');
	
	
	printf("%c", 
             (buf.st_mode & S_ISVTX) ?
				(buf.st_mode & S_IXOTH) ? 't' : 'T':
				(buf.st_mode & S_IXOTH) ? 'x' : '-'
	);	

	printf(" ");	
	
	//Hardlinks
	
	printf("%3i",buf.st_nlink); 
	printf(" ");
	
	
	//Username
	
	userInfo =  getpwuid(buf.st_uid);
	
	
	
	printf("%-8s", (userInfo!=NULL) ? userInfo->pw_name : "999999");		
	printf(" ");		

	//Groupname

	groupInfo = getgrgid(buf.st_gid);
	printf("%-8s", groupInfo->gr_name);		
	printf(" ");		
	
	// Size in Bytes
	
	if(fileType == 'b' || fileType == 'c'){
		printf("        ");
	}else{
		printf("%8lu",buf.st_size); 
	}
	
	
	printf(" ");	

	// Time
	
	char buff[20];
	time_t time = buf.st_mtime;
	strftime(buff, 20, "%b %e %H:%M", localtime(&time));
	printf("%s",buff);
	printf(" ");
	//Path
	//printf("%s",path);
	spclPrint(path);
	
	
	//symlink print 
	if(fileType == 'l'){
		printf(" -> ");
		
		
		linkname = malloc(buf.st_size + 1);
		r = readlink(path, linkname, buf.st_size + 1);
		linkname[r] = '\0';
		printf("%s",linkname);
	}

	printf("\n");
}

void spclPrint(char *str){
	while(*str != '\0'){
		if(*str == '\\') printf("\\\\");
		else if(*str == ' ') printf("\\ ");
		else printf("%c",*str);
		str++;
	}
	
}

// End:
