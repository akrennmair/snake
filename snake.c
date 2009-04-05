/* 
   (c) 2000-2001 Andreas Krennmair 
   SNAKE is free software; you can redistribute it and/or modify under the
   terms of the GNU General Public License as published by the Free Software
   Foundation; either version 2, or (at your opinion) any later version.

   SNAKE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License along with
   SNAKE; see the file COPYING. If not, write to the Free Software Foundation,
   675 Mass Ave, Cambridge, MA 02139, USA. 
*/

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

snake_elem * thesnake;
snake_elem oldpos;
unsigned int len_snake = 10;
unsigned long points = 0;
unsigned int count=0;

unsigned int height = 25, width = 80;

char key_up = 'i';
char key_down = 'k';
char key_left = 'j';
char key_right = 'l';

void usage(const char * argv0) {
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
  } 
  while (status!=0);
  quit_game();
  return 0;
}

/* procedure for handling signals and stuff */
void finish(int sig){
  endwin();
  fprintf(stderr,"Caught signal %d, exiting...\n",sig);
  exit(-1);
}

/* redraws the snake */
void draw_snake(snake_elem * snake,int num_elem){
  int i;
  unsigned int x, y;
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
  for (i=0;i<num_elem;++i){
    move(snake[i].y,snake[i].x);
    addch('o');
  }
  refresh();
}

/* initializes the terminal and prepares the snake */
void init_game(void){
  initscr();
  signal(SIGTERM,finish);
  signal(SIGINT,finish);
  cbreak();
  noecho();
  getmaxyx(stdscr, height, width);
  thesnake=malloc(len_snake*sizeof(snake_elem));
  {
    unsigned int i;
    for (i=0;i<len_snake;++i){
      thesnake[i].x=width/2+i;
      thesnake[i].y=height/2;
      thesnake[i].dir=LEFT;
    }
  }
  set_new_goodie();
  oldpos.x=width/2+len_snake;
  oldpos.y=height;
  oldpos.dir=LEFT;
  draw_snake(thesnake,len_snake);
}

/* procedure to quit the game (restoring terminal, freeing up memory) */
void quit_game(void){
  FILE * f;
  hs_list hs[10], entry;
  erase();
  refresh();
  endwin();
  /* free the snake!!! */
  free(thesnake);
  /* printf("Points: %d\n",(int)points); */
  try_again:
  if ((f=fopen("/var/lib/games/snake.score","rb"))==NULL){
    if ((f=fopen("/var/lib/games/snake.score","wb"))==NULL){
      fprintf(stderr,"Error: couldn't create /var/lib/games/snake.score!\n");
    }
    else {
      strcpy(hs[0].name,"Linus Torvalds");     /* Linux kernel */
      strcpy(hs[1].name,"Richard M Stallman"); /* GNU software */
      strcpy(hs[2].name,"Larry Wall");         /* Perl */
      strcpy(hs[3].name,"Dennis M Ritchie");   /* Programming language C */
      strcpy(hs[4].name,"Alan Cox");           /* Kernel hacker */
      strcpy(hs[5].name,"Eric S. Raymond");    /* "Cathedral and Bazaar" */
      strcpy(hs[6].name,"Dirk Hohndel");       /* XFree86 */
      strcpy(hs[7].name,"Ken Thompson");       /* Unix */
      strcpy(hs[8].name,"Niklaus Wirth");      /* Pascal */
      strcpy(hs[9].name,"Donald Knuth");       /* TeX, KMP search algorithm */
      {
        int i;
        for (i=9;i>=0;--i)
          hs[9-i].pts=(i+1)*20;
      }
      fwrite(hs,sizeof(hs_list), 10, f);
      fclose(f);
      chmod("/var/lib/games/snake.score",FILE_PERMS);
      goto try_again;
    }
  }
  else {
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
    {
      int i;
      for (i=0;i<10;++i){
        printf("%2d  %19s %4d\n",i+1,hs[i].name, hs[i].pts);
      }
    }
    if ((f=fopen("/var/lib/games/snake.score","wb"))==NULL){
      fprintf(stderr, "Error: couldn\'t create /var/lib/games/snake.score!\n");
    } 
    else {
      fwrite(hs, sizeof(hs_list), 10, f);
      fclose(f);
      chmod("/var/lib/games/snake.score",FILE_PERMS);
    }
  } 
  exit(0);
}

/* procedure to handle the snake's moves */
int make_a_move(void){
  char ch;
  unsigned int i;
  if (goodie.eaten) {
    if (--count > 0)
      thesnake=(snake_elem *)realloc(thesnake,++len_snake*sizeof(snake_elem)); 
    else
      set_new_goodie();
  }
  else {
    if (goodie.x==thesnake[0].x && goodie.y==thesnake[0].y){
      goodie.eaten=1;
      points+=goodie.pts;
      count=TO_BE_ADDED;
    }
  }
  oldpos=thesnake[len_snake-1];
  for (i=len_snake-1;i>0;--i){
    thesnake[i]=thesnake[i-1];
  }
  if (kbhit()){
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
        }
        else {
          if (ch==key_left) {
    	    if (thesnake[1].dir!=RIGHT) 
              thesnake[0].dir=LEFT;
	  }
          else {
            if (ch==key_right) {
 	      if (thesnake[1].dir!=LEFT)
                thesnake[0].dir=RIGHT;
	    } /* if   */
          } /* else */
        } /* else */
      } /* else */
    } /* else */
  } /* if   */
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
  } /* switch */
  if (thesnake[0].x==0 || thesnake[0].x==width-1 || 
      thesnake[0].y==0 || thesnake[0].y==height-1)
  {
    return 0;
  } /* if */
  for (i=1;i<len_snake;++i){
    if (thesnake[0].x==thesnake[i].x && thesnake[0].y==thesnake[i].y)
      return 0;
  }
  draw_snake(thesnake,len_snake);
  draw_boundary();
  draw_score();
  return 1;
} /* make_a_move */


/* returns !=0 if a key was hit; the value of the key stays in buffer */
/* (can be used just as the Turbo-C-conio.h-kbhit)                    */
int kbhit(void){
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
void set_new_goodie(void){
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
void read_config(void){
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
                          key_up,  key_down,  key_left,  key_right);
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

void draw_boundary(void){
  unsigned int i;
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
  refresh();
} 

void draw_score(void){
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

int cmp_hs(const void * elem1, const void * elem2){
  return ((hs_list *)elem2)->pts - ((hs_list *)elem1)->pts;
} 

/* here's the end of the file */
