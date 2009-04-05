#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "snake.h"

#define BOUNDARY_CHAR ' '

static goodie_t goodie;
static snake_elem * thesnake;
static snake_elem oldpos;
static unsigned int len_snake = 10;
static unsigned long points = 0;
static unsigned int count=0;
char step = 0;

static unsigned int height = 25, width = 80;

static char key_up = 'i';
static char key_down = 'k';
static char key_left = 'j';
static char key_right = 'l';

static void usage(const char * argv0) {
	fprintf(stderr, "%s: usage: %s\n", argv0, argv0);
	fprintf(stderr, "Available keys:\n");
	fprintf(stderr, "\tUp:\t%c\n\tDown:\t%c\n\tLeft:\t%c\n\tRight:\t%c\n", key_up, key_down, key_left, key_right);
	exit(1);
}

/* the main program */
int main(int argc, char * argv[]){
	int status;
	read_config();
	if (argc > 1) {
		usage(argv[0]);
	}
	init_game();
	do {
		status=make_a_move();
		usleep(100*1000); /* 100 ms */
	} while (status!=0);
	quit_game();
	return 0;
}

/* procedure for handling signals and stuff */
static void finish(int sig){
	endwin();
	fprintf(stderr,"Caught signal %d, exiting...\n",sig);
	exit(1);
}

/* redraws the snake */
static void draw_snake(snake_elem * snake,int num_elem){
	int i;
	unsigned int x, y;
	wattrset(stdscr, A_BOLD);
	if (count<1) {
		for (x=oldpos.x-1;x<=oldpos.x+1;++x) {
			for (y=oldpos.y-1;y<=oldpos.y+1;++y){
				move(y,x);
				addch(' ');
			}
		}
	}
	refresh();
	if (!goodie.eaten) {
		move(goodie.y,goodie.x);
		addch('*');
		move(oldpos.y,oldpos.x);
		addch(' ');
	}
	move(snake[0].y,snake[0].x);
	switch (snake[0].dir) {
		case LEFT:
			addch(step & 1 ? '>' : '='); break;
		case RIGHT:
			addch(step & 1 ? '<' : '='); break;
		case UP:
			addch(step & 1 ? 'V' : 'H'); break;
		case DOWN:
			addch(step & 1 ? 'A' : 'H'); break;
	}
	step = (step + 1) & 1;
	for (i=1;i<num_elem;++i){
		move(snake[i].y,snake[i].x);
		switch (snake[i].dir) {
			case LEFT:
			case RIGHT:
				addch('.'); break;
			case UP:
			case DOWN:
				addch('.'); break;
		}
	}
	wattrset(stdscr, 0);
	refresh();
}

/* initializes the terminal and prepares the snake */
static void init_game(void){
	unsigned int i;
	initscr();
	signal(SIGTERM,finish);
	signal(SIGINT,finish);
	cbreak();
	noecho();
	start_color();
	init_pair(0, -1,  -1);
	init_pair(1, COLOR_YELLOW, COLOR_YELLOW);
	getmaxyx(stdscr, height, width);
	srand(time(NULL) ^ getuid());
	thesnake=malloc(len_snake*sizeof(snake_elem));
	for (i=0;i<len_snake;++i){
		thesnake[i].x=width/2+i;
		thesnake[i].y=height/2;
		thesnake[i].dir=LEFT;
	}
	set_new_goodie();
	oldpos.x=width/2+len_snake;
	oldpos.y=height;
	oldpos.dir=LEFT;
	draw_snake(thesnake,len_snake);
}

/* procedure to quit the game (restoring terminal, freeing up memory) */
static void quit_game(void){
	FILE * f;
	hs_list hs[10], entry;
	erase();
	refresh();
	endwin();
	/* free the snake!!! */
	free(thesnake);
	printf("Final score: %lu\n",points);
try_again:
	if ((f=fopen("/var/lib/games/snake.score","rb"))==NULL){
		if ((f=fopen("/var/lib/games/snake.score","wb"))==NULL){
			fprintf(stderr,"Error: couldn't create /var/lib/games/snake.score!\n");
		} else {
			int i;
			memset(hs, 0, sizeof(hs));
			for (i=9;i>=0;--i)
				hs[9-i].pts=(i+1)*20;
			fwrite(hs,sizeof(hs_list), 10, f);
			fclose(f);
			chmod("/var/lib/games/snake.score",FILE_PERMS);
			goto try_again;
		}
	} else {
		unsigned int i;
		fread(hs, sizeof(hs_list), 10, f);
		fclose(f);
		if (points>=hs[9].pts) {
			while (kbhit()) getch();
			endwin();
			printf("You have enough points to be on the highscore list!\n");
			printf("What's your name? ");
			fflush(stdout);
			scanf("%19s",entry.name);
			entry.pts=points;
			hs[9]=entry;
			qsort(hs, 10, sizeof(hs_list), cmp_hs);
		}
		printf("\nHighscores:\n");
		printf("No                Name  Points\n");
		for (i=0;i<10;++i) {
			printf("%2d %19s %4d\n",i+1,hs[i].name, hs[i].pts);
		}
		if ((f=fopen("/var/lib/games/snake.score","wb"))==NULL){
			fprintf(stderr, "Error: couldn't create /var/lib/games/snake.score!\n");
		} 
		else {
			fwrite(hs, sizeof(hs_list), 10, f);
			fclose(f);
			chmod("/var/lib/games/snake.score",FILE_PERMS);
		}
	} 
}

