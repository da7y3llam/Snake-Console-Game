//compile using MSVC: cl /nologo /W4 /WX /DNODEBUG /EHsc /MD /GA /O2 main.c resources\resource.res /Fe:snake.exe && del /q /f *.obj && start "" cmd /c snake
//compile using gcc: gcc main.c -lwinmm -o snake.exe -s -O2 && start "" cmd /c snake
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")

#define BUFF_SIZE 1024

#define KEY_LEFT 75
#define KEY_UP 72
#define KEY_RIGHT 77
#define KEY_DOWN 80
#define KEY_BACKSPACE 32
#define KEY_ESC 27

wchar_t exesrc[BUFF_SIZE];

void CenterWindow();
void HideShowCursor(BOOL state);
void Clear();

void MainMenu();
void StartGame();
void Init();
void Input();
void Draw();
void Logic();
void SetGameMode();
int GetGameMode();
void ResetGame();
void ShowMaxScore();
void HowToPlay();
int GetMaxScore();
void GameOver();

int width, height;
int x, y, food_x, food_y, tail_x[300], tail_y[300], tail_length, current_tail;
char wall[] = {0xE2, 0x96, 0xA0}, tail[] = {0xE2, 0x97, 0x8F}, food[] = {0xE2, 0x99, 0xA5};

int score, max_score, game_difficulty;
bool game_over, isFirstStart;

wchar_t score_file[BUFF_SIZE];
wchar_t settings_file[BUFF_SIZE];

clock_t tic, toc;

enum Direction {STOP = 0, LEFT, UP, RIGHT, DOWN};
enum Direction direction;

HANDLE hStdOut;

int main() {
	system("mode 57,25");
	CenterWindow();
	
	SetConsoleOutputCP(CP_UTF8);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	if (hStdOut == INVALID_HANDLE_VALUE)
		return GetLastError();
	
	DWORD dwMode = 0;
	if (!GetConsoleMode(hStdOut, &dwMode))
		return GetLastError();
	
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(hStdOut, dwMode))
		return GetLastError();
	
	GetModuleFileNameW(NULL, exesrc, BUFF_SIZE);
	swprintf_s(score_file, BUFF_SIZE, L"%ls\\..\\resources\\max_score.data", exesrc);
	swprintf_s(settings_file, BUFF_SIZE, L"%ls\\..\\resources\\settings.data", exesrc);
		
	MainMenu();
	
    return 0;
}
void MainMenu() {
	Clear();
	HideShowCursor(TRUE);
	SetConsoleTitle("Snake - Console Game");
	puts("\x1b[96m* This game is developed by 'Dahi Allam'.");
	puts("* We hope you enjoy playing it as much as we enjoyed\ncreating it, have a nice day.");
	
	printf("\x1b[92m\n\n\n\n\n\t\t ");
	puts("ooooooooooooooooooooo");
	puts("\t\to  1. START GAME      o");
	puts("\t\to  2. GAME DIFFICULTY o");
	puts("\t\to  3. SHOW MAX SCORE  o");
	puts("\t\to  4. RESET GAME      o");
	puts("\t\to  5. HOW TO PLAY     o");
	puts("\t\to  ESC. EXIT          o");
	puts("\t\t ooooooooooooooooooooo");
	
	int ch = _getch();
	if (ch == 224)
		ch = _getch();
	
	switch (ch) {
		case '1':
			StartGame();
			break;
		case '2':
			SetGameMode();
			break;
		case '3':
			ShowMaxScore();
			break;
		case '4':
			ResetGame();
			break;
		case '5':
			HowToPlay();
			break;
		case KEY_ESC:
			return;
		break;
		default:
			MainMenu();
			break;
	}
}

void Input() {
	if (_kbhit()) {
		int ch = _getch();
		if (ch == 224)
			ch = _getch();
		
		switch (ch) {
			case 'a':
			case 'A':
			case KEY_LEFT:
				direction = LEFT;
				break;
			case 'w':
			case 'W':
			case KEY_UP:
				direction = UP;
				break;
			case 'd':
			case 'D':
			case KEY_RIGHT:
				direction = RIGHT;
				break;
			case 's':
			case 'S':
			case KEY_DOWN:
				direction = DOWN;
				break;
			case KEY_BACKSPACE:
				direction = STOP;
				break;
			case KEY_ESC:
				game_over = true;
				MainMenu();
				break;
		}
	}
}

void Init() {
	width = 40, height = 20;
	x = width / 2;
	y = height / 2;
	food_x = rand() % width;
	food_y = rand() % height;
	if (game_difficulty == 2)
		if (food_y == 5 || food_y == 15)
			food_y += 1;
	tail_length = 0;
	current_tail = 0;
	direction = STOP;
	score = 0;
	max_score = GetMaxScore();
	game_over = false;
	game_difficulty = GetGameMode();
	isFirstStart = true;
}

