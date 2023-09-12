//compile from the terminal: gcc -Wall -Werror main.c -static -O2 -s -o snake && clear && ./snake
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/time.h>

#include <termios.h>
#include <unistd.h>

#define LEFT_ARROW 72
#define UP_ARROW 75
#define RIGHT_ARROW 77
#define DOWN_ARROW 80

#define KEY_ESC 27
#define KEY_SPACE 32


void MainMenu();
void StartGame();
void SetGameMode();
int GetGameMode();
void ResetGame();
int GetMaxScore();
void ShowMaxScore();
void HowToPlay();

void Input();
void Init();
void Draw();
void Logic();
void GameOver();
void EatSound();
void DieSound();

void BlockSTDIN();
void UnblockSTDIN();
void Clear();
void ShowHideCursor(bool);

int width, height;
int x, y, food_x, food_y, tail_x[300], tail_y[300], tail_length, current_tail;
int game_difficulty = 2, score, max_score;
bool game_over;

enum Direction {STOP = 0, LEFT, UP, RIGHT, DOWN};
enum Direction direction;


struct timeval start, end;


int main(void) {
	printf("\e[8;26;60t");
	
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	
	
	MainMenu();
	
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	
	
	return 0;
}

void MainMenu() {
	Clear();
	ShowHideCursor(false);
	//SetConsoleTitle("Snake - Console Game");
	puts("\e[95m* This game is developed by 'Dahi Allam'.");
	puts("* We hope you enjoy playing it as much as we enjoyed\ncreating it, have a nice day.");
	
	printf("\n\n\n\n\n\t\t ");
	puts("\e[92m  ooooooooooooooooooooo");
	puts("\t\t  o  1. START GAME      o");
	puts("\t\t  o  2. GAME DIFFICULTY o");
	puts("\t\t  o  3. SHOW MAX SCORE  o");
	puts("\t\t  o  4. RESET GAME      o");
	puts("\t\t  o  5. HOW TO PLAY     o");
	puts("\t\t  o  X. EXIT            o");
	puts("\t\t   ooooooooooooooooooooo");
	
	
	BlockSTDIN();
	
	int ch = getchar();
	
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
		case 'x':
		case 'X':
			ShowHideCursor(true);
			exit(0);
		break;
		default:
			MainMenu();
			break;
	}
}

void StartGame() {
	ShowHideCursor(false);
	
	UnblockSTDIN();
	
	Init();
	gettimeofday(&start, NULL);
	while (!game_over) {
		Input();
		Draw();
		Logic();
		usleep(1000 * 100);
	}
}

void SetGameMode() {
	Clear();
	puts("     ================ Game Difficulty =================");
	puts("    =                   1. EASY                        =");
	puts("    =                   2. HARD                        =");
	puts("     ===================================================");
	puts("0. Main Menu");
	
	int ch = getchar();
	switch (ch) {
		case '1':
			game_difficulty = 1;
			break;
		case '2':
			game_difficulty = 2;
			break;
		case '0':
			MainMenu();
			break;
		default:
			SetGameMode();
	}
	
	FILE *fp = fopen("./resources/settings.data", "w");
	if (fp == NULL) {
		fputs("Failed to save the settings.\n", stderr);
		return;
	}
	
	if (ch == '1' || ch == '2') {
		int n = fprintf(fp, "%c", ch);
		Clear();
		
		printf("\n\n\n\n\n\n\n\n\n\n\t  ");
		if (n > 0 && ch == '1')
			puts("Game mode saved in EASY mode successfully.");
		else if (n > 0 && ch == '2')
			puts("Game mode saved in HARD mode successfully.");
		else
			fputs("\e[93mError: failed to save the game mode.\e[0m\n", stderr);
		
		fclose(fp);
		
		puts("\nPress `Enter` key to return to the main menu.");
		BlockSTDIN();
		system("/bin/bash -c read -n1");
		MainMenu();
	}
}

