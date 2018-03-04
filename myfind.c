//why the user 160 exists??? first char shouldn't be a number
//test-find.sh row 491 User hugo do not exist;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <pwd.h>

#include <sys/types.h>
#include <dirent.h>

#include <fnmatch.h>

#include <libgen.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <ctype.h>


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

uid_t getUidFromString(char *id);


int isValidOption(char *option);
int in_array ( char *needle , char *haystack[], int arraySize);
struct optionItem *searchOption(char* optName);

void do_dir(const char * dir_name,  char * parms[]);

char **cmdLine;

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
	
	do_dir(argv[1], &argv[2]);

	return 0;
}

void errorMsg(int i){
	
	for(i=0;cmdLine[i]!=NULL;i++){
		fprintf( stderr, "%s ",cmdLine[i]);	
	}

	
	
	switch(i) {
		case 1: fprintf( stderr, "Error: Can not open directory"); exit(i); break;
		case 2: fprintf( stderr, "Error: expect parameter"); exit(i); break;
		case 3: fprintf( stderr, "Error: unknown option"); exit(i); break;
		case 4: fprintf( stderr, "Error: Expect Option"); exit(i); break;
		case 5: fprintf( stderr, "Error: unknown User"); exit(i); break;
		case 6: fprintf( stderr, "Error: unknown file type"); exit(i); break;		
		case 7: fprintf( stderr, "Error: expect additional option at the end"); exit(i); break;
		case 8: fprintf( stderr, "Error"); exit(i); break;
		case 9: fprintf( stderr, "Error"); exit(i); break;		
		
		
		
		default: printf("Another Error"); exit(i); break;
	}
}

void do_dir(const char * dir_name,  char * parms[]){
	
	char dirname[255] = { '\0' };
	char filename[255] = { '\0' };
	const char* tmp = dir_name;
	int j = 0;
	
	
	char *lastSlash = strrchr ( dir_name, '/' ); 
	const char *tmpSlash = dir_name;
	int k = 1;
	
	while(lastSlash != tmpSlash){
		k++;
		tmpSlash++;
	}
	
	strncpy(dirname, dir_name, k);
	tmp = tmp + k;
	
	for(int i = 0; i<strlen(tmp); i++){
		if(tmp[i] == '*' || tmp[i] == '?' || tmp[i] == '['|| tmp[i] == '\\') {
			filename[j] = '\\';
			j++;
		}
		
		
		
		filename[j] = tmp[i];
		j++;
	}

	DIR *currentDirectory =  opendir(dirname);

	if(currentDirectory == NULL) errorMsg(1); // Error Opndir
	
	struct dirent *currentDirEnt = NULL;
	while((currentDirEnt = readdir(currentDirectory)) != NULL){
		int print = 1;
		int found = fnmatch(filename, currentDirEnt->d_name, 0);

		char ganzerPfad[1024] = {'\0'};

		strcpy(ganzerPfad,dirname);
		strcat(ganzerPfad,currentDirEnt->d_name);
		int fd = open(ganzerPfad, O_RDONLY);
		if(parms[0] != NULL){
			if( strcmp(parms[0],"-user") == 0){

				uid_t userid = getUidFromString(parms[1]);
				if(fd >= 0) { 
					struct stat fileStat;
					fstat(fd, &fileStat);
					if(fileStat.st_uid != userid){
						print = 0;
					}
				}
			}
			
			
			
			if( strcmp(parms[0],"-name") == 0){
				if(fnmatch(parms[1], currentDirEnt->d_name, 0) != 0) print = 0;
			}
			

			
			
			if( strcmp(parms[0],"-type") == 0){
				if(fd >= 0) { 
					struct stat fileStat;
					fstat(fd, &fileStat);
					print = 0;
					switch((int)parms[1][0]) {
						case 'f': if(S_ISREG(fileStat.st_mode)) print = 1; break;
						case 'd': if(S_ISDIR(fileStat.st_mode)) print = 1; break;
						case 'b': if(S_ISBLK(fileStat.st_mode)) print = 1; break;
						case 'c': if(S_ISCHR(fileStat.st_mode)) print = 1; break;
						case 'p': if(S_ISFIFO(fileStat.st_mode)) print = 1; break;
						case 'l': if(S_ISLNK(fileStat.st_mode)) print = 1; break;
						case 's':  print = 0; break;
						default: errorMsg(99); break;
					}
				}				
				
			}

			
			
		}

		if(found == 0 && print == 1) {
			printf("%s%s\n",dirname,currentDirEnt->d_name);
		}
	
	
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
	if(userinfo != NULL){
		return userinfo->pw_uid;
	}

	if(isdigit(id[0]) == 0) return 0;
	
	long int userid = strtol(id, NULL, 10);
	userinfo =  getpwuid(userid);
	if(userinfo != NULL){
		return userinfo->pw_uid;;
	}
	return 0;
}

