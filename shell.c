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

// Structure available to navigate files in the /usr/bin/ and /bin/ folders
struct dirent* dir;

typedef struct job job;
struct job{
	pid_t pid;
	int status; //1 for Running, 2 for Stopped, 3 for Terminated
	char** command;
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

    Update for all cases with spaces
*/
char** readInput(int* length) { 
    char** args = malloc(sizeof(char *));
    int i = 0;
    char c;
    int status;
    bool foundArgs = false;
    while(!feof(stdin)) {
		*length = i;
        int j = 2;
        args = realloc(args, (i+1) * sizeof(char *));
        args[i] = malloc(j * sizeof(char));
        if ((status = scanf("%c", &c)) == EOF && foundArgs == false) {            
            free(args[i]);
            args[i] = NULL;
            // free(args);
            *length = i;
            return args;
        }

        while (c == ' ' && status != 0) {
            status = scanf("%c", &c);
        }

        while(c != ' ' && c != '\n' && !feof(stdin) && status != 0) {
            foundArgs = true;
            args[i] = realloc(args[i], j * sizeof(char));
            args[i][j-2] = c;            
            args[i][j-1] = '\0';
            j++;
            status = scanf("%c", &c);
        }
        if (status == 1 && foundArgs) {
            *length = i + 1;
            // printf("ok. ");
            return args;
        }
        i++;
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

// Remy's Recursive Function to find file or command
int printDirs(DIR* dirp, char* root, char* key, char** updatedPath){
	// fileFound is to know to use the updated path with the file if a command is found
    bool fileFound = false;
	dir = readdir(dirp);
	while(dir != NULL){
        // This 'if' updated the 'updatedPath' with the location of the exact system method.
		if(strcmp(dir->d_name, key) == 0 && dir->d_name[0] != '.'){
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
			temp[i] = '\0';
            *updatedPath = temp;
		}
		dir = readdir(dirp);
	}

    // Returns whether a file has been found or not
    if (fileFound) {
        return 0;
    } else {
       return 1; 
    }
}

//Attempts to Find the Command or File given by user
int findFileOrCommand(char** path, char** updatedPath) {
        // printf("%s %s\n", *path, *updatedPath);
    // Looks within /usr/bin/ for the command or file
    if (path[0][0] != '.' && path[0][0] != '/') {
        char* filepath = "/usr/bin/";
        DIR* dirp = opendir(filepath);
        if (!printDirs(dirp, filepath, path[0], updatedPath)) {
            closedir(dirp);
            return 0;
        } else {
            closedir(dirp);
        }

        // printf("%s %s\n", *path, *updatedPath);
    // Looks within /bin/ for the command or file
        filepath = "/bin/";
        dirp = opendir(filepath);
        if (!printDirs(dirp, filepath, path[0], updatedPath)) {
            closedir(dirp);
            return 0;
        } else {
            closedir(dirp);
        }
        return 1;
    } else if (path[0][0] == '.') {
        updatedPath[0] = &(path[0][1]);
        return 0;
    } else if (path[0][0] == '/') {
		char* filepath = *path;
        DIR* dirp = opendir(filepath);
        if (printDirs(dirp, filepath, path[0], updatedPath) == 0) {
            // printf("%s \n", updatedPath[0]);
			closedir(dirp);
            return 0;
        } else {
            closedir(dirp);
			return 1;
        }
        // return 0;
    }
    // Shouldn't reach here, but extra case
    return 1;
}

// Lists the various processes that we have to implement
void executeChildProcess(char* args, char** argsList, int** pipefd) {
    // printf("%d", strcmp(args, "jobs"));
    close(pipefd[0][0]);              /* Close unused read end */

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
        return;
    } else if (strcmp(args, "fg") == 0) {
        // fg <jobID>L Run a suspended or background job in the foreground

    } else if (!strcmp(args, "jobs")) {
        // jobs: List current jobs, including their jobID, processID, current status, and
        //       command. If no jobs exist, this should print nothing.

		fflush(stdout);

        // printf("I am the child.\n");
        // printf("The child is about to read from the pipe.\n");
        // while (read(pipefd[0], &buf, 1) > 0){
        //     
        // }
        // write(STDOUT_FILENO, "\n\n", 2);

    } else if (strcmp(args, "kill") == 0) {
        // kill <jobID>: Send SIGTERM to the given job.
 
    } else {
        // Prints the various error outputs if file or command isn't found 
        char* foundPath = NULL;
        if (findFileOrCommand(&args, &foundPath) != 0 && args[0] != '\0') {
            if (args[0] == '.' || args[0] == '/') {
                printf("%s: No such file or directory\n", args);
            } else {
                printf("%s: command not found\n", args);
            }
        } else if (args[0] != '\0') {
            write(pipefd[0][1], "0", 2);
            execv(foundPath, argsList);
        }
    }
    write(pipefd[0][1], "1", 2);
    close(pipefd[0][1]);
}

// Main
int main() {
    signal(SIGINT, sigHandler);
    signal(SIGTSTP, sigHandler);
    int i = 1;
    // pid_t* pid = malloc(sizeof(pid_t));
    job* jobList = malloc(sizeof(job));
    jobList->status = 0;
    int* pipefd = malloc(2 * sizeof(int));                          // Consider Errors later
    while(!feof(stdin)) {
        printf("> ");
        int length = 0;
        int status = 0;
        int bg = 0;
        pid_t tempPid;
        char** args;
        if (!feof(stdin)) {
            args = readInput(&length);
        }
        // Determines if Process will run in the background
        if (length > 0 && args[0] != NULL) {
            bg = background(args, length);
        }

        // Forks to run the program and print program information, Also consider if people use commands incorrectly
        if (pipe(pipefd) == -1) {
            fprintf(stderr, "%s", "The call to pipe() has failed.\n");           
            exit(EXIT_FAILURE);
        }
        // printf("lmao ");
        // printf("%s  ", args[0]);
        if (!feof(stdin) && length > 0 && (tempPid = fork()) == 0) {
            executeChildProcess(args[0], args, &pipefd);
            exit(status);
        } else {
            close(pipefd[1]);              /* Closing write. */
            char* createdProcess = malloc(2);
            read(pipefd[0], createdProcess, 2);
            if (!strcmp(createdProcess, "0")) {
                job* newJob = malloc(sizeof(job));
                newJob->status = 1;
                newJob->command = args;                     // Need to find a way to concatenate all the strings
                newJob->next = NULL;
                newJob->pid = tempPid;
                i = 1;
                if (jobList->status == 0) {
                    printf("[%d] %d %s %s\n", i, newJob->pid, getStatus(newJob->status), newJob->command[0]);
                    newJob->next = jobList;
                    jobList = newJob;
                }
                else {
                    job* ptr = jobList;
                    i++;
                    while(ptr->next->status != 0){
                        ptr = ptr->next;
                        i++;
                    }
                    newJob->next = ptr->next;
                    ptr->next = newJob;
                    printf("[%d] %d %s %s\n", i, newJob->pid, getStatus(newJob->status), newJob->command[0]);
                }
            }
            free(createdProcess);
            close(pipefd[0]);              /* Close unused read end */
        }

        // Displays BG process Created
        if (!bg) {
            waitpid(tempPid, &status, 0);
        }
        else {
            printf("[%d] %d\n", i, tempPid);
        }
            
        i++;
        // pid = realloc(pid, i * sizeof(pid_t));
        if (feof(stdin)) {
            executeChildProcess("exit", args, &pipefd);
        }

        for (int i = 0; i < length; i++) {
            free(args[i]);
        }
        free(args);
    }    
    free(pipefd);
    job* temp = jobList;
    while (jobList->status != 0) {
        temp = temp->next;
        free(jobList);
        jobList = temp;
    }
    free(jobList);
    return 0;
}