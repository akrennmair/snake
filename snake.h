#ifndef SNAKE_H_INCLUDED
#define SNAKE_H_INCLUDED

#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4

#define BOUNDARY_CHAR 'X'

#define FILE_PERMS (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) 

#define TO_BE_ADDED 8

typedef struct sn_elem {
  unsigned int x, y;
  int dir;
} snake_elem;


typedef struct hsl {
  char name[20];
  unsigned int pts;
} hs_list; 

struct goodie_t {
  unsigned int x, y;
  unsigned int pts;
  int eaten;
} goodie;


void init_game(void);
void quit_game(void);
int make_a_move(void);
int kbhit(void);
void set_new_goodie(void);
void read_config(void);
void draw_boundary(void);
void draw_score(void);
int cmp_hs(const void *, const void *);

#endif
