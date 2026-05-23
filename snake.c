#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>

    int _kbhit() {
        struct timeval tv = {0L, 0L};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    }

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

#define WIDTH 40
#define HEIGHT 20
#define MAX_TAIL 100

bool gameOver;

int score;

int headX, headY;
int fruitX, fruitY;

int tailX[MAX_TAIL];
int tailY[MAX_TAIL];

int tailLength;

char foodTypes[] = {'#', '&', '$'};
char currentFruitChar;

enum Directions {
    Stop = 0,
    Left,
    Right,
    Up,
    Down
};

enum Directions dir;
enum Directions current_dir;

void game_controls();
void game_menu();
void setup();
void draw();
void input();
void logic();
void spawnFruit();

void game_controls() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    printf("=================================\n");
    printf("         GAME CONTROLS           \n");
    printf("=================================\n\n");

    printf("  W / UP-ARROW    -> Move Up\n");
    printf("  S / DOWN-ARROW  -> Move Down\n");
    printf("  A / LEFT-ARROW  -> Move Left\n");
    printf("  D / RIGHT-ARROW -> Move Right\n");
    printf("  X / ESC         -> Exit Game\n\n");

    printf("Press any key to go back to main menu...");

    _getch();
}

void game_menu() {
    while (1) {

#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        printf(" _________________________________________________________________\n");
        printf(" |  ________  ________   ________  ___  __    _______            |\n");
        printf(" | |\\   ____\\|\\   ___  \\|\\   __  \\|\\  \\|\\  \\ |\\  ___ \\           |\n");
        printf(" | \\ \\  \\___|\\ \\  \\\\ \\  \\ \\  \\|\\  \\ \\  \\/  /|\\ \\   __/|          |\n");
        printf(" |  \\ \\  \\    \\ \\  \\\\ \\  \\ \\   __  \\ \\   ___  \\ \\  \\_|/__        |\n");
        printf(" |   \\ \\  \\____\\ \\  \\\\ \\  \\ \\  \\ \\  \\ \\  \\\\ \\  \\ \\  \\_|\\ \\       |\n");
        printf(" |    \\ \\_______\\ \\__\\\\ \\__\\ \\__\\ \\__\\ \\__\\\\ \\__\\ \\_______\\      |\n");
        printf(" |     \\|_______|\\|__| \\|__|\\|__|\\|__|\\|__| \\|__|\\|_______|      |\n");
        printf(" |_______________________________________________________________|\n");

        printf("\n             [P] Play Game   [H] Help   [E] Exit\n\n");
        printf(" Choose an option: ");

        fflush(stdout);

        int choice = _getch();
        choice = tolower(choice);

        if (choice == 'p') {
            break;
        }
        else if (choice == 'h') {
            game_controls();
        }
        else if (choice == 'e' || choice == 27) {
            exit(0);
        }
    }
}

void spawnFruit() {
    bool valid;

    do {
        valid = true;

        fruitX = (rand() % (WIDTH - 2)) + 1;
        fruitY = (rand() % (HEIGHT - 2)) + 1;

        if (fruitX == headX && fruitY == headY) {
            valid = false;
        }

        for (int i = 0; i < tailLength; i++) {
            if (tailX[i] == fruitX && tailY[i] == fruitY) {
                valid = false;
                break;
            }
        }

    } while (!valid);

    currentFruitChar = foodTypes[rand() % 3];
}

void setup() {
    srand((unsigned int)time(NULL));

    gameOver = false;

    dir = Stop;
    current_dir = Stop;

    headX = WIDTH / 2;
    headY = HEIGHT / 2;

    score = 0;

    tailLength = 0;

    spawnFruit();
}