int GetGameMode() {
	FILE *fp = fopen("./resources/settings.data", "r");
	if (fp == NULL)
		return 1;
	
	int result;
	fscanf(fp, "%d", &result);
	
	fclose(fp);
	
	return result;
}

void ResetGame() {
	Clear();
	
	printf("\n\n\n\n\n\n\n\n\n");
	if (remove("./resources/settings.data") != 0)
		fputs("\t\e[91mERROR: failed to reset the game difficulty.\n", stderr);
	else
		puts("\t\e[92mThe Game difficulty has been reset successfully.");
	
	if (remove("./resources/maxScore.data") != 0)
		fputs("\t\e[91mERROR: failed to reset the game score.\n", stderr);
	else
		puts("\t\e[92mThe Game score has been reset successfully.");
	
	puts("\nPress `Enter` key to return to the main menu.");
	BlockSTDIN();
	system("/bin/bash -c read -n1");
	MainMenu();
}

int GetMaxScore() {
	FILE *fp = fopen("./resources/maxScore.data", "r");
	if (fp == NULL)
		return 0;
	
	int _max_score;
	fscanf(fp, "%d", &_max_score);
	fclose(fp);
	
	return _max_score;
}

void ShowMaxScore() {
	Clear();
	puts("     ==================== MAX SCORE ===================");
	puts("    =                                                  =");
	printf("    =               YOUR MAX SCORE = %-10d        =\n", GetMaxScore());
	puts("    =                                                  =");
	puts("     ==================================================");
	
	printf("0. Main Menu\n");
	int ch = getchar();
	switch (ch) {
		case '0':
			MainMenu();
			break;
		default:
			ShowMaxScore();
	}
}

void HowToPlay() {
	Clear();
	puts("    =================== HOW TO PLAY ====================");
	puts("   = Use `A` or `🡄` to navigate to the left.            =");
	puts("   = Use `W` or `🡅` to navigate to the Top.             =");
	puts("   = Use `D` or `🡆` to navigate to the right.           =");
	puts("   = Use `S` or `🡇` to navigate to the down.            =");
	puts("   = Use `Space` button to pause the game.              =");
	puts("   = Use `0` button to return to the Main Menu.         =");
	puts("    ====================================================");
	
	printf("0. Main Menu\n");
	int ch = getchar();
	switch (ch) {
		case '0':
			MainMenu();
			break;
		default:
			HowToPlay();
	}
}

void Input() {	
		
	char ch;
    int n = read(STDIN_FILENO, &ch, 1);
    
    if (n == 1) {
		if (ch == 27)
			if (getchar() == 91)
				switch (getchar()) {
					case 'D':
						ch = 72;
						break;
					case 'A':
						ch = 75;
						break;
					case 'C':
						ch = 77;
						break;
					case 'B':
						ch = 80;
						break;
				}
		
		switch (ch) {
			case 'a':
			case 'A':
			case LEFT_ARROW:
				direction = LEFT;
				break;
			case 'w':
			case 'W':
			case UP_ARROW:
				direction = UP;
				break;
			case 'd':
			case 'D':
			case RIGHT_ARROW:
				direction = RIGHT;
				break;
			case 's':
			case 'S':
			case DOWN_ARROW:
				direction = DOWN;
				break;
			case KEY_SPACE:
				direction = STOP;
				break;
			case 'X':
			case 'x':
				game_over = true;
				MainMenu();
				break;
		}
	}
}

void Init() {
	width = 40;
	height = 20;
	x = width / 2;
	y = height / 2;
	direction = STOP;
	tail_length = 0;
	current_tail = 0;
	food_x = rand() % width;
	food_y = rand() % height;
	game_difficulty = GetGameMode();
	score = 0;
	max_score = GetMaxScore();
	game_over = false;
	if (food_y == 5 || food_y == 15)
		food_y ++;
}

