/**
 * @file myfind.c
 * Betriebssysteme myfind
 * Beispiel 1
 *
 * @author Adam Kerenyi <ic14b080@technikum-wien.at>
 * @author Romeo Beck <ic14b037@technikum-wien.at>
 * @author Thomas Zeitinger <ic14b033@technikum-wien.at>
 * @date 2015/02/09
 *
 * @version 100
 *
 */

/*
 * -------------------------------------------------------------- includes --
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <fnmatch.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>

/*
 * --------------------------------------------------------------- defines --
 */

#define NAME_MAX	255
#define PATH_MAX	4096

#define MATCH		1
#define MISMATCH	0

#define NOPARAMETER (parms[parm_pos] == NULL)
#define HASNEXTARG (parm_pos + 1 < argc)
#define NAME (strcmp(parms[parm_pos], "-name") == 0)
#define PATH (strcmp(parms[parm_pos], "-path") == 0)
#define USER (strcmp(parms[parm_pos], "-user") == 0)
#define NOUSER (strcmp(parms[parm_pos], "-nouser") == 0)
#define TYPE (strcmp(parms[parm_pos], "-type") == 0)
#define LS (strcmp(parms[parm_pos], "-ls") == 0)
#define PRINT (strcmp(parms[parm_pos], "-print") == 0)

#define CHECK (check(dir_name,file,parms,parm_pos))

/*
 *  * ------------------------------------------------ function prototypes --
 */

void usage (void);
void do_file(const char * file_name, const char * const * parms);
void do_dir(const char * dir_name, const char * const * parms);	
int check(const char * file_name, struct stat file, const char * const * parms, int parm_pos);
int check_user(struct stat fd_in, const char * const * parms, int parm_pos);
void check_arg(const int argc, const char * argv[]);
void check_arg_error(void);
int check_arg_type(const char * argv);
int check_nouser(struct stat fd_in);
int check_type(struct stat file, const char * const * parms, int parm_pos);
int check_name(const char * file_name, const char * const * parms, int parm_pos);
int check_path(const char * file_name, const char * const * parms, int parm_pos);
int print_ls(const char * file_name, struct stat file);
int print(const char * file_name);
void myprintf(char *format, ...);
void itoa(int num, char str[]);
void reverse(char str[]);

/**
 *
 * \brief main function for myfind, a sipmle version of find
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return always "success"
 * \retval 0 always
 *
 */

int main(int argc, const char *argv[]) {

	/* Declare variables, need as const array of all parameters */
	const char * const *paramlist = (const char * const *)&argv[2];
	const char *filename = (const char *)argv[1];

	/* Check if parameters are correct */
	check_arg(argc, argv);

	/* check (all) first dirs */
	do_file(filename, paramlist);

	exit(EXIT_SUCCESS);
}

/**
 *
 * \brief check_arg checks the validity of the entered arguments depending on the action
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return void
 *
 */


void check_arg(const int argc, const char * argv[])
{
	const char * const * parms = argv;
	int parm_pos = 2;

	if (argc < 2) {
		check_arg_error();
	}

	while (parm_pos < argc) {
		if (NAME && HASNEXTARG) {
			if (parm_pos + 2 < argc) {
                                parm_pos +=2;
                        } else {
                                break;
			}
		} else if (PATH && HASNEXTARG) {
			if (parm_pos + 2 < argc) {
				parm_pos +=2;
			} else {
				break;
			}
		} else if (USER && HASNEXTARG) {
			if (parm_pos + 2 < argc) {
                                parm_pos +=2;
                        } else {
                                break;
			}
		} else if (TYPE && HASNEXTARG) {
			if (check_arg_type(argv[parm_pos + 1])) {
				if (parm_pos + 2 < argc) {
                                	parm_pos +=2;
                        	} else {
                                	break;
				}
			} else {
				check_arg_error();
			}
		}
		else if (NOUSER) {
			if (parm_pos + 1 < argc) {
                                parm_pos++;
                        } else {
                                break;
			}
		} else if (PRINT) {
			if (parm_pos + 1< argc) {
                                parm_pos++;
                        } else {
                                break;
                        }
		} else if (LS) {
			if (parm_pos + 1 < argc) {
                                parm_pos++;
                        } else {
                                break;
                        }
		} else check_arg_error();
	}
}

/**
 *
 * \brief check_arg_error prints the usage in case of invalid arguments
 *
 * \param void no parameters
 * 
 * \return void
 *
 */


void check_arg_error(void)
{
	errno = EINVAL;
	usage();
	error(1, 1, "%d", errno);
}