/* procedure to handle the snake's moves */
static int make_a_move(void){
	char ch;
	unsigned int i;
	if (goodie.eaten) {
		if (--count > 0)
			thesnake=(snake_elem *)realloc(thesnake,++len_snake*sizeof(snake_elem)); 
		else
			set_new_goodie();
	} else {
		if (goodie.x==thesnake[0].x && goodie.y==thesnake[0].y) {
			goodie.eaten=1;
			points+=goodie.pts;
			count=TO_BE_ADDED;
		}
	}
	oldpos=thesnake[len_snake-1];
	for (i=len_snake-1;i>0;--i) {
		thesnake[i]=thesnake[i-1];
	}
	if (kbhit()) {
		ch=getch();
		if (ch=='q') {
			return 0;
		}
		else {
			if (ch==key_up) {
				if (thesnake[1].dir!=DOWN)
					thesnake[0].dir=UP;
			}
			else {
				if (ch==key_down) {
					if (thesnake[1].dir!=UP)
						thesnake[0].dir=DOWN;
				} else {
					if (ch==key_left) {
						if (thesnake[1].dir!=RIGHT) 
							thesnake[0].dir=LEFT;
					} else {
						if (ch==key_right) {
							if (thesnake[1].dir!=LEFT)
								thesnake[0].dir=RIGHT;
						}
					}
				}
			}
		}
	}
	switch (thesnake[0].dir){
		case UP:
			thesnake[0].y--;
			break;
		case DOWN:
			thesnake[0].y++;
			break;
		case LEFT:
			thesnake[0].x--;
			break;
		case RIGHT:
			thesnake[0].x++;
			break;
	}
	if (thesnake[0].x==0 || thesnake[0].x==width-1 || 
			thesnake[0].y==0 || thesnake[0].y==height-1) {
		return 0;
	}
	for (i=1;i<len_snake;++i) {
		if (thesnake[0].x==thesnake[i].x && thesnake[0].y==thesnake[i].y)
			return 0;
	}
	draw_snake(thesnake,len_snake);
	draw_boundary();
	draw_score();
	return 1;
}

/* returns !=0 if a key was hit; the value of the key stays in buffer */
/* (can be used just as the Turbo-C-conio.h-kbhit)                    */
static int kbhit(void) {
	fd_set rfds;
	struct timeval tv;
	int taste;
	FD_ZERO(&rfds);
	FD_SET(0,&rfds);
	tv.tv_sec=0;
	tv.tv_usec=100;
	taste=select(1,&rfds,NULL,NULL,&tv);
	return(taste);
}


/* procedure to set a new goodie */
static void set_new_goodie(void){
	int isonthesnake;
	unsigned int i;
	do {
		isonthesnake=1;	
		goodie.x=(int)1.0+(((float)(width-2))*rand()/(RAND_MAX+1.0));
		goodie.y=(int)1.0+(((float)(height-2))*rand()/(RAND_MAX+1.0));
		for (i=0;i<len_snake;++i){
			if (thesnake[i].x==goodie.x && thesnake[i].y==goodie.y){
				isonthesnake=0;
			}
		}
	}
	while (!isonthesnake);
	goodie.eaten=0;
	goodie.pts=20;
}


/* read the configuration file at startup */
static void read_config(void){
	FILE * configfile;
	char line[10];
	char userpath[255];
	const char * homedir = getenv("HOME");
	if (!homedir) {
		struct passwd * spw = getpwuid(getuid());
		if (!spw) {
			fprintf(stderr, "Error: I don't exist! Please set $HOME or create a user for UID %u\nso that I know from where I can load my configuration!\n", getuid());
			exit(1);
		}
		homedir = spw->pw_dir;
	}
	snprintf(userpath, sizeof(userpath), "%s/.snake", homedir);
	if ((configfile=fopen(userpath,"rt"))==NULL){
		if ((configfile=fopen(userpath,"wt"))!=NULL){
			fprintf(configfile,"%c # up\n%c # down\n%c # left\n%c # right\n",
													key_up,	key_down,	key_left,	key_right);
		}
	}
	else {
		fgets(line,100,configfile);
		key_up=*line;
		fgets(line,100,configfile);
		key_down=*line;
		fgets(line,100,configfile);
		key_left=*line;
		fgets(line,100,configfile);
		key_right=*line;
	}
}

static void draw_boundary(void){
	unsigned int i;
	wattrset(stdscr, COLOR_PAIR(1));
	move(0,0);
	for (i=0;i<width;++i)
		addch(BOUNDARY_CHAR);
	for (i=1;i<height-1;++i){
		move(i,0);
		addch(BOUNDARY_CHAR);
		move(i,width-1);
		addch(BOUNDARY_CHAR);
	}
	move(height-1,0);
	for (i=0;i<width;++i)
		addch(BOUNDARY_CHAR);
	wattrset(stdscr, 0);
	refresh();
}

static void draw_score(void){
	char pointstr[20];
	unsigned int i;
	move(0,3);
	snprintf(pointstr, sizeof(pointstr), "%lu", points);
	addch(' ');
	for (i=0;i<strlen(pointstr);++i)
		addch(pointstr[i]);
	addch(' ');
	refresh();
}

static int cmp_hs(const void * elem1, const void * elem2){
	return ((hs_list *)elem2)->pts - ((hs_list *)elem1)->pts;
}

