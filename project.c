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

#define MESSAGE "o"
#define BLANK "    "
#define NUMBER 50
#define MOUSE1 "@_____@"
#define MOUSE2 "(* . *)"

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

pthread_t t1;

int main()
{
	initscr();
	set_cr_noecho_mode();
	srand(time(NULL));
	ioctl(0,TIOCGWINSZ, &wbuf);

	nextlevel();
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
	delay = 400;
	count = 0;

	strindex = 0;

	mvaddstr(wbuf.ws_row-2,wbuf.ws_col/2,MOUSE1);
	mvaddstr(wbuf.ws_row-1,wbuf.ws_col/2,MOUSE2);

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
	random = (rand()%4)+1; // 약간의 텀을 두고 떨어지게 설정
	if(random != 1) { // 떨어지는 확률은 66% 만약 random의 값이 1 이아니면 count의 개수를 늘려서 하나더 떨어지게 만듬
		if(count != NUMBER)
			count++;
	}
	for(i=0;i<count;i++) {
		for(j=0;j<count;j++) {
			if(strcmp(t[j].fall,str) == 0 && t[j].del == 0)
			{
				mvaddstr(t[j].row,t[j].col,BLANK);
				t[j].del = 1;
				k=1;
				clear1++;
			}

		}


		if(t[i].del == 0) { // 삭제 안됫으면 ( 삭제 구현시 수정 필요)
			move(t[i].row,t[i].col);
			addstr(BLANK);
			t[i].row += dir;
			if(t[i].row == wbuf.ws_row-2) { 
				gameover();
			}
			move(t[i].row,t[i].col);
			addstr(t[i].fall);
			refresh();
		}
	}
	/*signal(SIGALRM, move_msg);
	move(row, col);
	addstr(BLANK);
	row += dir;
	move(row,col);
	addstr(MESSAGE);
	refresh();
	*/
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

void *get_msg(void *m)
{
	while(1)
	{
		str[strindex] = getchar();
		mvaddstr(wbuf.ws_row-1,0,str);
		strindex++;
		if(strindex==4) {
			str[strindex] = '\0';
			strindex=0;
		}
		
	}

}

void gameover()
{
	signal(SIGALRM, SIG_IGN);
	clear();
	
	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2,"Game over TT");
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
	mvaddstr(wbuf.ws_row/2,wbuf.ws_col/2-5, "3 second later Next level strat!");
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
