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
void sighandler(int signum) {
    switch (signum) {
        case 2:
        exit();
        
    }
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
        args[i] = malloc(j * sizeof(char));
        scanf("%c", &c);
        if (c == '\n') {
            args[i] = NULL;
            *length = i;
            return args;
        }
        while (c == ' ') {
            scanf("%c", &c);
        }
        while(c != ' ' && c != '\n') {
            args[i][j-2] = c;
            j++;
            args[i] = realloc(args[i], j * sizeof(char));
            scanf("%c", &c);
        }
        i++;
        args = realloc(args, (i+1) * sizeof(char *));
        if (c == '\n') {
            args[i] = NULL;
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

int main() {
    signal(SIGINT, sighandler);
    int i = 1;
    pid_t* pid = malloc(sizeof(pid_t));
    while(1) {
        printf("> ");
        int length = 0;
        int status;
        pid_t tempPid;
        char** args = readInput(&length);
        int bg = background(args, length);
        if ((tempPid = fork()) == 0) {
            execv(args[0], args);
            exit(status);
        }
        pid[i-1] = tempPid;
        if (!bg) {
            waitpid(tempPid, &status, 0);
        }
        else {
            printf("[%d] %d\n", i, tempPid);
        }
        i++;
        pid = realloc(pid, i * sizeof(pid_t));
    }    
}