#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <direct.h>
#include <shellapi.h>
#include <conio.h>

#define MAX_INPUT 100
#define MAX_HISTORY 10

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0;
    

void print_prompt() {
    char cwd[MAX_PATH];

    if (GetCurrentDirectoryA(MAX_PATH, cwd)) {

        printf("%s\nceashell> ", cwd);
    }

    fflush(stdout);
}

void add_to_history(const char *input) {
    if (strlen(input) == 0) return;

    if (history_count > 0 &&
        strcmp(history[history_count - 1], input) == 0) {
        return;
    }

    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], input);
    } else {
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[MAX_HISTORY - 1], input);
    }
}

void read_input(char input[]) {
    int pos = 0;
    int history_index = history_count;

    memset(input, 0, MAX_INPUT);

    while (1) {
        int ch = _getch();

        if (ch == 13) {
            input[pos] = '\0';
            printf("\n");
            return;
        }

        else if (ch == 8 && pos > 0) {
            pos--;
            printf("\b \b");
        }

        else if (ch == 0 || ch == 224) {
            int arrow = _getch();

            if (arrow == 72 && history_index > 0) {
                history_index--;

                while (pos > 0) {
                    printf("\b \b");
                    pos--;
                }

                strcpy(input, history[history_index]);
                pos = strlen(input);
                printf("%s", input);
            }

            else if (arrow == 80) {
                while (pos > 0) {
                    printf("\b \b");
                    pos--;
                }

                if (history_index < history_count - 1) {
                    history_index++;
                    strcpy(input, history[history_index]);
                    pos = strlen(input);
                    printf("%s", input);
                } else {
                    history_index = history_count;
                    input[0] = '\0';
                    pos = 0;
                }
            }
        }

        else if (ch >= 32 && ch <= 126 && pos < MAX_INPUT - 1) {
            input[pos++] = (char)ch;
            printf("%c", ch);
        }
    }
}

void command_pwd() {
    char cwd[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
        printf("%s\n", cwd);
    }
}

void command_cd(char *arg) {
    if (!arg) {
        printf("Usage: cd <path>\n");
        return;
    }
    if (!SetCurrentDirectoryA(arg)) {
        printf("Path not found: %s\n", arg);
    }
}

void command_touch(char *arg) {
    if (!arg) { 
        printf("Usage: touch <file>\n");
        return;
    }
    FILE *file = fopen(arg, "a");
    if (file) {
        fclose(file);
    }
}

void command_cat(char *arg) {
    if (!arg) {
        printf("Usage: cat <file>\n");
        return;
    }
    char cmd[128];
    sprintf(cmd, "type %s", arg);
    system(cmd);
}

void command_search(char *arg) {
    if (!arg) {
        printf("Usage: search <query>\n");
        return;
    }
    char url[256] = "https://www.google.com/search?q=";
    strcat(url, arg);
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

void command_help() {
    printf("  help             Display this terminal menu\n");
    printf("  clear / cls      Flush screen buffer memory\n");
    printf("  pwd              Print current working directory path\n");
    printf("  cd <dir>         Change system directory node\n");
    printf("  ls               List directory objects in wide format\n");
    printf("  touch <file>     Instantiate or update target file metadata\n");
    printf("  cat <file>       Output file stream buffer content directly\n");
    printf("  search <query>   Fork query to default local web browser process\n");
    printf("  echo <string>    Print matching text array\n");
    printf("  exit             Terminate shell instance loop\n");
}


bool handle_command(char input[]) {
    char copy[MAX_INPUT];
    strcpy(copy, input);

    char *command = strtok(copy, " ");
    char *arg = strtok(NULL, "");

    if (!command) {
        return true;
    }

    if (strcmp(command, "exit") == 0) {
        return false;
    }
    else if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0) {
        system("cls");
    }
    else if (strcmp(command, "help") == 0) {
        command_help();
    }
    else if (strcmp(command, "pwd") == 0) {
        command_pwd();
    }
    else if (strcmp(command, "cd") == 0) {
        command_cd(arg);
    }
    else if (strcmp(command, "ls") == 0) {
        system("dir /w");
    }
    else if (strcmp(command, "touch") == 0) {
        command_touch(arg);
    }
    else if (strcmp(command, "cat") == 0) {
        command_cat(arg);
    }
    else if (strcmp(command, "search") == 0) {
        command_search(arg);
    }
    else if (strcmp(command, "echo") == 0) {
        printf("%s\n", arg ? arg : "");
    }
    else {
        printf("CeaShell: '%s' is not recognized.\n", command);
    }

    return true;
}

int main() {
    char input[MAX_INPUT];
    bool running = true;

    system("cls");
    printf("Ceashell\n");

    while (running) {
        print_prompt();
        read_input(input);
        add_to_history(input);
        running = handle_command(input);
    }

    return 0;
}