void draw() {

#ifdef _WIN32
    COORD cursorPosition;

    cursorPosition.X = 0;
    cursorPosition.Y = 0;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
#else
    printf("\033[H\033[J");
#endif

    char frameBuffer[(HEIGHT * (WIDTH + 1)) + 100];

    int ptr = 0;

    for (int y = 0; y < HEIGHT; y++) {

        for (int x = 0; x < WIDTH; x++) {

            if (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1) {
                frameBuffer[ptr++] = '#';
            }

            else if (x == headX && y == headY) {
                frameBuffer[ptr++] = '@';
            }

            else if (x == fruitX && y == fruitY) {
                frameBuffer[ptr++] = currentFruitChar;
            }

            else {

                bool isTail = false;

                for (int i = 0; i < tailLength; i++) {

                    if (tailX[i] == x && tailY[i] == y) {
                        frameBuffer[ptr++] = 'o';
                        isTail = true;
                        break;
                    }
                }

                if (!isTail) {
                    frameBuffer[ptr++] = ' ';
                }
            }
        }

        frameBuffer[ptr++] = '\n';
    }

    frameBuffer[ptr] = '\0';

    printf("%s", frameBuffer);

    printf("\nScore: %d\n", score);

    fflush(stdout);
}

void input() {

#ifdef _WIN32

    if (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP) & 0x8000) {

        if (current_dir != Down)
            dir = Up;
    }

    else if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN) & 0x8000) {

        if (current_dir != Up)
            dir = Down;
    }

    else if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT) & 0x8000) {

        if (current_dir != Right)
            dir = Left;
    }

    else if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT) & 0x8000) {

        if (current_dir != Left)
            dir = Right;
    }

    else if (GetAsyncKeyState('X') & 0x8000 || GetAsyncKeyState(VK_ESCAPE) & 0x8000) {

        gameOver = true;
    }

#else

    while (_kbhit()) {

        int key = _getch();

        if (key == 27) {

            if (_kbhit()) {

                int bracket = _getch();

                if (bracket == 91 && _kbhit()) {

                    int arrow = _getch();

                    if (arrow == 65 && current_dir != Down)
                        dir = Up;

                    else if (arrow == 66 && current_dir != Up)
                        dir = Down;

                    else if (arrow == 68 && current_dir != Right)
                        dir = Left;

                    else if (arrow == 67 && current_dir != Left)
                        dir = Right;
                }
            }
            else {
                gameOver = true;
            }
        }

        else {

            key = tolower(key);

            if (key == 'w' && current_dir != Down)
                dir = Up;

            else if (key == 's' && current_dir != Up)
                dir = Down;

            else if (key == 'a' && current_dir != Right)
                dir = Left;

            else if (key == 'd' && current_dir != Left)
                dir = Right;

            else if (key == 'x')
                gameOver = true;
        }
    }

#endif
}

void logic() {

    if (dir != Stop)
        current_dir = dir;

    int prevX = tailX[0];
    int prevY = tailY[0];

    int prev2X, prev2Y;

    tailX[0] = headX;
    tailY[0] = headY;

    for (int i = 1; i < tailLength; i++) {

        prev2X = tailX[i];
        prev2Y = tailY[i];

        tailX[i] = prevX;
        tailY[i] = prevY;

        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {

        case Left:
            headX--;
            break;

        case Right:
            headX++;
            break;

        case Up:
            headY--;
            break;

        case Down:
            headY++;
            break;

        default:
            break;
    }

    if (headX <= 0 || headX >= WIDTH - 1 ||
        headY <= 0 || headY >= HEIGHT - 1) {

        gameOver = true;
    }

    for (int i = 1; i < tailLength; i++) {

        if (tailX[i] == headX && tailY[i] == headY) {
            gameOver = true;
        }
    }

    if (headX == fruitX && headY == fruitY) {

        score += 10;

        if (tailLength < MAX_TAIL)
            tailLength++;

        spawnFruit();
    }
}

int main() {

#ifdef _WIN32

    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO info;

    info.dwSize = 100;
    info.bVisible = FALSE;

    SetConsoleCursorInfo(consoleHandle, &info);

#else

    printf("\e[?25l");

#endif

    game_menu();

    setup();

#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    while (!gameOver) {

        draw();

        input();

        logic();

#ifdef _WIN32
        Sleep(60);
#else
        usleep(60000);
#endif
    }

#ifdef _WIN32

    system("cls");

    info.bVisible = TRUE;

    SetConsoleCursorInfo(consoleHandle, &info);

#else

    system("clear");

    printf("\e[?25h");

#endif

    printf("=================================\n");
    printf("  GAME OVER! Final Score: %d\n", score);
    printf("=================================\n\n");

    return 0;
}