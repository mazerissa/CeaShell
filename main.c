#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv)
{
    char input[100];
    bool running = true;

    do{
    printf("CeaShell> ");
    fflush(stdout);
    fgets(input, sizeof(input), stdin);
    
    char *command = strtok(input, " \n");
    
    if (command == NULL) {
        continue;
    }

    if (strcmp(command, "exit") == 0) {
        running = false;
    }
    else if (strcmp(command, "help") == 0) {
        printf("Available commands: exit, help\n");
    }
    else {
        printf("CeaShell: '%s' is not a recognized command. \n", command);
    }
    }

    while(running);
    printf("Tschuss \n");

    return 0;
}