/**
 * 
 * \brief check_arg_type checks the arguments for action -type
 *
 * \param *argv argument entered for -find 
 *
 * \return MATCH if argument is valid, MISMATCh if argument isn't valid
 * \retval 1 if argument is valid
 * \retval 0 if argument isn't valid
 *
 */

int check_arg_type(const char * argv) {
   	switch ((char) * argv) {
		case 'b':
		case 'c':
		case 'd':
		case 'p':
		case 'f':
		case 'l':
		case 's':
			return MATCH;
			break;
		default:
			return MISMATCH;	
	}
}

/**
 * 
 * \brief do_dir opens and reads a directory, excludes . and .., prepares the path and calls do_file  
 *
 * \param *dir_name name of the directory
 * \param *parms actions and arguments entered by the user
 *
 * \return void
 *
 */


void do_dir(const char * dir_name, const char * const * parms) {
	DIR *dir;
	struct dirent *d;
	char filename[PATH_MAX];

	if ((dir = opendir(dir_name)) == NULL) {
		if (errno != EACCES) {
			error(1, 1, "%d", errno);
			exit(EXIT_FAILURE);
		}
	} else {
		while ((d = readdir(dir)) != NULL) {
			if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) continue;
			else {

				strcpy(filename, dir_name);
				/* if there is already a /, dont catenate again */
				if (dir_name[strlen(dir_name) - 1] != '/') {
					strcat(filename, "/");
				}
				strcat(filename, d->d_name);

				do_file(filename,parms);
			}

		}
	}

	closedir(dir);
}

/**
 * 
 * \brief d_file reads file details, calls check (which checks the file for all arguments), prints if no parameter was entered and calls do_dir if the file is a directory
 *
 * \param *dir_name name of the file
 * \param *parms actions and arguments entered by the user
 *
 * \return void
 *
 */


void do_file(const char * file_name, const char * const * parms) {
	int parm_pos = 0;
	/* create filedescriptor */
	struct stat fd_in;

	/* lstat file and write content to struct
	 * throw error if failed with Filename */
	if (lstat(file_name, &fd_in) == -1) {
		error(1, 1, "%d", errno);
		exit(EXIT_FAILURE);
	}

	
	check(file_name, fd_in, parms, parm_pos);
	if NOPARAMETER {
		print(file_name);}
	if (S_ISDIR(fd_in.st_mode)) { do_dir(file_name,parms); }
}

/**
 * 
 * \brief check checks a file if it meets the given arguments and calls print if it does
 *
 * \param *dir_name name of the file
 * \param file file detail (from lstat)
 * \param parms actions and arguments entered by the user
 * \param parm_pos current action (which is checked)
 *
 * \return calls itself until a check failed or there isn't anything to check left, then returns MISMATCH
 * \retval 0 if a check failed or there isn't anything to check left
 *
 */


int check(const char * dir_name, struct stat file, const char * const * parms, int parm_pos) {

	if NOPARAMETER {
		return MISMATCH;
	}
	else if NAME {
		if (check_name(dir_name,parms,parm_pos)) {
			parm_pos=parm_pos+2;
			if (!CHECK) print(dir_name);
		}
	}
	else if PATH {
		if (check_path(dir_name,parms,parm_pos)) {
			parm_pos=parm_pos+2;
			if (!CHECK) print(dir_name);
		}
	}
	else if USER {
		if (check_user(file,parms,parm_pos)) {
			parm_pos=parm_pos+2;
			if (!CHECK)  print(dir_name);
		}
	}
	else if NOUSER {
		if (check_nouser(file)) {
		       parm_pos++;
		       if (!CHECK) print(dir_name);
		}
	}
	else if TYPE {
		if (check_type(file, parms, parm_pos)) {
			parm_pos=parm_pos+2;
			if (!CHECK) print(dir_name);
		}
	}
	else if LS {
		if (print_ls(dir_name, file)) {
			parm_pos=parm_pos+2;
			CHECK;
		}
	}
	else if PRINT {
		if (print(dir_name)) {
			parm_pos=parm_pos+1;
			CHECK;
		}
	}

	return MISMATCH;
}

/**
 * 
 * \brief check_name checks for action -name if the entered name is the name of the file
 *
 * \param *file_name name of the file
 * \param parms actions and arguments entered by the user
 * \param parm_pos current action (which is checked)
 *
 * \return MATCH if the entered name is the file's name, MISMATCH if not
 * \retval 0 if the entered name isn't the file's name
 * \retval 1 if the entered name is the file's name
 *
 */


int check_name(const char * file_name, const char * const * parms, int parm_pos) {
	char *basec;
	char *basen;

	basec = strdup(file_name);
	basen = basename(basec);

	if(fnmatch(basen,parms[parm_pos+1],FNM_NOESCAPE) == 0) {
		return MATCH;
	} else {
		return MISMATCH;
	}

}

