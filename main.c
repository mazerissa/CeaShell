#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h> 
#include <direct.h>
#include <shellapi.h>
#include <conio.h>

#define MAX_HISTORY 10

int main() {
    char input[100];
    char cwd[MAX_PATH];
    bool running = true;

    char history[MAX_HISTORY][100];
    int history_count = 0;
    int history_index = -1;

    printf("CeaShell\n\n");

    do {
        if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
            printf("[%s]\nceashell> ", cwd);
        } else {
            printf("ceashell> ");
        }
        fflush(stdout);

        int pos = 0;
        int current_view_idx = history_count; 
        memset(input, 0, sizeof(input));

        while (1) {
            int ch = _getch();

            if (ch == 13) {
                input[pos] = '\0';
                printf("\n");
                break;
            } 
            else if (ch == 8) {
                if (pos > 0) {
                    pos--;
                    printf("\b \b");
                }
            } 
            else if (ch == 0 || ch == 224) {
                int arrow = _getch();
                if (arrow == 72) {
                    if (history_count > 0 && current_view_idx > 0) {
                        current_view_idx--;
                        for (int i = 0; i < pos; i++) printf("\b \b");
                        strcpy(input, history[current_view_idx]);
                        pos = strlen(input);
                        printf("%s", input);
                    }
                } 
                else if (arrow == 80) {
                    if (current_view_idx < history_count - 1) {
                        current_view_idx++;
                        for (int i = 0; i < pos; i++) printf("\b \b");
                        strcpy(input, history[current_view_idx]);
                        pos = strlen(input);
                        printf("%s", input);
                    } else {
                        current_view_idx = history_count;
                        for (int i = 0; i < pos; i++) printf("\b \b");
                        pos = 0;
                        input[0] = '\0';
                    }
                }
            } 
            else if (pos < 99 && ch >= 32 && ch <= 126) {
                input[pos++] = (char)ch;
                printf("%c", ch);
            }
        }

        if (strlen(input) == 0) continue;

        if (history_count == 0 || strcmp(history[history_count - 1], input) != 0) {
            if (history_count < MAX_HISTORY) {
                strcpy(history[history_count++], input);
            } else {
                for (int i = 0; i < MAX_HISTORY - 1; i++) strcpy(history[i], history[i + 1]);
                strcpy(history[MAX_HISTORY - 1], input);
            }
        }
        char input_copy[100];
        strcpy(input_copy, input);
        char *command = strtok(input_copy, " ");

        if (strcmp(command, "exit") == 0) {
            running = false;
        } 
        else if (strcmp(command, "help") == 0) {
            printf("Built-ins: exit, cd, clear, search\nArrows: Up/Down for history\n");
        }
        else if (strcmp(command, "cd") == 0) {
            char *path = strtok(NULL, "");
            if (path && !SetCurrentDirectoryA(path)) printf("Path not found.\n");
        }
        else if (strcmp(command, "clear") == 0) {
            system("cls");
        }
        else if (strcmp(command, "search") == 0) {
            char *query = strtok(NULL, "");
            if (query) {
                char url[256] = "https://www.google.com/search?q=";
                strcat(url, query);
                ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
            }
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
                printf("CeaShell: Command '%s' failed, car T'es stupide.\n", command);
            }
        }

    } while (running);

    return 0;
}