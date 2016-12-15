/*
	시프 팀 기말 프로젝트 
	작동원리 : 쓰레드 구현해서 메인 쓰레드는 SIGALRM 시그널을 돌면서 서브 쓰레드가 끝나기를 기다리고
	서브 쓰레드는 getchar() 함수로 계속 문자열을 받으면서 위에서 떨어지는 문자열을 제거
	만약 레벨별로 일정한 개수의 문자열을 제거하면 다음 스테이지로 넘어가게됨
   */

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>

#define BLANK "    "
#define NUMBER 50
#define MOUSE1 "@_____@"
#define MOUSE2 "(* . *)"

#define CAT1 "A______A   "
#define CAT2 "<o - o *>~~"

struct temp // 구조체 생성해서 하나씩 떨어지는값을 각각 설정, del 은 지워졋는지 안지워졋는지 여부
{
	char *fall;
	int del; // 0 = 삭제안됨, 1 = 삭제됨
	int row;
	int col;
}temp;

int dir;
int count=0; // 몇개가 떨어졌는지 세는거
char str[5];
int k;
int inputnumber;
int level = 0;
int clear1 = 0;
int strindex;

struct winsize wbuf; // 콘솔창 크기 구할때 쓰는 구조체
struct temp *t;

void gameover();
void enable_signals();
void set_cr_noecho_mode();
int set_ticker(int);
void input();
void nextlevel();
void gameclear();
void gamestart(int);

int main()
{
	initscr();
	set_cr_noecho_mode();
	srand(time(NULL));
	ioctl(0,TIOCGWINSZ, &wbuf);

	firstlevelstart();
}


void gamestart(int level)
{
	int delay;
	int i,j;
	int n;
	char c;
	void move_msg(int);

	pthread_t t1;

	void *get_msg(void *);

	t = (struct temp *)malloc(sizeof(struct temp)*NUMBER);

	pthread_create(&t1, NULL, get_msg, NULL);

	for(i=0;i<NUMBER;i++) // 맨위부터떨어지무로 row값 0, col 의 값은 난수로 설정해서 어디서 떨어지는 모르게 설정
	{
		t[i].del = 0; t[i].row = 0; t[i].col = rand()%(wbuf.ws_col-3);
		t[i].fall = (char *)malloc(sizeof(char)*5);
		for(j=0;j<4;j++)
		{
			n = rand()%4;
			switch(n)
			{
				case 0:
					t[i].fall[j] = 'w';
					break;
				case 1:
					t[i].fall[j] = 'a';
					break;
				case 2:
					t[i].fall[j] = 's';
					break;
				case 3:
					t[i].fall[j] = 'd';
					break;
			}
		}
		t[i].fall[4] = '\0';

	}

	clear();
	refresh();
	dir = 1;
	count = 0;
	switch(level) // 레벨별로 떨어지는 속도 다르게 설정
	{
		case 1:
			delay = 500;
			break;
		case 2:
			delay = 450;
			break;
		case 3:
			delay = 400;
			break;
		case 4:
			delay = 300;
			break;
		case 5:
			delay = 200;
			break;
	}


	strindex = 0;

	mvaddstr(wbuf.ws_row-2,wbuf.ws_col/2,MOUSE1);
	mvaddstr(wbuf.ws_row-1,wbuf.ws_col/2,MOUSE2);
	mvaddstr(1,wbuf.ws_col/2,CAT1);
	mvaddstr(2,wbuf.ws_col/2,CAT2);

	signal(SIGALRM, move_msg);
	set_ticker(delay);

	pthread_join(t1,NULL);
	free(t);
	nextlevel();
}

void move_msg(int signum)
{
	int random,i,j;
	char c[5];

	signal(SIGALRM, move_msg);
	random = (rand()%3)+1; // 약간의 텀을 두고 떨어지게 설정
	if(random == 1) { // 떨어지는 확률은 33% 만약 random의 값이 1 이면 count의 개수를 늘려서 하나더 떨어지게 만듬
		if(count != NUMBER)
			count++;
	}
	for(i=0;i<count;i++) {
		for(j=0;j<count;j++) {
			if(strcmp(t[j].fall,str) == 0 && t[j].del == 0) // 삭제 코드
			{
				mvaddstr(t[j].row+3,t[j].col,BLANK);
				t[j].del = 1;
				k=1;
				clear1++;
			}

		}

		if(t[i].del == 0) { // 떨어지는 코드, 만약 맨 아래까지 떨어지면 gameover
			move(t[i].row+3,t[i].col);
			addstr(BLANK);
			t[i].row += dir;
			if(t[i].row == wbuf.ws_row-5) { 
				gameover();
			}
			move(t[i].row+3,t[i].col);
			addstr(t[i].fall);
			refresh();
		}
	}
}

void set_cr_noecho_mode()
{
	struct termios ttystate;

	tcgetattr(0, &ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	ttystate.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &ttystate);
}

void *get_msg(void *m) // 쓰레드 이용해서 구현 레벨 별로 없애야하는 글자수를 다르게 해서 클리어 조건 다르게 설정
{
	while(1)
	{
		str[strindex] = getchar();
		mvaddstr(wbuf.ws_row-1,0,"CURRENT INPUT : ");
		mvaddstr(wbuf.ws_row-1,15,str);
		if (str[strindex]=='b' && strindex > 0)//backspace
		{
			strindex--;
		}
		else
			strindex++;
		if(strindex==4) {
			str[strindex] = '\0';
			strindex=0;
		}
		switch(level) // 레벨별 클리어 개수 설정
		{
			case 1:
				if(clear1 > 15) {
					nextlevel();
					return NULL;
				}
			case 2:
				if(clear1 > 20) {
					nextlevel();
					return NULL;
				}
			case 3:
				if(clear1 > 25) {
					nextlevel();
					return NULL;
				}
			case 4:
				if(clear1 > 35) {
					nextlevel();
					return NULL;
				}
			case 5:
				if(clear1 > 45) {
					gameclear();
					return NULL;
				}


		}
	}

}

void gameover()
{
	signal(SIGALRM, SIG_IGN);
	clear();
	
	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2-10,"~(%:> Game over <:3)~");
	refresh();
	sleep(3);

	endwin();
	exit(0);

}

void nextlevel()
{
	int i;
	signal(SIGALRM, SIG_IGN);
	clear1 = 0;

	clear();
	
	level++;
	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2-10, "3 second later Next level start!");
	refresh();
	sleep(3);

	strindex = 0;	
	gamestart(level);

}

void gameclear()
{
	signal(SIGALRM, SIG_IGN);
	clear();

	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2,"Game clear!!");
	refresh();
	sleep(2);

	endwin();
	exit(0);
}

void firstlevelstart()
{
	int i;
	signal(SIGALRM, SIG_IGN);
	clear1 = 0;

	clear();
	
	level++;
	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2-10, "Welcome To CATCHCATS!");
	mvaddstr(wbuf.ws_row/2+1,wbuf.ws_col/2-10, "After 3s, GAME START!");
	refresh();
	sleep(3);

	strindex = 0;	
	gamestart(level);

}