/**
 * 
 * \brief check_patch checks for action -path if the entered patch is the name of the file's path
 *
 * \param *file_name name of the file
 * \param parms actions and arguments entered by the user
 * \param parm_pos current action (which is checked)
 *
 * \return MATCH if the entered path is the file's path, MISMATCH if not
 * \retval 0 if the entered path isn't the file's path
 * \retval 1 if the entered path is the file's path
 *
 */


int check_path(const char * file_name, const char * const * parms, int parm_pos) {

	if(fnmatch(parms[parm_pos+1],file_name,FNM_NOESCAPE) == 0) {
		return MATCH;
	} else {
		return MISMATCH;
	}

}

/**
 * 
 * \brief check_user checks for action -user if the entered user (name or UID) is the owner of the file and checks if the user (if a name was entered exists) 
 *
 * \param fd_in file details (from lstat)
 * \param parms actions and arguments entered by the user
 * \param parm_pos current action (which is checked)
 *
 * \return MATCH if the entered user is the file's owner, MISMATCH if not
 * \retval 0 if the entered user isn't the file's owner
 * \retval 1 if the entered user is the file's owner
 *
 */


int check_user(struct stat fd_in, const char * const * parms, int parm_pos)
{
	struct passwd *username = NULL;
	char *endptr = NULL;
	int char2int = 0;

	username = getpwnam(parms[parm_pos +1]);
	if (username == NULL) {
		char2int = strtol(parms[parm_pos + 1], &endptr, 10);
		if (char2int == 0) {
			error(0, 0, "%s is not the name of a known user", parms[parm_pos +1]);
			/* 
 			 * According to script this should be printed on stdout, however
 			 * as it is an error we print it on stderr
 			 */
			return MISMATCH;
		}
		username = getpwuid(char2int);
		if (username == NULL ) {
			error(0, 0, "%s is not the name of a known user", parms[parm_pos +1]);
			return MISMATCH;
		}
	}
	/* compare entered user with file's owner name */
	if ((fd_in.st_uid) == (username->pw_uid)) return MATCH;
	return MISMATCH;
}

/**
 * 
 * \brief check_nouser checks if a file's owner is still an existing user 
 *
 * \param fd_in file details (from lstat)
 *   
 * \return MATCH if the file's owner doesn't exist, MISMATCH if it exists
 * \retval 0 if the file's owner exists
 * \retval 1 if the file's owner doesn't exist
 *
 */

int check_nouser(struct stat fd_in)
{
	struct passwd *userdet = NULL;
	/* get user data with UID */
	userdet = getpwuid(fd_in.st_uid);
	/* return MATCH is user doesn't exist */
	if(userdet == NULL) return MATCH;
	else return MISMATCH;

}

/**
 * 
 * \brief check_type checks vor action -type if the entered type is the same as the file's type
 *
 * \param file file details (from lstat)
 * \param parms actions and arguments entered by the user
 * \param parm_pos current action (which is checked)
 * \return MATCH if the the entered type is the same as the file's type, MISMATCH if it isn't the same
 * \retval 0 if the the entered type isn't the same as the file's type
 * \retval 1 if the the entered type is the same as the file's type
 * 
 */


