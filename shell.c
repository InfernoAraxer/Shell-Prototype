#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

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

void executeChildProcess(char* args, char** argsList) {
    if (strcmp(args, "bg")) {
        // Input bg <jobID>: Run a suspended job in the background

    } else if (strcmp(args, "cd")) {
        // cd [path]: Changes current directory to the given absolute or relative path. 
        //           If no path is give, use the value of environment variable HOME.
        //           Your shell should update the environment variable PWD with the
        //           (absolute) present working directory after running cd.

    } else if (strcmp(args, "exit")) {
        // exit: Exit the shell. The shell should also exit if the user hits ctrl-d on
        //       on an empty input line. When the shell exist, it should first send SIGHUP
        //       followed by SIGCONT to any stopped jobs, and SIGHUP to any running jobs.

    } else if (strcmp(args, "fg")) {
        // fg <jobID>L Run a suspended or background job in the foreground

    } else if (strcmp(args, "jobs")) {
        // jobs: List current jobs, including their jobID, processID, current status, and
        //       command. If no jobs exist, this should print nothing.

    } else if (strcmp(args, "kill")) {
        // kill <jobID>: Send SIGTERM to the given job.

    }
    execv(args, argsList);
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
        if (length > 1) {
            bg = background(args, length);
        }
        if (length > 1 && (tempPid = fork()) == 0) {
            executeChildProcess(args[0], args);
            exit(status);
            pid[i-1] = tempPid;
        }
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