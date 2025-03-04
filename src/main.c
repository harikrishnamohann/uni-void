#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "../include/arena.h"

typedef enum {
  invalid, up, down, left, right
} direction;

struct gmatrix {
  uint16_t curs_x;
  uint16_t curs_y;
  uint16_t order;
  int** mat;
};

static void swap(int *x, int *y) { *x = *x ^ *y; *y = *x ^ *y; *x = *x ^ *y; }

// return 1 if elements are sorted.
int render_matrix(const struct gmatrix gmat) {
  int height, width, count = 0, in_place = 0, n_elements = gmat.order * gmat.order;
  getmaxyx(stdscr, height, width);
  erase();
  for (int i = 0; i < gmat.order; i++) {
    move(((height - gmat.order) / 2) + i, (width - gmat.order * 4) / 2);
    for (int j = 0; j < gmat.order; j++) {
      if (i == gmat.curs_x && j == gmat.curs_y) {
        printw("    ");
      } else {
        if (count == gmat.mat[i][j] - 1) {
          in_place++;
          attron(A_BOLD);
          printw("%3d ", gmat.mat[i][j]);
          attroff(A_BOLD);
        } else {
          printw("%3d ", gmat.mat[i][j]);
        }
        count++;
      }
    }
    printw("\n");
  }
  return (in_place == n_elements - 1);
}

direction decode_dir(int ch) {
  switch (ch) {
    case 's' : case 'j' : case KEY_DOWN : return down;
    case 'w' : case 'k' : case KEY_UP : return up;
    case 'd' : case 'l' : case KEY_RIGHT : return right;
    case 'a' : case 'h' : case KEY_LEFT : return left;
  }
  return invalid;
}

int mov_zero(struct gmatrix* gmatrix, int ch) {
  int x = gmatrix->curs_x, y = gmatrix->curs_y;
  switch (decode_dir(ch)) {
    case up :
      if (x == 0) return 0; else x--; break;
    case down :
      if (x == gmatrix->order - 1) return 0; else x++; break;
    case right :
      if (y == gmatrix->order - 1) return 0; else y++; break;
    case left :
      if (y == 0) return 0; else y--; break;
    case invalid : return 0;
  }
  swap(&(gmatrix->mat[x][y]), &(gmatrix->mat[gmatrix->curs_x][gmatrix->curs_y]));
  gmatrix->curs_x = x;
  gmatrix->curs_y = y;
  return 1;
}

void shuffle_matrix(struct gmatrix* gmatrix) {
  int order = gmatrix->order, n = order * order, elements[n], pos;
  elements[0] = 0;
  for (size_t i = 1; i < n; i++) {
    elements[i] = i;
    pos = rand() % i;
    swap(&elements[pos], &elements[i]);
  }
  pos = 0;
  for (int i = 0; i < order; i++) {
    for (int j = 0; j < order; j++) {
      if (elements[pos] == 0) {
        gmatrix->curs_x = i;
        gmatrix->curs_y = j;
      }
      gmatrix->mat[i][j] = elements[pos];
      pos++;
    }
  }
}

struct gmatrix gmatrix_init(Arena* arena, int order) {
  struct gmatrix gmat = {
    .order = order,
    .curs_x = -1,
    .curs_y = -1,
    .mat = arena_alloc(arena, sizeof(int*) * order)
  };
  for (int i = 0; i < order; i++) {
    gmat.mat[i] = arena_alloc(arena, sizeof(int) * order);
  }
  shuffle_matrix(&gmat);
  return gmat;
}

void print_status_line(size_t count, direction key, const char* msg) {
  int x, y, current_x, current_y;
  getyx(stdscr, current_x, current_y);
  getmaxyx(stdscr, y, x);
  int msg_start = (x - strlen(msg)) / 2;

  mvprintw(y - 1, 1, " moves: %2zu", count);
  mvprintw(y - 1, msg_start, "%s", msg);

  move(y - 1, x - 2);
  switch(decode_dir(key)) {
    case up: printw("U"); break;
    case down: printw("D"); break;
    case left: printw("L"); break;
    case right: printw("R"); break;
    case invalid: break;
  }
  mvchgat(y - 1, 0, -1, A_REVERSE, 1, NULL);
  
  move(current_x, current_y);
}

void debug_print_matrix(struct gmatrix mat);

int main(int argc, char* argv[]) {
  int difficulty = 3;
  srand(time(NULL));
  Arena *arena = arena_init(ARENA_128);
  struct gmatrix gmat = gmatrix_init(arena, difficulty);

  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  cbreak();

  int ch = 0, moves = 0, completed = 0;
  char *msg = "sort the matrix";
  render_matrix(gmat);
  print_status_line(moves, ch, msg);
  while (ch != 'q') {
    ch = getch();
    moves += mov_zero(&gmat, ch);
    completed = render_matrix(gmat);
    if (completed) {
      msg = "You Won! press 'q' to quit!";
      ch = 'q';
    }
    print_status_line(moves, ch, msg);
    refresh();
  } 

  if (!completed) print_status_line(moves, ch, "interrupted. press 'q' to quit");
  while(getch() != 'q');

  endwin();
  arena_free(arena);
  return 0;
}













void debug_print_matrix(struct gmatrix mat) {
  for (int i = 0; i < mat.order; i++) {
    for (int j = 0; j < mat.order; j++) {
      printf("%3d ", mat.mat[i][j]);
    }
    printf("\n");
  }
  printf("\norder: %d\n", mat.order);
  printf("curs(%d, %d)\n", mat.curs_x, mat.curs_y);
}