void Draw() {
	Clear();

	printf("\e[96m\tSCORE: %06d\t\t MAX SCORE: %06d\e[0m\n", score, max_score);
	
	printf("\t\e[32m⏺\e[0m ");
	for (int i = 0; i < width - 2; i ++)
		printf("\e[32m⏹\e[0m");
	printf(" \e[32m⏺\e[0m\n");

	for (int i = 0; i < height; i ++) {
		for (int j = 0; j < width; j ++) {
			if (j == 0)
				printf("\t\e[32m⏹\e[0m");
			
			if (game_difficulty == 2 && (i == 5 || i == 15) && j >= 7 && j <= 30)
				printf("\e[32m⏹\e[0m");
			else if (x == j && y == i)
				printf("\e[93mO\e[0m");
			else if (food_x == j && food_y == i)
				printf("\e[91m♥\e[0m");
			else {
				bool print = false;
				
				for (int k = 0; k < tail_length; k ++)
					if (tail_x[k] == j && tail_y[k] == i) {
						printf("\e[38;5;214m⏺\e[0m");
						print = true;
					}
				if (!print)
					printf(" ");
			}

			if (j == width - 1)
				printf("\e[32m⏹\e[0m");
		}
		printf("\n");
	}
	
	printf("\t\e[32m⏺\e[0m ");
	for (int i = 0; i < width - 2; i ++)
		printf("\e[32m⏹\e[0m");
	printf(" \e[32m⏺\e[0m\n\t\e[93mSNAKE LENGTH: %-5d  GAME DIFFICULTY: %s\n", tail_length + 1, game_difficulty == 1 ? "EASY" : "HARD\n");
}

void Logic() {
	if (direction == STOP)
		return;

	if (tail_length > 0) {
		if (current_tail >= tail_length)
			current_tail = 0;

		tail_x[current_tail] = x;
		tail_y[current_tail ++] = y;
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
		case STOP:
			break;
	}

	for (int i = 0; i < tail_length; i ++)
		if (x == tail_x[i] && y == tail_y[i])
			GameOver();

	if (game_difficulty == 2 && (x >=  width ||  x < 0 || y >= height || y < 0 || ((y == 5 || y == 15) && x >= 7 && x <= 30)))
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
		if (food_y == 5 || food_y == 15)
			food_y = food_y - 1;
		
		tail_x[tail_length - 1] = -1;
		tail_y[tail_length - 1] = -1;
		EatSound();
		
		if (score > max_score) {
			FILE *fp = fopen("./resources/maxScore.data", "w");
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
	sleep(1);
	Clear();
	game_over = true;
	DieSound();
	
	gettimeofday(&end, NULL);
	double elapsed_time = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0) * 1000;
	int hours = (int)elapsed_time / (1000 * 60 * 60);
    int minutes = (int)(elapsed_time / (1000 * 60)) % 60;
    int sec = (int)(elapsed_time / 1000) % 60;
    
	puts("\e[91m          ======================================");
	puts("         =       GAME OVER                      =");
	printf("         =       YOUR SCORE: %-10d         =\n", score);
	printf("         =       ELIPSED TIME: %02d:%02d:%02d         =\n", hours, minutes, sec);
	puts("          ======================================");
	
	puts("\nPress `Enter` key to return to the main menu.");
	BlockSTDIN();
	system("/bin/bash -c read -n1");
	
	MainMenu();
}

void EatSound() {
	system("aplay ./resources/eat.wav > /dev/null 2>&1 &");
}

void DieSound() {
	system("aplay ./resources/die.wav > /dev/null 2>&1 &");
}

void Clear() {
	system("clear");
}

void ShowHideCursor(bool status) {
	if (status)
		printf("\e[?25h");
	else
		printf("\e[?25l");
}

void BlockSTDIN() {
	int flags = fcntl(STDIN_FILENO, F_GETFL);
	flags &= ~O_NONBLOCK;
	fcntl(STDIN_FILENO, F_SETFL, flags);
}

void UnblockSTDIN() {
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}
