/**
* @file myfind.c
* Betriebssysteme My Find File
* Beispiel 1
*
* @author Dominic Schebeck
* @author Dominik Marcel Rychly
* @author Thomas Neugschwandtner
*
* @date 03/13/2018
*
* @version 1.0
*/

/*
* ------------------------------------------------------includes----------
*/

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
#include <err.h>
#include <errno.h>

struct optionItem {
	char *name;
	int params;
};

static void do_dir(const char const * dir_name,  char ** parms);
static void do_file(char * dir_name,  char ** parms);

static uid_t getUidFromString(const char const *id);
static struct optionItem *searchOption(const char const *optName);
static void errorMsg(const int i);
static void warnMsg(const int i);
static void lsprint(const char const *path);
static void spclPrint(const char const *str);

//error/warn enum
enum {	ExpParam,
		ExpOpt,
		ExpOptAdditOpt,
		UnknwnOpt,
		UnknwnUser,
		UnknwnFileType,
		CantOpenDir,
		Closedir_Error,
		LSTAT_Error,
		GetPwUID_Error,
		StrToL_Error,
		GetGrpID_Error,
		StrFTime_Error,
		Localtime_Error,
		Readlink_Error,
		GetPwNam_Error,
		Readdir_Error,
		SmthgWrdHpnd
	 };

/**
* \brief The MyFind C program
*		 This is the main entry point for any C program
*
* \param argc the number of arguments
* \param argv the arguments itselves (including the program name in argv[0])
*
* \return always "success"
* \retval 0 always
*/

int main(const int argc, char *argv[])
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
					if(expParam > 0) errorMsg(ExpParam);
					
					optItem = searchOption(argv[i]);
					if(optItem == NULL) errorMsg(UnknwnOpt);
					
					optionPos = i; // Store Option Position
					expParam = optItem->params;
					
				}else{
					//Parameter
					if(expParam == 0) errorMsg(ExpOpt);
					expParam--;
			
				}
				break;	
		}	

		if(optItem != NULL && optionPos != 0 && expParam == 0){
			if(strcmp(optItem->name,"-user") == 0){
				if(getUidFromString(argv[( optionPos + 1 )]) == 0) errorMsg(UnknwnUser);
			}
			
			if(strcmp(optItem->name,"-type") == 0){
				char allowedFileTypes[] = "fdbcpls";
				char *validType = memchr(allowedFileTypes, argv[( optionPos + 1 )][0], strlen(allowedFileTypes));
				
				if(validType == NULL) errorMsg(UnknwnFileType);
			}
		}
	}
	
	if(expParam > 0) errorMsg(ExpOptAdditOpt);
	do_file(argv[1], &argv[2]);	
	return 0;
}

/**
* \brief prints error massages
*
* \param i -> error case
*/

static void errorMsg(const int i){

	
	if(errno != 0){
		errx(errno, strerror(errno));
	}else{

		switch(i) {
			
			case ExpParam:			errx(99, "Parameter is missing"); break;
			case ExpOpt:			errx(99, "Option is missing"); break;
			case ExpOptAdditOpt:	errx(99, "Option is missing"); break;
			case UnknwnOpt:			errx(99, "Unknown option"); break;
			case UnknwnUser:		errx(99, "Unknown user"); break;
			case UnknwnFileType:	errx(99, "Unknown file-type"); break;
			case CantOpenDir:		errx(99, "Can not open directory"); break;
			case Closedir_Error:	errx(99, "Can not close directory"); break;
			case LSTAT_Error:		errx(99, "LSTAT Error"); break;
			case GetPwUID_Error:	errx(99, "GETPWUID Error"); break;
			case StrToL_Error:		errx(99, "STRTOL Error"); break;
			case GetGrpID_Error:	errx(99, "GETGRPID Error"); break;
			case StrFTime_Error:	errx(99, "STRFTIME Error"); break;
			case Localtime_Error:	errx(99, "LOCALTIME Error"); break;
			case Readlink_Error:	errx(99, "READLINK Error"); break;
			case GetPwNam_Error:	errx(99, "GETPWNAME Error"); break;
			case Readdir_Error:		errx(99, "READDIR Error"); break;
			default:				errx(999, "something weird happened"); break;
		}
	}
}