void Draw() {
	
	if (!isFirstStart && direction == STOP)
		return;
	if (isFirstStart)
		isFirstStart = false;
	
	Clear();
	printf("\t\x1b[36mSCORE: %05d\t\t  MAX SCORE: %05d\x1b[0m\n", score, max_score);
	
	printf("\t");
	
	for (int i = 0; i < width + 2; i ++)
		printf("\x1b[32m%s\x1b[0m", wall);
	printf("\n");
	
	for (int i = 0; i < height; i ++) {
		for (int j = 0; j < width; j ++) {
			if (j == 0)
				printf("\x1b[32m\t%s\x1b[0m", wall);
			
			if (game_difficulty == 2 && (i == 5 || i == 15) && j >= 10 && j <= 30)
				printf("\x1b[32m%s\x1b[0m", wall);
			else if (x == j && y == i)
				printf("\x1b[93mO\x1b[0m");
			else if (food_x == j && food_y == i)
				printf("\x1b[31m%s\x1b[0m", food);
			else {
				bool print_tail = false;
				for (int k = 0; k < tail_length; k ++)
					if (tail_x[k] == j && tail_y[k] == i) {
						printf("\x1b[93m%s\x1b[0m", tail);
						print_tail = true;
					}
				if (print_tail == false)
					printf(" ");
			}
			
			if (j == width - 1)
				printf("\x1b[32m%s\x1b[0m", wall);
		}
		printf("\n");
	}
	
	printf("\t");
	for (int i = 0; i < width + 2; i ++)
		printf("\x1b[32m%s\x1b[0m", wall);
	printf("\n\t\x1b[33mSNAKE LENGTH: %-5d  GAME DIFFICULTY: %s\x1b[0m", tail_length + 1, game_difficulty == 1 ? "EASY" : "HARD");
}

void Logic() {
	if (direction == STOP)
		return;
	
	if (tail_length > 0) {
		if (current_tail >= tail_length)
			current_tail = 0;
		
		tail_x[current_tail] = x;
		tail_y[current_tail] = y;
		current_tail ++;
	}
	
	switch (direction) {
		case LEFT:
			x --;
			break;
		case UP:
			y --;
			break;
		case RIGHT:
			x ++;
			break;
		case DOWN:
			y ++;
			break;
	}
	
	// Check if the snake ate its tail.
	for (int i = 0; i < tail_length; i ++)
		if (tail_x[i] == x && tail_y[i] == y)
			GameOver();
	
	 // Check if the snake's head has hit the wall.
	if (game_difficulty == 2)
		if (x >= width || x < 0 || y >= height || y < 0 || (y == 5 || y == 15) && x >= 10 && x <= 30)
			GameOver();
	
	if (x >= width)
		x = 0;
	else if (x < 0)
		x = width - 1;
	if (y >= height)
		y = 0;
	else if (y < 0)
		y = height - 1;
	

	if (food_x == x && food_y == y) {
		tail_length ++;
		score = score + 7;
		food_x = rand() % width;
		food_y = rand() % height;
		if (game_difficulty == 2)
			if (food_y == 5 || food_y == 15)
				food_y += 1;
		
		tail_x[tail_length - 1] = -1;
		tail_y[tail_length - 1] = -1;
		PlaySoundA("resources\\eat.wav", NULL, SND_ASYNC | SND_FILENAME);
		
		if (score > max_score) {
			FILE *fp;
			_wfopen_s(&fp, score_file, L"w");
			if (fp == NULL) {
				fputs("Failed to save the max score.\n", stderr);
				return;
			}
			fprintf(fp, "%d", score);	
			fclose(fp);
			max_score = score;
		}
	}
}

void GameOver() {
	game_over = true;
	Sleep(250);
	
	toc = clock();
	double elapsed_time = (double)(toc - tic) / CLOCKS_PER_SEC;
	int hours = (int)elapsed_time / 60 / 60;
	int minuts = (int)elapsed_time % (60 * 60) / 60;
	int sec = (int)elapsed_time % (60 * 60) % 60;
	Clear();
	puts("\x1b[31m          ======================================");
	puts("         =       GAME OVER                      =");
	printf("         =       YOUR SCORE: %-10d         =\n", score);
	printf("         =       ELIPSED TIME: %02d:%02d:%02d         =\n", hours, minuts, sec);
	puts("          =====================================");
	PlaySoundA("resources\\die.wav", 0, SND_SYNC | SND_FILENAME);
	puts("\nPress any key to return to the main menu.");
	system("PAUSE > NUL");
	
	MainMenu();
}

