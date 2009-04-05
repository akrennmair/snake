#ifndef SNAKE_H_INCLUDED
#define SNAKE_H_INCLUDED

enum direction { LEFT = 1, RIGHT, UP, DOWN };

#define FILE_PERMS (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) 

#define TO_BE_ADDED 8

typedef struct sn_elem {
  unsigned int x, y;
  enum direction dir;
} snake_elem;

typedef struct hsl {
  char name[20];
  unsigned int pts;
} hs_list; 

typedef struct {
  unsigned int x, y;
  unsigned int pts;
  int eaten;
} goodie_t;

static void init_game(void);
static void quit_game(void);
static int make_a_move(void);
static int kbhit(void);
static void set_new_goodie(void);
static void read_config(void);
static void draw_boundary(void);
static void draw_score(void);
static int cmp_hs(const void *, const void *);

#endif
