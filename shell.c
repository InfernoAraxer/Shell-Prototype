#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main() {

    while(1) {
        printf(">");
        int length = 0;
        char** args = readInput(&length);
        for(int i = 0; i < length; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        break;
    }    
}