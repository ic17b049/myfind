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
//exit(1) unknown option
//exit(2) expect Option
//exit(3) expect Param

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




int isValidOption(char *option);
int in_array ( char *needle , char *haystack[], int arraySize);
struct optionItem *searchOption(char* optName);

void do_dir(const char * dir_name,  char * parms[]);
void printTest(const char *text);

int main(int argc, char* argv[])
{	


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
					if(expParam > 0) exit(2); // expect Parameter
					
					optItem = searchOption(argv[i]);
					if(optItem == NULL) exit(1); // unknown Option
					
					optionPos = i; // Store Option Position
					expParam = optItem->params;
					
				}else{
					//Parameter
					if(expParam == 0) exit(3); // Expect Option
					expParam--;
			
				}
				break;	
		}	
		

		
		if(optItem != NULL && optionPos != 0 && expParam == 0){
			if(strcmp(optItem->name,"-user") == 0){
				
				struct passwd *userInfo;
				
				//userInfo = getpwnam(argv[(optionPos +1 )]);
				//if(userInfo == NULL) exit(55);  // unknown User
				
				
				if(isdigit(argv[(optionPos +1 )][0])== 0){
					long int uid = strtol(argv[(optionPos +1 )], NULL, 10);
					userInfo = getpwuid(uid);
					if(userInfo == NULL) exit(55);  // unknown User
				}else{
					userInfo = getpwnam(argv[(optionPos +1 )]);
					if(userInfo == NULL) exit(55);  // unknown User
				}
				
				
				
				
				
				
				
			}
			
			if(strcmp(optItem->name,"-type") == 0){
				char allowedFileTypes[] = "-dbcpls";
				char *validType = memchr(allowedFileTypes, optionPos +1, strlen(allowedFileTypes));
				
				if(validType == NULL) exit(1);  // unknown User
			}
				
		}
		
		
	}
	
	if(expParam > 0) exit(4);  // expect additional option at the end;
	
	
	
	
	do_dir(argv[1], &argv[2]);

	return 0;
}



void printTest(const char *text){
	char* tmp = (char*)text;
	printf("_____");
	while(*tmp != '\0'){
	printf("%c",*tmp);
	tmp++;
	}
	printf("_____\n");
}




void do_dir(const char * dir_name,  char * parms[]){
	

	
	
	//printTest(dir_name);

	//char* dirname = "/var/tmp/test-find/simple/";
	//char* filename = "%*u";
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

	if(currentDirectory == NULL) exit(1); // Error Opndir
	
	struct dirent *currentDirEnt = NULL;
	while((currentDirEnt = readdir(currentDirectory)) != NULL){
		int print = 1;
		int found = fnmatch(filename, currentDirEnt->d_name, 0);
		
		if(parms[0] != NULL){
			if( strcmp(parms[0],"-user") == 0){

				char ganzerPfad[1024] = {'\n'};
				
				
				struct passwd *userInfo = getpwnam(parms[1]);
				
				
				strcpy(ganzerPfad,dirname);
				strcat(ganzerPfad,currentDirEnt->d_name);
				uid_t userid =  userInfo->pw_uid;  
				
				int fd = open(ganzerPfad, O_RDONLY);
				

				if(fd >= 0) { 
					struct stat buffer;
					fstat(fd, &buffer);
					if(buffer.st_uid != userid){
						print = 0;
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



