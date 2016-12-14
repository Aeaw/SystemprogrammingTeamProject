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

#define MESSAGE "o"
#define BLANK "    "
#define TEMP_ARRAY 30 // 임시로 만든 array 개수

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

struct winsize wbuf; // 콘솔창 크기 구할때 쓰는 구조체
struct temp t[TEMP_ARRAY];

void gameover();
void enable_signals();
void set_cr_noecho_mode();
int set_ticker(int);
void input();


int main()
{
	int delay;
	int i,j;
	int n;
	char c;
	void move_msg(int);

	void *get_msg(void *);

	srand(time(NULL)); // 랜덤 난수 생성
	ioctl(0,TIOCGWINSZ, &wbuf); 

	for(i=0;i<TEMP_ARRAY;i++) // 맨위부터떨어지무로 row값 0, col 의 값은 난수로 설정해서 어디서 떨어지는 모르게 설정
	{
		t[i].del = 0; t[i].row = 0; t[i].col = rand()%wbuf.ws_col;
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

	}

	initscr();
	//set_cr_noecho_mode();
	clear();

	dir = 1;
	delay = 400;

	signal(SIGIO,input);
	enable_signals();

	move(t[0].row,t[0].col); // 처음에 하나 떨어지는 공 설정
	addstr(t[0].fall);
	signal(SIGALRM, move_msg);
	set_ticker(delay);

	while(1)
	{
		pause();
		/*fgets(str,sizeof(str),stdin);
		//str[strlen(str)-1] = '\0';

	
			
		if(k==0) {
			mvaddstr(wbuf.ws_row-1,0,"Wrong!");
		}
		else
			mvaddstr(wbuf.ws_row-1,0,"      ");*/

		/*for(i=0;i<=count; i++)
			if(t[i].row == wbuf.ws_row) { // 만약 어떤 거라도 바닥에 떨어지게되면 프로그램 종료
				endwin();
				return 0;
			}*/
			
	}
	endwin();
	return 0;
}

void move_msg(int signum)
{
	int random,i,j;

	signal(SIGALRM, move_msg);
	random = (rand()%3)+1; // 약간의 텀을 두고 떨어지게 설정
	if(random != 1) { // 떨어지는 확률은 66% 만약 random의 값이 1 이아니면 count의 개수를 늘려서 하나더 떨어지게 만듬
		if(count != TEMP_ARRAY)
			count++;
	}
	for(i=0;i<count;i++) {
		for(j=0;j<count;j++) {
			if(strcmp(t[j].fall,str) == 0 && t[j].del == 0)
			{
				mvaddstr(t[j].row,t[j].col,BLANK);
				t[j].del = 1;
				k=1;
			}

		}


		if(t[i].del == 0) { // 삭제 안됫으면 ( 삭제 구현시 수정 필요)
			move(t[i].row,t[i].col);
			addstr(BLANK);
			t[i].row += dir;
			if(t[i].row == wbuf.ws_row) { 
				//gameover();
				endwin();
				exit(0);
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

void input()
{
	char c;

	for(inputnumber=0; inputnumber<4; inputnumber++)
	{
		str[inputnumber] = getchar();
		mvaddstr(wbuf.ws_row-1,0,str);
	}
	return ;

}

void enable_signals()
{
	int flags;

	fcntl(0, F_SETOWN, getpid());
	flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL,(flags | O_ASYNC));
}