/**
* \brief prints warn massages
*
* \param i -> error case
*/

static void warnMsg(const int i){
	
	if(errno != 0){
		warn(strerror(errno));
	}else{

		switch(i) {
			
			case ExpParam:			warn("Parameter is missing"); break;
			case ExpOpt:			warn("Option is missing"); break;
			case ExpOptAdditOpt:	warn("Option is missing"); break;
			case UnknwnOpt:			warn("Unknown option"); break;
			case UnknwnUser:		warn("Unknown user"); break;
			case UnknwnFileType:	warn("Unknown file-type"); break;
			case CantOpenDir:		warn("Can not open directory"); break;
			case Closedir_Error:	warn("Can not close directory"); break;
			case LSTAT_Error:		warn("LSTAT Warn"); break;
			case GetPwUID_Error:	warn("GETPWUID Warn"); break;
			case StrToL_Error:		warn("STRTOL Warn"); break;
			case GetGrpID_Error:	warn("GETGRPID Warn"); break;
			case StrFTime_Error:	warn("STRFTIME Warn"); break;
			case Localtime_Error:	warn("LOCALTIME Warn"); break;
			case Readlink_Error:	warn("READLINK Warn"); break;
			case GetPwNam_Error:	warn("GETPWNAME Warn"); break;
			case Readdir_Error:		warn("READDIR Warn"); break;
			default:				warn("something weird happened"); break;
		}
	}
	
	errno = 0;
	
}

/**
* \brief calls do_file for every child of directory
*
* \param dir_name full path of directory
* \param parms filter and print parameter
*/

static void do_dir(const char const *dir_name,  char ** parms){
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);	
	DIR *directory = opendir(dir_name);
	if(directory == NULL){
		warnMsg(CantOpenDir);
		return;
	}else{
		struct dirent *direntry;
		
		if(errno != 0) errorMsg(SmthgWrdHpnd);
		while((direntry = readdir(directory))){
			
			size_t newFileLen = strlen(dir_name)+ 1 +strlen(direntry->d_name) + 1 ;
			char newFile[newFileLen * sizeof(char)];
			
			if( strcmp(direntry->d_name,".") == 0 ) continue;
			if( strcmp(direntry->d_name,"..") == 0 ) continue;
			
			strcpy(newFile, dir_name);
			strcat(newFile, "/");
			strcat(newFile, direntry->d_name);
			do_file(newFile,parms);

		}
		if(errno != 0) errorMsg(Readdir_Error);
	}
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	int clRes = closedir(directory);
	if(clRes == -1) errorMsg(Closedir_Error);
}

/**
* \brief prints file if every filter matches 
*		  returns as soon as one filter does not match and
*		  additionally calls do_file if file is a directory
*
* \param dir_name full path of directory
* \param parms filter and print parameter
*/ 

