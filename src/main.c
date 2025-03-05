#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/arena.h"

#define STK_SIZE 50

typedef enum {
  key_invalid,
  key_up = 1,
  key_down = -1,
  key_left = 2,
  key_right = -2,
  key_return = 3,
  key_undo,
  key_redo,
  key_exit,
  key_resize,
} Key;

typedef enum {
  mode_invalid,
  mode_easy = 0,
  mode_normal = 1,
  mode_hard = 2,
  mode_custom = 3,
  mode_exit = 4,
} Mode;

struct gmatrix {
  uint16_t curs_x;
  uint16_t curs_y;
  uint16_t order;
  Key undo_stack[STK_SIZE];
  Key redo_stack[STK_SIZE];
  int16_t utop;
  int16_t rtop;
  int** mat;
};

static void swap(int *x, int *y) { *x = *x ^ *y; *y = *x ^ *y; *x = *x ^ *y; }

void push_key(Key stk[], int16_t *top, Key key) {
  if (*top == STK_SIZE - 1) { // stack full
    for (int i = 1; i < STK_SIZE; i++) {
      stk[i - 1] = stk[i];
    }
    (*top)--;
  }  
  stk[++(*top)] = key;
}

Key pop_key(Key stk[], int16_t *top) {
  if (*top == -1) {
    return key_invalid;
  }
  return stk[(*top)--];
}

// return 1 if elements are sorted.
int update_matrix_view(const struct gmatrix gmat) {
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

Key decode_key(int ch) {
  switch (ch) {
    case 's' : case 'j' : case KEY_DOWN : return key_down;
    case 'w' : case 'k' : case KEY_UP : return key_up;
    case 'd' : case 'l' : case KEY_RIGHT : return key_right;
    case 'a' : case 'h' : case KEY_LEFT : return key_left;
    case KEY_ENTER : case '\n' :  return key_return;
    case 'u' : return key_undo;
    case 'r' : return key_redo;
    case 'q' : return key_exit;
    case KEY_RESIZE : return key_resize;
  }
  return key_invalid;
}

int mov_zero(struct gmatrix* gmat, Key key, bool undoing) {
  int x = gmat->curs_x, y = gmat->curs_y;
  switch (key) {
    case key_up :
      if (x == 0) return 0; else x--; break;
    case key_down :
      if (x == gmat->order - 1) return 0; else x++; break;
    case key_right :
      if (y == gmat->order - 1) return 0; else y++; break;
    case key_left :
      if (y == 0) return 0; else y--; break;
    default : return 0;
  }

  if (!undoing) {
    push_key(gmat->undo_stack, &gmat->utop, key * -1);
  }
  swap(&(gmat->mat[x][y]), &(gmat->mat[gmat->curs_x][gmat->curs_y]));
  gmat->curs_x = x;
  gmat->curs_y = y;
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
    .mat = arena_alloc(arena, sizeof(int*) * order),
    .utop = -1,
    .rtop = -1,
  };
  for (int i = 0; i < order; i++) {
    gmat.mat[i] = arena_alloc(arena, sizeof(int) * order);
  }
  shuffle_matrix(&gmat);
  return gmat;
}

void print_status_line(size_t count, Key key, const char* msg) {
  int x, y, current_x, current_y;
  getyx(stdscr, current_x, current_y);
  getmaxyx(stdscr, y, x);
  int msg_start = (x - strlen(msg)) / 2;

  mvprintw(y - 1, 1, " moves: %2zu", count);
  mvprintw(y - 1, msg_start, "%s", msg);

  move(y - 1, x - 2);
  switch(key) {
    case key_up: printw("U"); break;
    case key_down: printw("D"); break;
    case key_left: printw("L"); break;
    case key_right: printw("R"); break;
    default: break;
  }
  mvchgat(y - 1, 0, -1, A_REVERSE, 1, NULL);
  
  move(current_x, current_y);
}

void show_menu(Key key, uint *highlight) {
  int height, width, x, y;
  char* menu_items[] = {
    "  easy  ",
    " normal ",
    "  hard  ",
    " custom ",
    "  exit  ",
  };
  size_t menu_size = sizeof(menu_items) / sizeof(char*);

  if (key == key_down)
    *highlight = (*highlight + 1) % menu_size;
  else if (key == key_up)
    *highlight = (*highlight == 0) ? menu_size - 1 : *highlight - 1;

  getmaxyx(stdscr, height, width);
  x = (width - strlen("Welcome to uni-void!")) / 2;
  y = (height - (3 + menu_size)) / 2;

  erase();
  attron(A_BOLD );
  mvprintw(y++, x, "WELCOME TO UNI-VOID!");
  attroff(A_BOLD);
  y++;
  attron(A_UNDERLINE);
  mvprintw(y++, x, "Choose a difficulty");
  attroff(A_UNDERLINE);

  for (size_t i = 0; i < menu_size; i++) {
    x = (width - strlen(menu_items[i])) / 2;
    if (i == *highlight) {
      attron(A_REVERSE | A_BOLD);
      mvprintw(y++, x, "%s", menu_items[i]);    
      attroff(A_REVERSE | A_BOLD);
    } else {
      mvprintw(y++, x, "%s", menu_items[i]);    
    }
  } 
}

int input_difficulty() {
  erase();
  char difficulty[10];
  int x, y, height, width;
  getmaxyx(stdscr, height, width);
  char* query = "Enter order of square matrix: ";
  x = (width - strlen(query)) / 2;
  y = height / 2;
  mvprintw(y, x, "%s", query);
  echo();
  nocbreak();
  curs_set(1);
  refresh();
  getstr(difficulty);
  noecho();
  cbreak();
  curs_set(0);
  return atoi(difficulty);
}

int get_difficulty() {
  uint highlight = 0;
  int ch;
  Key key;
  do {
    key = decode_key(ch);
    if (key == key_return) {
      if (highlight == mode_exit) {
        break;
      } else if (highlight == mode_custom) {
        return input_difficulty();
      } else {
        return highlight + 3;
      }
    } else {
      show_menu(key, &highlight);
    }
    refresh();
  } while ((ch = getch()) != 'q');
  endwin();
  exit(0);
}

int main(int argc, char* argv[]) {
  int difficulty = 0;
  srand(time(NULL));

  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  cbreak();


  difficulty = get_difficulty();

  Arena *arena = arena_init(ARENA_128);
  struct gmatrix gmat = gmatrix_init(arena, difficulty);

  int moves = 0, completed = 0;
  Key key;
  char *msg = "sort the matrix";
  update_matrix_view(gmat);
  print_status_line(moves, key, msg);

  bool undo;
  while (key != key_exit) {
    undo = false;
    key = decode_key(getch());

    if (key == key_invalid || key == key_exit) continue;
    else if(key == key_undo) {
      if ((key = pop_key(gmat.undo_stack, &gmat.utop)) == key_invalid) continue;
      push_key(gmat.redo_stack, &gmat.rtop, key * -1);
      undo = true;
    } else if (key == key_redo) {
      if ((key = pop_key(gmat.redo_stack, &gmat.rtop)) == key_invalid) continue;
      push_key(gmat.undo_stack, &gmat.utop, key * -1);
      undo = true;
    }
    moves += mov_zero(&gmat, key, undo);
    completed = update_matrix_view(gmat);
    if (completed) {
      msg = "You Won! press 'q' to quit!";
      key = key_exit;
    }
    print_status_line(moves, key, msg);
    refresh();
  } 

  if (!completed) print_status_line(moves, key, "interrupted. press 'q' to quit");
  while(getch() != 'q');

  endwin();
  arena_free(arena);
  return 0;
}
