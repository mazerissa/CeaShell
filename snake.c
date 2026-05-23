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

    static int _kbhit(void) {
        struct timeval tv = {0L, 0L};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    }

    static int _getch(void) {
        struct termios old, neo;
        tcgetattr(STDIN_FILENO, &old);
        neo = old;
        neo.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &neo);
        int ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        return ch;
    }
#endif

#define WIDTH    40
#define HEIGHT   20
#define MAX_TAIL (WIDTH * HEIGHT)


typedef struct { char ch; int pts; } Food;
static const Food FOODS[] = { {'#', 10}, {'&', 20}, {'$', 30} };
#define NUM_FOODS 3


#define SPEED_START 120000   
#define SPEED_MIN    30000  
#define SPEED_STEP   10000 


static bool gameOver;
static int  score, highScore;
static int  headX, headY;
static int  fruitX, fruitY;
static int  tailX[MAX_TAIL], tailY[MAX_TAIL];
static int  tailLen;
static bool grid[HEIGHT][WIDTH];
static int  foodIdx;
static int  tickDelay;

typedef enum { Stop = 0, Left, Right, Up, Down } Dir;
static Dir dir, curDir;
static void clearScreen(void);
static void showControls(void);
static void showMenu(void);
static void spawnFood(void);
static void setup(void);
static void draw(void);
static void handleInput(void);
static void tick(void);

static void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    fputs("\033[2J\033[H", stdout);
    fflush(stdout);
#endif
}

static void showControls(void) {
    clearScreen();
    puts("=================================");
    puts("         GAME CONTROLS           ");
    puts("=================================\n");
    puts("  W / UP-ARROW    -> Move Up");
    puts("  S / DOWN-ARROW  -> Move Down");
    puts("  A / LEFT-ARROW  -> Move Left");
    puts("  D / RIGHT-ARROW -> Move Right");
    puts("  X / ESC         -> Quit\n");
    puts("  Food & points:");
    puts("    #  =  10 pts  (common)");
    puts("    &  =  20 pts  (rare)");
    puts("    $  =  30 pts  (jackpot)\n");
    puts("  Speed increases each time you eat!\n");
    puts("Press any key to return...");
    _getch();
}

static void showMenu(void) {
    for (;;) {
        clearScreen();
        puts(" _________________________________________________________________");
        puts(" |  ________  ________   ________  ___  __    _______            |");
        puts(" | |\\   ____\\|\\   ___  \\|\\   __  \\|\\  \\|\\  \\ |\\  ___ \\           |");
        puts(" | \\ \\  \\___|\\ \\  \\\\ \\  \\ \\  \\|\\  \\ \\  \\/  /|\\ \\   __/|          |");
        puts(" |  \\ \\  \\    \\ \\  \\\\ \\  \\ \\   __  \\ \\   ___  \\ \\  \\_|/__        |");
        puts(" |   \\ \\  \\____\\ \\  \\\\ \\  \\ \\  \\ \\  \\ \\  \\\\ \\  \\ \\  \\_|\\ \\       |");
        puts(" |    \\ \\_______\\ \\__\\\\ \\__\\ \\__\\ \\__\\ \\__\\\\ \\__\\ \\_______\\      |");
        puts(" |     \\|_______|\\|__| \\|__|\\|__|\\|__|\\|__| \\|__|\\|_______|      |");
        puts(" |_______________________________________________________________|");

        if (highScore > 0) printf("\n             High Score: %d\n", highScore);
        puts("\n             [P] Play   [H] Help   [E] Exit\n");
        fputs(" Choose: ", stdout);
        fflush(stdout);

        int ch = tolower(_getch());
        if      (ch == 'p')            break;
        else if (ch == 'h')            showControls();
        else if (ch == 'e' || ch == 27) exit(0);
    }
}

static void spawnFood(void) {
    int x, y;
    do {
        x = (rand() % (WIDTH  - 2)) + 1;
        y = (rand() % (HEIGHT - 2)) + 1;
    } while ((x == headX && y == headY) || grid[y][x]);
    fruitX  = x;
    fruitY  = y;
    foodIdx = rand() % NUM_FOODS;
}

static void setup(void) {
    srand((unsigned int)time(NULL));
    gameOver  = false;
    dir       = Stop;
    curDir    = Stop;
    headX     = WIDTH  / 2;
    headY     = HEIGHT / 2;
    score     = 0;
    tailLen   = 0;
    tickDelay = SPEED_START;
    memset(grid, 0, sizeof(grid));
    spawnFood();
}

static void draw(void) {
    static char buf[HEIGHT * (WIDTH + 1) + 128];
    int ptr = 0;

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            char c;
            if (y == 0 || y == HEIGHT-1 || x == 0 || x == WIDTH-1)
                c = '#';
            else if (x == headX && y == headY)
                c = '@';
            else if (x == fruitX && y == fruitY)
                c = FOODS[foodIdx].ch;
            else if (grid[y][x])
                c = 'o';
            else
                c = ' ';
            buf[ptr++] = c;
        }
        buf[ptr++] = '\n';
    }
    buf[ptr] = '\0';
