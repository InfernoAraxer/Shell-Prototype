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
	int status; //1 for Running, 2 for Stopped, 3 for Terminated
	char** command;
    int commandLength;
	job* next;
};

// Repeatedly handles Ctrl+Z and Ctrl+C Inputs
    // We will have to update the functions so they will be able to terminate
    // jobs that are in the fore/background.
void sigHandler(int signum) {
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);

    switch (signum) {
        //SIGINT = Case 2
        case 2:
            // Method to send SIGINT to all Forground jobs and its child processes
            break;
        //SIGTSTP = Case 20;
        case 20:
            // Implement method to send SIGTSTP to all forgound jobs and its child
            // processes.
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
        args[length - 1] = NULL;
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
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);
    // int i = 1;
    // pid_t* pid = malloc(sizeof(pid_t));
    job* jobList = malloc(sizeof(job));
    jobList->status = 0;
    char* home = getenv("HOME");
    setenv("PWD", home, 1);
    int length = 0;
    int status = 0;
    int bg = 0;
    char** args;
    while(!feof(stdin)) {
        printf("> ");
        length = 0;
        status = 0;
        bg = 0;
        pid_t tempPid;
        args = NULL;
        char* pwd = getenv("PWD");

        //read input
        if (!feof(stdin)) {
            args = readInput(&length);
        }

        //if empty
        if (length == 0 && args[0] == NULL) {
            free(args);
            continue;
        }

        // Determines if Process will run in the background
        if (!feof(stdin)) {
            bg = background(args, length);
            if (!strcmp(args[0], "bg")) {
                // Input bg <jobID>: Run a suspended job in the background

                free(args[0]);
                free(args);
            } else if (!strcmp(args[0], "cd")) {
                // cd [path]: Changes current directory to the given absolute or relative path. 
                //           If no path is give, use the value of environment variable HOME.
                //           Your shell should update the environment variable PWD with the
                //           (absolute) present working directory after running cd.

                
                if (length == 1) {
                    setenv("PWD", home, 1);
                } else {
                    char* path = malloc((strlen(pwd) + strlen(args[1]) + 11)*sizeof(char));
                    int exists = -1;
                    struct stat buf;
                    strcpy(path, pwd);
                    if (args[0][0] != '/') {
                        strcat(path, "/");
                    }
                    for (int i = 1; i < length; i++) {
                        strcat(path, args[i]);
                    }
                    printf("%s   ", path); // Testing Line

                    exists = stat(path, &buf);

                    if (!exists && S_ISDIR(buf.st_mode)) {
                        setenv("PWD", path, 1);
                    } else if (!exists) {
                        printf("bash: cd: %s: Not a directory\n", args[1]);
                    } else {
                        printf("bash: cd: %s: No such file or directory\n", args[1]);
                    }
                    free(path);
                }

                
                for (int i = 0; i < length; i++) {
                    free(args[i]);
                }                
                free(args);
            } else if (!strcmp(args[0], "exit")) {
                // exit: Exit the shell. The shell should also exit if the user hits ctrl-d on
                //       on an empty input line. When the shell exist, it should first send SIGHUP
                //       followed by SIGCONT to any stopped jobs, and SIGHUP to any running jobs.
                
                return 0;
            } else if (!strcmp(args[0], "fg")) {
                // fg <jobID>L Run a suspended or background job in the foreground

                free(args[0]);
                free(args);
            } else if (!strcmp(args[0], "jobs")) {
                // jobs: List current jobs, including their jobID, processID, current status, and
                //       command. If no jobs exist, this should print nothing.
                
                if (jobList->status != 0) {
                    job* temp = jobList; 
                    int i = 1;
                    while (temp->status != 0) {
                        printf("[%d] %d %s %s", i, temp->pid, getStatus(temp->status), temp->command[0]);
                        for (int i = 1; i < temp->commandLength; i++) {
                            printf("%s ", temp->command[i]);
                        }
                        printf("\n");
                        temp = temp->next;
                        i++;
                    }
                }
                free(args[0]);
                free(args);

            } else if (!strcmp(args[0], "kill")) {
                // kill <jobID>: Send SIGTERM to the given job.
                
                free(args[0]);
                free(args);
            } else {
                //this is where the stuff happens
                //basically im putting working directory and other directories into path to see if it existss
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
                        if (!(tempPid = fork())) {
                            execv(path, args);
                            exit(status);
                        } else {

                            // HERE IS WHERE THE PARENT PROCESS BEGINS AFTER THE FORK
                            // I THINK JOB STUFF GOES HERE
                            // PUT JOB STUFF HERE
                            // AND ALSO WHATEVER ELSE YOU WANT
                            
                            job* newJob = malloc(sizeof(job));
                            newJob->status = 1;
                            newJob->command = args;                     // Need to find a way to concatenate all the strings
                            newJob->next = NULL;
                            newJob->pid = tempPid;
                            newJob->commandLength = length;
                            int i = 1;
                            if (jobList->status == 0) {
                                printf("[%d] %d %s %s\n", i, newJob->pid, getStatus(newJob->status), newJob->command[0]);
                                newJob->next = jobList;
                                jobList = newJob;
                            }
                            else {
                                job* ptr = jobList;
                                i++;
                                while (ptr->next->status != 0){
                                    ptr = ptr->next;
                                    i++;
                                }
                                newJob->next = ptr->next;
                                ptr->next = newJob;
                                printf("[%d] %d %s %s\n", i, newJob->pid, getStatus(newJob->status), newJob->command[0]);
                            }

                            free(path);
                            if (!bg) {
                                int stat;
                                waitpid(tempPid, &stat, 0);
                            }
                        }
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
                
    }
    for(int i = 0; i < length; i++) {
        free(args[i]);
    }
    free(args); 
    job* temp = jobList;
    while (jobList->status != 0) {
        temp = temp->next;
        for (int i = 0; i < jobList->commandLength; i++) {
            free(jobList->command[i]);
        }
        free(jobList->command);
        free(jobList);
        jobList = temp;
    }
    free(jobList);
    return 0;
}