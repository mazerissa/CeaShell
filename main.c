#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h> //nik el windows
#include <direct.h>

int main() {
    char input[100];
    char cwd[MAX_PATH];
    bool running = true;

    printf("CeaShell\n\n");

    do {
        if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
            printf("[%s]\nceashell> ", cwd);
        } else {
            printf("ceashell> ");
        }
        fflush(stdout);

        if (fgets(input, 100, stdin) == NULL) break;
        input[strcspn(input, "\r\n")] = 0;

        char input_copy[100];
        strcpy(input_copy, input);
        char *command = strtok(input_copy, " ");

        if (command == NULL || strlen(command) == 0) continue;
        
        if (strcmp(command, "exit") == 0) {
            running = false;
        } 
        else if (strcmp(command, "help") == 0) {
            printf("Built-ins: exit, cd, clear\n");
        }
        else if (strcmp(command, "cd") == 0) {
            char *path = strtok(NULL, "");
            if (path == NULL) {
                printf("Usage: cd <directory>\n");
            } else {
                if (!SetCurrentDirectoryA(path)) {
                    printf("Directory not found: %s\n", path);
                }
            }
        }
        else if (strcmp(command, "clear") == 0) {
            system("clear"); 
        }
        else {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (CreateProcess(NULL, input, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } else {
                printf("CeaShell: Command '%s' failed car Tu es stupide.\n", command);
            }
        }

    } while (running);

    return 0;
}