#ifdef _WIN32
    COORD origin = {0, 0};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), origin);
#else
    fputs("\033[H", stdout);
#endif

    fputs(buf, stdout);

    int level = (SPEED_START - tickDelay) / SPEED_STEP + 1;
    printf("Score: %-6d  Best: %-6d  Level: %d\n", score, highScore, level);
    fflush(stdout);
}

static void handleInput(void) {
#ifdef _WIN32
    if      (GetAsyncKeyState('W') & 0x8000 || GetAsyncKeyState(VK_UP)     & 0x8000) { if (curDir != Down)  dir = Up;    }
    else if (GetAsyncKeyState('S') & 0x8000 || GetAsyncKeyState(VK_DOWN)   & 0x8000) { if (curDir != Up)    dir = Down;  }
    else if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState(VK_LEFT)   & 0x8000) { if (curDir != Right) dir = Left;  }
    else if (GetAsyncKeyState('D') & 0x8000 || GetAsyncKeyState(VK_RIGHT)  & 0x8000) { if (curDir != Left)  dir = Right; }
    else if (GetAsyncKeyState('X') & 0x8000 || GetAsyncKeyState(VK_ESCAPE) & 0x8000) { gameOver = true; }
#else
    while (_kbhit()) {
        int k = _getch();
        if (k == 27) {
            if (_kbhit()) {
                if (_getch() == 91 && _kbhit()) {
                    int a = _getch();
                    if      (a == 65 && curDir != Down)  dir = Up;
                    else if (a == 66 && curDir != Up)    dir = Down;
                    else if (a == 68 && curDir != Right) dir = Left;
                    else if (a == 67 && curDir != Left)  dir = Right;
                }
            } else {
                gameOver = true;
            }
        } else {
            k = tolower(k);
            if      (k == 'w' && curDir != Down)  dir = Up;
            else if (k == 's' && curDir != Up)    dir = Down;
            else if (k == 'a' && curDir != Right) dir = Left;
            else if (k == 'd' && curDir != Left)  dir = Right;
            else if (k == 'x')                    gameOver = true;
        }
    }
#endif
}

static void tick(void) {
    if (dir != Stop) curDir = dir;
    if (curDir == Stop) return;

    int nx = headX, ny = headY;
    switch (curDir) {
        case Left:  nx--; break;
        case Right: nx++; break;
        case Up:    ny--; break;
        case Down:  ny++; break;
        default:          break;
    }

    bool ate = (nx == fruitX && ny == fruitY);
    if (!ate && tailLen > 0)
        grid[tailY[tailLen - 1]][tailX[tailLen - 1]] = false;
    if (nx <= 0 || nx >= WIDTH-1 || ny <= 0 || ny >= HEIGHT-1) {
        gameOver = true;
        return;
    }
    if (grid[ny][nx]) {
        gameOver = true;
        return;
    }
    memmove(&tailX[1], &tailX[0], (size_t)tailLen * sizeof(int));
    memmove(&tailY[1], &tailY[0], (size_t)tailLen * sizeof(int));
    tailX[0] = headX;
    tailY[0] = headY;

    if (tailLen > 0 || ate)
        grid[headY][headX] = true;

    headX = nx;
    headY = ny;

    if (ate) {
        score += FOODS[foodIdx].pts;
        if (score > highScore) highScore = score;
        if (tailLen < MAX_TAIL - 1) tailLen++;
        if (tickDelay > SPEED_MIN)  tickDelay -= SPEED_STEP;
        spawnFood();
    }
}


int main(void) {
#ifdef _WIN32
    HANDLE con = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO ci = {100, FALSE};
    SetConsoleCursorInfo(con, &ci);
#else
    fputs("\e[?25l", stdout);
    fflush(stdout);
#endif

    highScore = 0;

    for (;;) {
        showMenu();
        setup();
        clearScreen();

        while (!gameOver) {
            draw();
            handleInput();
            tick();
#ifdef _WIN32
            Sleep(tickDelay / 1000);
#else
            usleep(tickDelay);
#endif
        }
        clearScreen();
        puts("=================================");
        printf("  GAME OVER!  Score : %d\n", score);
        printf("  High Score  : %d\n", highScore);
        puts("=================================\n");
        puts("  [R] Play Again    [E] Exit\n");
        fputs("  > ", stdout);
        fflush(stdout);

        for (;;) {
            int ch = tolower(_getch());
            if (ch == 'r') break;
            if (ch == 'e' || ch == 27) goto done;
        }
    }

done:
#ifdef _WIN32
    ci.bVisible = TRUE;
    SetConsoleCursorInfo(con, &ci);
    system("cls");
#else
    fputs("\e[?25h", stdout);
#endif
    return 0;
}