#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

int main() {
    char input[100];
    bool keep_running = true;

    printf("Type 'help' for commands or 'exit' to quit.\n\n");

    do {
        printf("ceashell> ");
        fflush(stdout);

        if (fgets(input, 100, stdin) == NULL) break;

        input[strcspn(input, "\r\n")] = 0;

        char input_copy[100];
        strcpy(input_copy, input);
        char *command = strtok(input_copy, " ");

        if (command == NULL || strlen(command) == 0) {
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            keep_running = false;
        } 
        else if (strcmp(command, "help") == 0) {
            printf("--- CeaShell Help ---\n");
            printf("exit       : Close the shell\n");
            printf("help       : Show this menu\n");
            printf("system     : Any other command will run as a Windows process\n");
        } 
        else {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcess(
                    NULL,          
                    input,          
                    NULL,           
                    NULL,           
                    FALSE,          
                    0,              
                    NULL,           
                    NULL,            
                    &si,            
                    &pi)            
            ) {
                WaitForSingleObject(pi.hProcess, INFINITE);

                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } 
            else {
                printf("CeaShell: '%s' is not recognized as an internal or external command. \n", command);
            }
        }

    } while (keep_running);

    return 0;
}