static void do_file(char * dir_name,  char **parms){

	int print = 1;
	int smthPrinted = 0;
	int currentParam = 0;
	struct stat newStatBuffer;
	char *fileName = basename(dir_name);
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);	
	int lstatMsg = lstat(dir_name, &newStatBuffer);
	if (lstatMsg == -1) errorMsg(LSTAT_Error);

	
	while(parms[currentParam] != NULL  && print == 1){
		int cParam = 0;
		
		if( strcmp(parms[currentParam],"-user") == 0){
			uid_t userid = getUidFromString(parms[currentParam + 1]);
			if(newStatBuffer.st_uid != userid)	print = 0;
			cParam += 2;
		}
		
		if( strcmp(parms[currentParam],"-nouser") == 0){
			if(errno != 0) errorMsg(SmthgWrdHpnd);
			if(getpwuid(newStatBuffer.st_uid) != NULL ) print = 0;
			else if(errno != 0) errorMsg(GetPwUID_Error);
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

/**
* \brief check if option is supported
*
* \param option string
*
* \return if successful returns pointer to struct optionItem
* \retval pointer to struct
* \retval NULL
*/

static struct optionItem *searchOption(const char const *optName){
	static struct optionItem optArray[] = {
		{.name="-user",.params=1},
		{.name="-name",.params=1},
		{.name="-type",.params=1},
		{.name="-print",.params=0},
		{.name="-ls",.params=0},
		{.name="-nouser",.params=0},
		{.name="-path",.params=1}
	};
	
	int optionItemLen = (sizeof(optArray)/sizeof(optArray[0]));
	
	for(int i = 0;i<optionItemLen;i++){
		if(strcmp(optArray[i].name,optName) == 0) return &optArray[i];
	}
	
	return NULL;
}

/**
* \brief get User-ID from string
*
* \param string contains name or id
*
* \return User-ID or 0
* \retval uid
* \retval 0
*/

static uid_t getUidFromString(const char const *id){
	struct passwd *userinfo;
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	userinfo = getpwnam(id);
	if(userinfo == NULL && errno != 0) errorMsg(GetPwNam_Error);
	
	if(userinfo != NULL)	return userinfo->pw_uid;
	
	int tmpi = 0;
	while(id[tmpi] != '\0'){
		if(isdigit(id[tmpi]) == 0) return 0;
		tmpi++;
	}
	
	
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	long int userid = strtol(id, NULL, 10);
	if(errno != 0) errorMsg(StrToL_Error);
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	userinfo =  getpwuid(userid);
	if(userinfo == NULL && errno != 0) errorMsg(GetPwUID_Error);
	
	if(userinfo != NULL)	return userinfo->pw_uid;
	
	return 0;
}

/**
* \brief prints file in -dils style
*
* \param directory path
*/

static void lsprint(const char const *path){
	struct stat buf;
	
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	int statRes = lstat(path, &buf);
	if(statRes == -1) errorMsg(LSTAT_Error);
	
	struct passwd *userInfo;
	struct group *groupInfo;

	
	
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
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	userInfo =  getpwuid(buf.st_uid);
	if(userInfo == NULL && errno != 0) errorMsg(GetPwUID_Error);
	
	printf("%-8s", (userInfo!=NULL) ? userInfo->pw_name : "999999");		
	printf(" ");		

	//Groupname
	if(errno != 0) errorMsg(SmthgWrdHpnd);
	groupInfo = getgrgid(buf.st_gid);
	if(groupInfo == NULL && errno != 0) errorMsg(GetGrpID_Error);
	
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
	struct tm *lcTime = localtime(&time);
	if(lcTime == NULL) errorMsg(Localtime_Error);
		
	size_t strftRes = strftime(buff, 20, "%b %e %H:%M", lcTime);
	if(strftRes == 0) errorMsg(StrFTime_Error);
	
	printf("%s",buff);
	printf(" ");
	
	//Path
	
	spclPrint(path);
	
	
	//symlink print 
	
	if(fileType == 'l'){
		char linkname[buf.st_size + 1];
		
		if(errno != 0) errorMsg(SmthgWrdHpnd);
		ssize_t r = readlink(path, linkname, buf.st_size + 1);
		if(r == -1) errorMsg(Readlink_Error);
		
		linkname[r] = '\0';
		
		printf(" -> %s",linkname);
	}
	
	printf("\n");
}

/**
* \brief prints the whole string and
*		 escapes blanks and backslashes with backslash
*
* \param string to print
*/

static void spclPrint(const char const *str){
	while(*str != '\0'){
		if(*str == '\\') printf("\\\\");
		else if(*str == ' ') printf("\\ ");
		else printf("%c",*str);
		str++;
	}
	
}

//------------------------------------eof--------------------------------------------------
