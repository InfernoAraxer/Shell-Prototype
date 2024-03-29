#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>

// Structure available to navigate files in the /usr/bin/ and /bin/ folders
struct dirent* dir;

typedef struct job job;
struct job{
	pid_t pid;
    int jobID;
	int status; //1 for Running, 2 for Stopped, 3 for Terminated
	char* command;
    int ground;
	job* next;
};

volatile sig_atomic_t chldflag = 0;

void Signal(int signum, void *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /* Block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
        printf("Signal error");
}

// Repeatedly handles Ctrl+Z and Ctrl+C Inputs
    // We will have to update the functions so they will be able to terminate
    // jobs that are in the fore/background.
void sigHandler(int signum) {

	int errtemp = errno;
	sigset_t mask, prev;
	sigfillset(&mask);
    Signal(SIGTERM, sigHandler);
    sigprocmask(SIG_BLOCK, &mask, &prev);

    switch (signum) {
        //SIGTERM = Case 
        case SIGTERM:
            _Exit(0);

		case SIGCHLD:
			errno = errtemp;
            chldflag = 1;
			break;
        default:
            break;
        
    }
    sigprocmask(SIG_SETMASK, &prev, NULL);
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
    while(1) {
        int j = 2;
        args = realloc(args, (i+1) * sizeof(char *));
        args[i] = malloc(j * sizeof(char));
        scanf("%c", &c);
        if (feof(stdin)) {
            *length = 0;
            for (int k = 0; k < i + 1; k++) {
                free(args[i]);
            }
            return args;
        }
        if (c == '\n' && !feof(stdin)) {
            free(args[i]);
            args[i] = NULL;
            *length = i;
            return args;
        }
        while (c == ' ' && !feof(stdin)) {
            scanf("%c", &c);
        }
        if (c == '\n' && !feof(stdin)) {
            free(args[i]);
            args[i] = NULL;
            *length = i;
            return args;
        }
        while(c != ' ' && c != '\n' && !feof(stdin)) {
            args[i][j-2] = c;
            j++;
            args[i] = realloc(args[i], j * sizeof(char));
            scanf("%c", &c);
        }
        args[i][j-2] = '\0';
        i++;
        if (c == '\n' && !feof(stdin)) {
            if (j == 2) {
                free(args[0]);
                i-=1;
            }
            *length = i;
            return args;
        }
    }
}

int background(char** args, int length) {
    if (args[length - 1][0] == '&') {
        free(args[length - 1]);
		args[length - 1] = NULL;
		free(args[length]);
        return 1;
    }
    return 0;
}



//Status Key
char* getStatus(int key){
	switch (key){
	case 1:
		return "Running";
		break;
	case 2:
		return "Stopped";
	case 3:
		return "Terminated";
	default:
		return "Invalid Status";
		break;
	}
}

// Main
int main() {
	sigset_t childmask, parentmask, prev;
	sigemptyset(&childmask);
    sigemptyset(&parentmask);
    sigaddset(&parentmask, SIGINT);
    sigaddset(&parentmask, SIGTERM);
    sigaddset(&parentmask, SIGTSTP);
    sigaddset(&childmask, SIGCHLD);

	Signal(SIGCHLD, sigHandler);
    sigprocmask(SIG_BLOCK, &parentmask, &prev);
    // int i = 1;
    // pid_t* pid = malloc(sizeof(pid_t));
    job* jobList = malloc(sizeof(job));
    jobList->status = 0;
    char* home = getenv("HOME");
    setenv("PWD", home, 1);
    int length = 0;
    int status = 0;
    // int bg = 0;
    char** args;
    while(!feof(stdin)) {
        printf("> ");
        length = 0;
        status = 0;
        // bg = 0;
        pid_t tempPid;
        args = NULL;
        char* pwd = getenv("PWD");
        chdir(pwd);

        //read input
        if (!feof(stdin)) {
            args = readInput(&length);
        }

        if (feof(stdin)) {
            free(args);
            break;
        }

        args = realloc(args, (length + 1) * sizeof(char*));
        args[length] = NULL;

        //if empty
        if (length == 0 && args[0] == NULL) {
            free(args[0]);
            free(args);
            continue;
        }

        if (chldflag) {
            fflush(stdout);
            job* ptr = jobList;
			job* prev = NULL;
            while(ptr->status > 0) {
				int waitstatus = 1;
				errno = 0;
                waitpid(ptr->pid, &waitstatus, WNOHANG);
				if (errno == 10 || WIFEXITED(waitstatus) || waitstatus == 15) {
					if(!prev){
						jobList = ptr->next;
						ptr->status = 3;
						free(ptr->command);
						free(ptr);
						ptr = jobList;
					}
					else{
						prev->next = ptr->next;
						free(ptr->command);
						free(ptr);
						ptr = prev;
					}
				}
				prev = ptr;
				if(ptr->status > 0){
					ptr = ptr->next;
				}
            }
			chldflag = 0;
		}


        // Determines if Process will run in the background
        if (!feof(stdin)) {
            // bg = background(args, length);
            if (!strcmp(args[0], "bg")) {
                // Input bg <jobID>: Run a suspended job in the background
                if (args[1][0] != '%') {
                    printf("Invalid input");
                }
                else {
                    char* tempStr = malloc(strlen(args[1]) * sizeof(char));
                    for(int i = 1; i < strlen(args[1]); i++) {
                        tempStr[i-1] = args[1][i];
                    }
                    tempStr[strlen(args[1]) - 1] = '\0';
                    strcpy(args[1], tempStr);
                    free(tempStr);
                    if (jobList->status != 0) {
                        job* ptr = jobList; 
                        while (ptr->status != 0) {
                            if (ptr->jobID == atoi(args[1])) {
                                kill(ptr->pid, SIGCONT);
                                ptr->status = 1;
                            }
                            ptr = ptr->next;
                        }
                    }
                }

            } else if (!strcmp(args[0], "cd")) {
                // cd [path]: Changes current directory to the given absolute or relative path. 
                //           If no path is give, use the value of environment variable HOME.
                //           Your shell should update the environment variable PWD with the
                //           (absolute) present working directory after running cd.

                
                if (length == 1) {
                    setenv("PWD", home, 1);
                    chdir(home);
                } else if (length > 2) {
                    printf("cd: too many arguments");
                }
                else {
                    char* path = malloc((strlen(pwd) + strlen(args[1]) + 11)*sizeof(char));
                    int exists = -1;
                    struct stat buf;
                    strcpy(path, pwd);
                    if (args[0][0] != '/') {
                        strcat(path, "/");
                    }
                    strcat(path, args[1]);

                    exists = stat(path, &buf);

                    if (!exists && S_ISDIR(buf.st_mode)) {
                        setenv("PWD", path, 1);
                        chdir(path);
                    } else if (!exists) {
                        printf("bash: cd: %s: Not a directory\n", args[1]);
                    } else {
                        printf("bash: cd: %s: No such file or directory\n", args[1]);
                    }
                    free(path);
                }
            } else if (!strcmp(args[0], "exit")) {
                // exit: Exit the shell. The shell should also exit if the user hits ctrl-d on
                //       on an empty input line. When the shell exist, it should first send SIGHUP
                //       followed by SIGCONT to any stopped jobs, and SIGHUP to any running jobs.

                if (jobList->status != 0) {
                    job* ptr = jobList; 
                    while (ptr->status != 0) {
                        kill(ptr->pid, SIGHUP);
                        if (ptr->status == 2) {
                            kill(ptr->pid, SIGCONT);
                        }
                        ptr = ptr->next;
                    }
                }

				for (int i = 0; i < length; i++) {
                    free(args[i]);
                }                
                free(args);
                break;
        
            } else if (!strcmp(args[0], "fg")) {
                // fg <jobID>L Run a suspended or background job in the foreground
                if (args[1][0] != '%') {
                    printf("Invalid input");
                }
                else {
                    char* tempStr = malloc(strlen(args[1]) * sizeof(char));
                    for(int i = 1; i < strlen(args[1]); i++) {
                        tempStr[i-1] = args[1][i];
                    }
                    tempStr[strlen(args[1]) - 1] = '\0';
                    strcpy(args[1], tempStr);
                    free(tempStr);
                    if (jobList->status != 0) {
                    job* ptr = jobList; 
                        while (ptr->status != 0) {
                            if (ptr->jobID == atoi(args[1])) {
                                int status = 0;
                                if (ptr->status == 2) {
                                    kill(ptr->pid, SIGCONT);
                                    waitpid(ptr->pid, &status, WUNTRACED);
                                    if (WIFSTOPPED(status)) {
                                        ptr->status = 2;
                                        printf("\n");
                                    }
                                    if (status == 2) { 
                                        printf("\n[%d] %d terminated by signal 2\n", ptr->jobID, ptr->pid);
                                    }
                                }
                                else {
                                    waitpid(ptr->pid, &status, WUNTRACED);
                                    if (WIFSTOPPED(status)) {
                                        ptr->status = 2;
                                        printf("\n");
                                    }
                                    if (status == 2) { 
                                        printf("\n[%d] %d terminated by signal 2\n", ptr->jobID, ptr->pid);
                                    }
                                }
                            }
                            ptr = ptr->next;
                        }
                    }
                }

            } else if (!strcmp(args[0], "jobs")) {
                // jobs: List current jobs, including their jobID, processID, current status, and
                //       command. If no jobs exist, this should print nothing.
                
                if (jobList->status != 0) {
                    job* ptr = jobList; 
                    while (ptr->status != 0) {
                        if (ptr->status == 1) {
                            printf("[%d] %d %s %s &", ptr->jobID, ptr->pid, getStatus(ptr->status), ptr->command);
                        }
                        else {
                            printf("[%d] %d %s %s", ptr->jobID, ptr->pid, getStatus(ptr->status), ptr->command);
                        }
                        printf("\n");
                        ptr = ptr->next;
                    }
                }

            } else if (!strcmp(args[0], "kill")) {
                if (args[1][0] != '%') {
                    printf("Invalid input");
                }
                else {
                    char* tempStr = malloc(strlen(args[1]) * sizeof(char));
                    for(int i = 1; i < strlen(args[1]); i++) {
                        tempStr[i-1] = args[1][i];
                    }
                    tempStr[strlen(args[1]) - 1] = '\0';
                    strcpy(args[1], tempStr);
                    free(tempStr);
                    job* ptr = jobList;
                    while (ptr->status > 0) { 
                        if (ptr->jobID == atoi(args[1])) {
                            if (ptr->status == 2) {
                                kill(ptr->pid, SIGCONT);
                            }
                            kill(ptr->pid, SIGTERM);
                            printf("[%d] %d terminated by signal 15\n", ptr->jobID, ptr->pid);
                        }
                    ptr = ptr->next;
                    }
                }

            } else {
                //this is where the stuff happens
                //basically im putting working directory and other directories into path to see if it exists
                struct stat buf;
                char* path = malloc((strlen(pwd) + strlen(args[0]) + 11)*sizeof(char));
                strcpy(path, pwd);
                if (args[0][0] != '/') {
                    strcat(path, "/");
                    strcat(path, args[0]);
                } else {
                    strcpy(path, args[0]);
                }
                int exists = -1;
                exists = stat(path, &buf);
                if (exists) {
                    strcpy(path, "/usr/bin/");
                    strcat(path, args[0]);
                    exists = stat(path, &buf);
                    if (exists) {
                        strcpy(path, "/bin/");
                        strcat(path, args[0]);
                        exists = stat(path, &buf);
                    }
                }
                if (!exists) {
                    if (S_ISREG(buf.st_mode)) {
						int bg = background(args, length);
						if (bg) {
							length--;
						}
                        free(args[0]);
                        args[0] = malloc((strlen(path) + 4) * sizeof(char));
                        strcpy(args[0], path);
                        free(path);
                        if (!(tempPid = fork())) {

                            //execv
                            sigprocmask(SIG_UNBLOCK, &parentmask, NULL);
							sigprocmask(SIG_BLOCK, &childmask, NULL);
                            execv(args[0], args);
                            exit(status);
                        }
                        else {
                            //int status;
							job* newJob = malloc(sizeof(job));
                            newJob->status = 1;
                            char* command = malloc((strlen(args[0]) + 4) * sizeof(char));
                            strcpy(command, args[0]);
                            for(int i = 1; i < length; i++) {
                                strcat(command, " ");
                                command = realloc(command, (strlen(command) + strlen(args[i]) + 2) * sizeof(char));
                                strcat(command, args[i]);
                            }
                            newJob->command = command;                     // Need to find a way to concatenate all the strings
                            newJob->next = NULL;
                            newJob->pid = tempPid;
                            if (jobList->status == 0) {
                                newJob->next = jobList;
                                newJob->jobID = 1;
                                jobList = newJob;
                            } else {
                                job* ptr = jobList;
                                while (ptr->next->status != 0){
                                    ptr = ptr->next;
                                }
                                newJob->next = ptr->next;
                                newJob->jobID = (ptr->jobID) + 1;
                                ptr->next = newJob;
                            }
							if (bg) {
                                printf("[%d] %d\n", newJob->jobID, newJob->pid);
                            } else {
                            	waitpid(tempPid, &status, WUNTRACED);
                                if (WIFSTOPPED(status)) {
                                    newJob->status = 2;
                                    printf("\n");
                                }
                                if (status == 2) { 
                                    printf("\n[%d] %d terminated by signal 2\n", newJob->jobID, newJob->pid);
                                }
                            }
                        }
                        // else {

                        //     // HERE IS WHERE THE PARENT PROCESS BEGINS AFTER THE FORK
                        //     // I THINK JOB STUFF GOES HERE
                        //     // PUT JOB STUFF HERE
                        //     // AND ALSO WHATEVER ELSE YOU WANT

                        //     free(path);
                        //     if (!bg) {
                        //         int stat;
                        //         waitpid(tempPid, &stat, 0);
                        //     } else {
                        //         printf("[%d] %d\n", i, newJob->pid);
                        //     }
                        // }
                    }
                    else {
                        free(path);
                        printf("%s: Is a directory\n", args[0]);
                    }
                } else {
                    free(path);
                    int slash = 0;
                    for(int i = 0; i < strlen(args[0]); i++) {
                        if (args[0][i] == '/') {
                            slash = 1;
                        }
                    }
                
                    if (slash) {
                        printf("%s: No such file or directory\n", args[0]);
                    } else {
                        printf("%s: command not found\n", args[0]);
                    }
                }
            }
        }
        for(int i = 0; i < length + 1; i++) {
            free(args[i]);
        }
        free(args);
        
        //
        // handling sigchld now
        // but need to delete the jobs in joblist
        //
    }

    //comment this out if you want to test : VVVVV
    if (jobList->status != 0) {
        job* ptr = jobList; 
        while (ptr->status != 0) {
            kill(ptr->pid, SIGHUP);
            if (ptr->status == 2) {
                kill(ptr->pid, SIGCONT);
            }
            ptr = ptr->next;
        }
    }
    //comment above ^^^^^^
    job* temp = jobList;
    while (jobList->status != 0) {
        temp = temp->next;
        free(jobList->command);
        free(jobList);
        jobList = temp;
    }
    free(jobList);
    return 0;
}