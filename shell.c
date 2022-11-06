#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

struct dirent* dir;

/*

*/

void sigHandler(int signum) {
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);

    switch (signum) {
        //SIGINT
        case 2:
            break;
        //SIGTSTP
        case 20:
            break;
        default:
            break;
        
    }
    // return 0;
}


/*
    Reads user input, ignoring whitespace from user.
    Returns an array of strings.
    Last element in array is a NULL pointer because execv expects this.
    Length parameter is actually 1 less than the real length of array,
    i.e. this is the length if you do not count the NULL pointer
    (makes printing look nicer)
*/
char** readInput(int* length) { 
    char** args = malloc(sizeof(char *));
    int i = 0;
    char c;
    while(!feof(stdin)) {
        int j = 2;
        args[i] = malloc(j * sizeof(char));
        scanf("%c", &c);
        if (c == '\n' && !feof(stdin)) {
            args[i] = NULL;
            *length = i;
            return args;
        }
        while (c == ' ' && !feof(stdin)) {
            scanf("%c", &c);
        }
        while(c != ' ' && c != '\n' && !feof(stdin)) {
            args[i][j-2] = c;
            j++;
            args[i] = realloc(args[i], j * sizeof(char));
            scanf("%c", &c);
        }
        i++;
        args = realloc(args, (i+1) * sizeof(char *));
        if (c == '\n' && !feof(stdin)) {
            free(args[i]);
            args[i] = NULL;
            *length = i;
            return args;
        }
    }
    return args;
}

int background(char** args, int length) {
    if (args[length - 1][0] == '&') {
        args[length - 1] = NULL;
        return 1;
    }
    return 0;
}

int printDirs(DIR* dirp, char* root, char* key, char** updatedPath){
	errno = 0;
    bool fileFound = false;
	dir = readdir(dirp);
	while(dir != NULL){
		if(strstr(dir->d_name, key) && dir->d_name[0] != '.'){
            int rootLength = strlen(root);
			char* temp = malloc(sizeof(char) * 256);
			int i = 0;
            fileFound = true;

            while(root[i] != '\0'){
				temp[i] = root[i];
				i++;
			}

			while(dir->d_name[i - rootLength] != '\0'){
				temp[i] = dir->d_name[i - rootLength];
				i++;
			}
            temp[i++] = '/';
			temp[i] = '\0';
            *updatedPath = temp;
		}
		if(dir->d_name[0] != '.' && dir->d_type == 4){
			int rootLength = strlen(root);
			char* temp = malloc(sizeof(char) * 256);
			int i = 0;
			
			while(root[i] != '\0'){
				temp[i] = root[i];
				i++;
			}

			while(dir->d_name[i - rootLength] != '\0'){
				temp[i] = dir->d_name[i - rootLength];
				i++;
			}
			
			temp[i++] = '/';
			temp[i] = '\0';

			DIR* newDir = opendir(temp);
			if (printDirs(newDir, temp, key, updatedPath) == 1 || fileFound) {
                fileFound = true;
            }
			closedir(newDir);
			free(temp);
		}
		dir = readdir(dirp);
	}
	if(errno){
		printf("error!");
	}

    if (fileFound) {
        return 0;
    } else {
       return 1; 
    }
}

int findFileOrCommand(char** path, char** updatedPath) {
    // printf("%s %s\n", *path, *updatedPath);
    if (path[0][0] != '.' && path[0][0] != '/') {
        char* filepath = "/usr/bin/";
        DIR* dirp = opendir(filepath);
        if (!printDirs(dirp, filepath, path[0], updatedPath)) {
            closedir(dirp);
            return 1;
        } else {
            closedir(dirp);
        }
    // printf("%s %s\n", *path, *updatedPath);

        filepath = "/bin/";
        dirp = opendir(filepath);
        if (!printDirs(dirp, filepath, path[0], updatedPath)) {
            closedir(dirp);
            return 1;
        } else {
            closedir(dirp);
        }
    // printf("%s %s\n", *path, *updatedPath);
        //update return;
        return 0;
    } else if (path[0][0] == '.') {
        path[0] = &(path[0][1]);
        return 1;
    }
    return 0;
}

void executeChildProcess(char* args, char** argsList) {
    if (strcmp(args, "bg") == 0) {
        // Input bg <jobID>: Run a suspended job in the background

    } else if (strcmp(args, "cd") == 0) {
        // cd [path]: Changes current directory to the given absolute or relative path. 
        //           If no path is give, use the value of environment variable HOME.
        //           Your shell should update the environment variable PWD with the
        //           (absolute) present working directory after running cd.

    } else if (strcmp(args, "exit") == 0) {
        // exit: Exit the shell. The shell should also exit if the user hits ctrl-d on
        //       on an empty input line. When the shell exist, it should first send SIGHUP
        //       followed by SIGCONT to any stopped jobs, and SIGHUP to any running jobs.
        
    } else if (strcmp(args, "fg") == 0) {
        // fg <jobID>L Run a suspended or background job in the foreground

    } else if (strcmp(args, "jobs") == 0) {
        // jobs: List current jobs, including their jobID, processID, current status, and
        //       command. If no jobs exist, this should print nothing.

    } else if (strcmp(args, "kill") == 0) {
        // kill <jobID>: Send SIGTERM to the given job.

    } else {
        char* foundPath = NULL;
        if (findFileOrCommand(&args, &foundPath) == 0) {
            if (args[0] == '.' || args[0] == '/') {
                printf("%s: No such file or directory\n", args);
            } else {
                printf("%s: command not found\n", args);
            }
        } else {
            printf("%s %s\n", *path, *updatedPath);
            execv(foundPath, argsList);
        }
    }
}

int main() {
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);
    int i = 1;
    pid_t* pid = malloc(sizeof(pid_t));
    while(!feof(stdin)) {
        printf("> ");
        int length = 0;
        int status = 0;
        int bg = 0;
        pid_t tempPid;
        char** args = readInput(&length);
        if (length > 1 && args[0] != NULL) {
            bg = background(args, length);
        }
        if ((tempPid = fork()) == 0) {
            // for (int i = 0; i < length; i++) {
            //     printf("%s  ", args[i]);
            // }
            // printf("%s  %d   ", args[0], length);
            executeChildProcess(args[0], args);
            exit(status);
            pid[i-1] = tempPid;   
        }
        
        // Displays BG process Created
        if (!bg) {
            waitpid(tempPid, &status, 0);
        }
        else {
            printf("[%d] %d\n", i, tempPid);
        }
            
        i++;
        pid = realloc(pid, i * sizeof(pid_t));
        if (feof(stdin)) {
            executeChildProcess("exit", args);
        }
    }    
    return 0;
}