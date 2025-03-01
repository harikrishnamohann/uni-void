#include <ncurses.h>
#include <stdlib.h>
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

void print_matrix(const struct gmatrix gmatrix) {
  int height, width;
  getmaxyx(stdscr, height, width);
  erase();
  for (int i = 0; i < gmatrix.order; i++) {
    move(((height - gmatrix.order) / 2) + i, (width - gmatrix.order * 4) / 2);
    for (int j = 0; j < gmatrix.order; j++) {
      if (i == gmatrix.curs_x && j == gmatrix.curs_y) {
        printw("    ");
      } else {
        printw("%3d ", gmatrix.mat[i][j]);
      }
    }
    printw("\n");
  }
  refresh();
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

void mov_zero(struct gmatrix* gmatrix, int ch) {
  int x = gmatrix->curs_x, y = gmatrix->curs_y;
  switch (decode_dir(ch)) {
    case up : if (x != 0) x--; break;
    case down : if (x != gmatrix->order - 1) x++; break;
    case right : if (y != gmatrix->order - 1) y++; break;
    case left : if (y != 0) y--; break;
    case invalid : break;
  }
  swap(&(gmatrix->mat[x][y]), &(gmatrix->mat[gmatrix->curs_x][gmatrix->curs_y]));
  gmatrix->curs_x = x;
  gmatrix->curs_y = y;
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

void debug_print_matrix(struct gmatrix mat);

int main(int argc, char* argv[]) {
  srand(time(NULL));
  Arena *arena = arena_init(ARENA_128);
  struct gmatrix gmat = gmatrix_init(arena, 4);

  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);

  int ch;
  print_matrix(gmat);
  while ((ch = getch()) != 'q') {
    mov_zero(&gmat, ch);
    print_matrix(gmat);
    refresh();
  }

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

