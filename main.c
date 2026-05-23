#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_INPUT 100
#define MAX_HISTORY 10

char history[MAX_HISTORY][MAX_INPUT];
int history_count = 0;

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #include <shellapi.h>
    #include <conio.h>
    #define CLEAR_SCREEN "cls"
    #define LIST_DIR "dir /w"
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/stat.h>
    #define CLEAR_SCREEN "clear"
    #define LIST_DIR "ls"
    #define MAX_PATH 4096

    int _getch(void) {
        struct termios oldattr, newattr;
        int ch;
        tcgetattr(STDIN_FILENO, &oldattr);
        newattr = oldattr;
        newattr.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
        return ch;
    }
#endif

void print_prompt() {
    char cwd[MAX_PATH];
#ifdef _WIN32
    if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
#else
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
#endif
        printf("%s\nceashell> ", cwd);
    }
    fflush(stdout);
}

void add_to_history(const char *input) {
    if (strlen(input) == 0) return;

    if (history_count > 0 && strcmp(history[history_count - 1], input) == 0) {
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

        if (ch == 13 || ch == 10) {
            input[pos] = '\0';
            printf("\n");
            return;
        }

        else if ((ch == 8 || ch == 127) && pos > 0) {
            pos--;
            printf("\b \b");
        }

#ifdef _WIN32
        else if (ch == 0 || ch == 224) {
            int arrow = _getch();
#else
        else if (ch == 27) {
            if (_getch() == 91) {
                int arrow = _getch();

                if (arrow == 65) arrow = 72;
                else if (arrow == 66) arrow = 80;
                else continue;
#endif
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
#ifndef _WIN32
            }
#endif
        }
        else if (ch >= 32 && ch <= 126 && pos < MAX_INPUT - 1) {
            input[pos++] = (char)ch;
            printf("%c", ch);
        }
    }
}

void command_pwd() {
    char cwd[MAX_PATH];
#ifdef _WIN32
    if (GetCurrentDirectoryA(MAX_PATH, cwd)) {
#else
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
#endif
        printf("%s\n", cwd);
    }
}

void command_cd(char *arg) {
    if (!arg) {
        printf("Usage: cd <path>\n");
        return;
    }
#ifdef _WIN32
    if (!SetCurrentDirectoryA(arg)) {
#else
    if (chdir(arg) != 0) {
#endif
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
    char cmd[256];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "type \"%s\"", arg);
#else
    snprintf(cmd, sizeof(cmd), "cat \"%s\"", arg);
#endif
    system(cmd);
}

void open_url(const char *url) {
#ifdef _WIN32
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
#elif __APPLE__
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "open \"%s\"", url);
    system(cmd);
#else // Linux / WSL
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" > /dev/null 2>&1", url);
    system(cmd);
#endif
}

void command_search(char *arg) {
    if (!arg) {
        printf("Usage: search <query>\n");
        return;
    }

    char url[512] = "https://www.google.com/search?q=";
    int url_len = strlen(url);

    for (int i = 0; arg[i] != '\0' && url_len < 510; i++) {
        if (arg[i] == ' ') {
            url[url_len++] = '+';
        } else {
            url[url_len++] = arg[i];
        }
    }
    url[url_len] = '\0';

    open_url(url);
}

void command_help() {
    printf("  help             Display this terminal menu\n");
    printf("  snake            Display the built-in snake game\n");
    printf("  clear / cls      Flush screen buffer memory\n");
    printf("  pwd              Print current working directory path\n");
    printf("  cd <dir>         Change system directory node\n");
    printf("  ls               List directory objects\n");
    printf("  touch <file>     Instantiate or update target file metadata\n");
    printf("  cat <file>       Output file stream buffer content directly\n");
    printf("  search <query>   Fork query to default local web browser process\n");
    printf("  snake            Launch the built-in retro arcade engine\n");
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
        system(CLEAR_SCREEN);
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
        system(LIST_DIR);
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
    else if (strcmp(command, "snake") == 0) {
#ifdef _WIN32
        if (GetFileAttributesA("snake.exe") == INVALID_FILE_ATTRIBUTES) {
            printf("CeaShell: 'snake.exe' not found. Automating build from 'snake.c'...\n");
            
            if (GetFileAttributesA("snake.c") == INVALID_FILE_ATTRIBUTES) {
                printf("Error: 'snake.c' source file is missing! Cannot compile.\n");
            } else {
                system("gcc snake.c -o snake.exe");
                printf("Build successful!\n");
                system(".\\snake.exe");
            }
        } else {
            system(".\\snake.exe");
        }
#else
        if (access("snake", F_OK) != 0) {
            printf("CeaShell: 'snake' binary not found. Automating build from 'snake.c'...\n");
            if (access("snake.c", F_OK) != 0) {
                printf("Error: 'snake.c' source file is missing! Cannot compile.\n");
            } else {
                system("gcc snake.c -o snake");
                printf("Build successful!\n");
                system("./snake");
            }
        } else {
            system("./snake");
        }
#endif
    }
    else {
        printf("CeaShell: '%s' is not recognized.\n", command);
        printf("Would you like to request this command on GitHub? (y/n): ");
        fflush(stdout);
        
        int choice = _getch();
        printf("%c\n", choice);

        if (choice == 'y' || choice == 'Y') {
            open_url("https://github.com/mazerissa/CeaShell/issues/new");
        }
    }
    return true;
}

int main() {
    char input[MAX_INPUT];
    bool running = true;

    system(CLEAR_SCREEN);
    printf("Ceashell\n");

    while (running) {
        print_prompt();
        read_input(input);
        add_to_history(input);
        running = handle_command(input);
    }

    return 0;
}