int check_type(struct stat file, const char * const * parms, int parm_pos) {
	char type = (char) *parms[parm_pos + 1];

	switch (type) {
		case 'b':
			if (S_ISBLK(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 'c':
			if (S_ISCHR(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 'd':
			if (S_ISDIR(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 'p':	
			if (S_ISFIFO(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 'f':
			if (S_ISREG(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 'l':
			if (S_ISLNK(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		case 's':
			if (S_ISSOCK(file.st_mode)) {
				return MATCH;
			} else {
				return MISMATCH;
			}
			break;
		default:
			error(1, 1, "%d", errno);
			return MISMATCH;
	}
}

/**
 * 
 * \brief action -ls: prints the file's details 
 *
 * \param file_name name of the file 
 * \param file file details (from lstat)
 *
 * \return MATCH after printing
 * \retval 1 after printing
 * 
 */

int print_ls(const char * file_name, struct stat file) {
	/*20696685	8 -rw-r--r--	1 akerenyi	staff	1453 Mar  2 18:19 ./Makefile*/

	struct passwd *passwd = getpwuid(file.st_uid);
	struct group *group = getgrgid(file.st_gid);
	char mtime[NAME_MAX];
	struct tm *time;
	char user_to_print[NAME_MAX];
	char group_to_print[NAME_MAX];
	/*char buffer[NAME_MAX];*/

	if (!passwd->pw_name) {
		sprintf(user_to_print,"%d",file.st_uid);
		fprintf(stdout, "%s", user_to_print);
	} else {
		strcpy(user_to_print, passwd->pw_name);
	}

	if (!group->gr_name) {
		sprintf(group_to_print,"%d", file.st_gid);
		fprintf(stdout, "%s", group_to_print);
	} else {
		strcpy(group_to_print, group->gr_name);
	}

	time = localtime(&file.st_mtime);
	strftime(mtime, 1000, "%b %d %H:%M", time);
	/* correct day format would require %e however it is not allowed within the C90 standard*/

	myprintf(" %ld %4.0f ",
		(long) file.st_ino,
		(double)file.st_blocks);
	/* standard st_blocks size is 512 Bytes altough divinding by 2 would result in
 	 * correct number, according to our knowledge result should be shown in 
 	 * 512 Bytes, so we left it unaltered
 	 */
	
        myprintf((S_ISDIR(file.st_mode)) ? "d" : "-");
        myprintf((file.st_mode & S_IRUSR) ? "r" : "-");
        myprintf((file.st_mode & S_IWUSR) ? "w" : "-");
	if (file.st_mode & S_IXUSR) {
		if (file.st_mode & S_ISUID) myprintf("s");
		else myprintf("x");
	} else if (file.st_mode & S_ISUID) {
		myprintf("S");
	} else myprintf("-");
	myprintf((file.st_mode & S_IRGRP) ? "r" : "-");
	myprintf((file.st_mode & S_IWGRP) ? "w" : "-");
	if (file.st_mode & S_IXGRP) {
		if (file.st_mode & S_ISGID) myprintf("s");
		else myprintf("x");
	} else if (file.st_mode & S_ISGID) {
		myprintf("S");
	} else myprintf("-");
	myprintf((file.st_mode & S_IROTH) ? "r" : "-");
	myprintf((file.st_mode & S_IWOTH) ? "w" : "-");
	if (file.st_mode & S_IXOTH) {
		if (file.st_mode & S_ISVTX) myprintf("t");
		else myprintf("x");
	} else if (file.st_mode & S_ISVTX) {
		myprintf("T");
	} else myprintf("-");
	myprintf(" %3ld %s %8s %12.0f %s %s",
		(long) file.st_nlink,		/* number of links */
		user_to_print,			/* user name */
		group_to_print,			/* group name */
		(double) file.st_size,		/* file size */
		mtime,				/* last modification date */
		file_name			/* file name */
	);

	if (S_ISLNK(file.st_mode)) {
		char buffer[PATH_MAX];
		int count = readlink(file_name, buffer, sizeof(buffer));
		if (count >= 0) {
			buffer[count] = '\0';
			myprintf(" -> %s\n", buffer);
		}
	} else {
		myprintf("\n");
	}
	return MATCH;
}

/**
 * 
 * \brief prints the checked file's path'
 *
 * \param file_name name of the file 
 *
 * \return MATCH after printing
 * \retval 1 after printing
 * 
 */

int print(const char * file_name) {
	myprintf("%s\n", file_name);
	return MATCH;
}

/**
 * 
 * \brief wrapper for printf
 *
 * \param format
 * \brief wrapper for printf with error checking
 *
 * \param formatstrings
 *
 */

void myprintf(char *format, ...) {
   va_list args;

   va_start(args, format);
   if (vprintf(format, args) < 0) error(1, 1, "%d", errno);
   va_end(args);
}

/**
 * 
 * \brief prints the usege if entered arguments are not valid
 *
 * \return void
 * 
 */

void usage (void) {
	fprintf(stderr, "\nUsage: myfind <directory> [ <options> ] ...\n\n");
	fprintf(stderr, "myfind is a simple command line tool for getting files and folders\n");
	fprintf(stderr, "under the specified path matching the given options.\n\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-name\t\t<pattern>\tfind files/directories with matching name\n");
	fprintf(stderr, "\t-user\t\t<name>/<uid>\tfind files/directories of a user\n");
	fprintf(stderr, "\t-nouser\t\t\t\tfind files/directories without a user\n");
	fprintf(stderr, "\t-path\t\t<pattern>\tfind files/directories with matchin path (incl. name)\n");
	fprintf(stderr, "\t-type\t\t[bcdpfls]\tfind files/directories with matching type\n");
	fprintf(stderr, "\t-print\t\t\t\tprint name of file/directory to console\n");
	fprintf(stderr, "\t-ls\t\t\t\tprint number of inode, number of blocks, permissions, etc...\n\n");
}

/*
 * =================================================================== eof ==
 */