void StartGame() {
	HideShowCursor(FALSE);
	Init();
	
	tic = clock();
	
	while (!game_over) {
		Draw();
		Input();
		Logic();
		Sleep(10);
	}
}

void SetGameMode() {
	Clear();
	puts("   ================ GAME MODE PAGE ===================");
	puts("  =                   1. EASY                         =");
	puts("  =                   2. HARD                         =");
	puts("   ===================================================");
	puts("0. Main Menu");
	
	int ch = _getch();
	switch (ch) {
		case '1':
			game_difficulty = 0;
			break;
		case '2':
			game_difficulty = 1;
			break;
		case '0':
			MainMenu();
			break;
		default:
			SetGameMode();
	}
	
	FILE *fp;
	_wfopen_s(&fp, settings_file, L"w");
	if (fp == NULL) {
		fputs("Failed to save the settings.\n", stderr);
		return;
	}
	if (ch == '1' || ch == '2') {
		int n = fprintf(fp, "%c", ch);
		fclose(fp);
		Clear();
		
		printf("\n\n\n\n\n\n\n\n");
		if (n > 0 && ch == '1')
			puts(" The game difficulty changed to EASY mode successfully.\n");
		else if (n > 0 && ch == '2')
			puts(" The game difficulty changed to HARD mode successfully.\n");
		else
			fputs("    Error: failed to save the game difficulty.\n", stderr);
		printf("Press any key to return to the main menu.");
		system("PAUSE > NUL 2>&1");
		MainMenu();
	}
	
}

int GetGameMode() {
	int _game_difficulty = 1;
	
	FILE *fp;
	_wfopen_s(&fp, settings_file, L"r");
	if (fp == NULL) {
		fputs("Failed to open the settings file.\n", stderr);
		return _game_difficulty;
	}
	fscanf_s(fp, "%d", &_game_difficulty);
	fclose(fp);
	
	return _game_difficulty;
}

void ShowMaxScore() {
	Clear();
	int _max_score = GetMaxScore();
	puts("   ================ MAX SCORE PAGE ==================");
	puts("  =                                                  =");
	printf("  =               YOUR MAX SCORE = %-10d        =\n", _max_score);
	puts("  =                                                  =");
	puts("   ==================================================");
	
	printf("0. Main Menu\n");
	int ch = _getch();
	switch (ch) {
		case '0':
			MainMenu();
			break;
		default:
			ShowMaxScore();
	}
}

int GetMaxScore() {
	FILE *fp;
	_wfopen_s(&fp, score_file, L"r");
	if (fp == NULL)
		return 0;
	
	int _score;
	fscanf_s(fp, "%d", &_score);
	fclose(fp);
	
	return _score;
}

void ResetGame() {
	Clear();
	
	printf("\n\n\n\n\n\n\n\n\n");
	if (_wremove(L"./resources/settings.data") != 0)
		fputs("\x1b[31m\tERROR: failed to reset the game difficulty.\n", stderr);
	else
		puts("\x1b[32m    The game difficulty has been reset successfully.");
	
	if (_wremove(L"./resources/max_score.data") != 0)
		fputs("\t\x1b[31mERROR: failed to reset the game score.\n", stderr);
	else
		puts("\t\x1b[32mThe game score has been reset successfully.");
	
	printf("Press any key to return to the main menu.");
	system("PAUSE > NUL 2>&1");
	MainMenu();
}

void HowToPlay() {
	Clear();
	puts("   ================ HOW TO PLAY PAGE =================");
	puts("  = Use `A` or `Left-Arrow` to navigate to the left.  =");
	puts("  = Use `W` or `UP-Arrow` to navigate to the Top.     =");
	puts("  = Use `D` or `Right-Arrow` to navigate to the right.=");
	puts("  = Use `S` or `Down-Arrow` to navigate to the down.  =");
	puts("  = Use `Space` button to pause the game.             =");
	puts("  = Use `ESC` button to return to main menu.          =");
	puts("   ===================================================");
	
	printf("0. Main Menu\n");
	int ch = _getch();
	switch (ch) {
		case '0':
			MainMenu();
			break;
		default:
			HowToPlay();
	}
}

void CenterWindow() {
	HWND hwnd = GetConsoleWindow();
		
	RECT rc = {0};
	
    GetWindowRect(hwnd, &rc);
    int win_w = rc.right - rc.left;
    int win_h = rc.bottom - rc.top;
	
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    
    SetWindowPos(hwnd, HWND_TOP, (screen_w - win_w)/2, 
        (screen_h - win_h)/2, 0, 0, SWP_NOSIZE);
}

void Clear() {
	system("cls");
}

void HideShowCursor(BOOL state) {
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(hStdOut, &curInfo);
	curInfo.bVisible = state;
	SetConsoleCursorInfo(hStdOut, &curInfo);
}
