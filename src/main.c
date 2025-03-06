#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/arena.h"

#define STK_SIZE 50
#define CENTER_Y(height) (((LINES - (height)) / 2))
#define CENTER_X(width) (((COLS - (width)) / 2))

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
  key_usage,
} Key;

typedef enum {
  mode_load,
  mode_easy,
  mode_normal,
  mode_hard,
  mode_custom,
  mode_exit,
} Mode;

struct status_line {
  size_t moves;
  char *msg;
  Key key;
};

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

struct game_obj {
  Mode mode;
  struct gmatrix gmat;
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
bool update_matrix_view(const struct gmatrix gmat) {
  int count = 0, in_place = 0, n_elements = gmat.order * gmat.order;
  erase();
  
  for (int i = 0; i < gmat.order; i++) {
    move(CENTER_Y(gmat.order) + i, CENTER_X(gmat.order * 4));
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
    case '?' : return key_usage;
  }
  return key_invalid;
}

void mov_zero(struct gmatrix* gmat, struct status_line *sl, Key key, bool undoing) {
  int x = gmat->curs_x, y = gmat->curs_y;
  switch (key) {
    case key_up :
      if (x == 0) return; else x--; break;
    case key_down :
      if (x == gmat->order - 1) return; else x++; break;
    case key_right :
      if (y == gmat->order - 1) return; else y++; break;
    case key_left :
      if (y == 0) return; else y--; break;
    default : return;
  }

  if (!undoing) {
    push_key(gmat->undo_stack, &gmat->utop, key * -1);
    sl->moves++;
    sl->key = key;
  }
  swap(&(gmat->mat[x][y]), &(gmat->mat[gmat->curs_x][gmat->curs_y]));
  gmat->curs_x = x;
  gmat->curs_y = y;
}

void gmatrix_populate(struct gmatrix* gmatrix) {
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
  gmatrix_populate(&gmat);
  return gmat;
}

void print_status_line(struct status_line data) {
  int current_x, current_y;
  getyx(stdscr, current_x, current_y);

  mvprintw(LINES - 1, 1, " moves: %2zu", data.moves);
  mvprintw(LINES - 1, CENTER_X(strlen(data.msg)), "%s", data.msg);

  move(LINES - 1, COLS - 2);
  switch(data.key) {
    case key_up: printw("U"); break;
    case key_down: printw("D"); break;
    case key_left: printw("L"); break;
    case key_right: printw("R"); break;
    default: break;
  }
  mvchgat(LINES - 1, 0, -1, A_REVERSE, 1, NULL);
  
  move(current_x, current_y);
}

void show_menu(Key key, uint16_t *highlight) {
  int x, y;
  const char* menu_items[] = {
    "  Load  ",
    "  Easy  ",
    " Normal ",
    "  Hard  ",
    " Custom ",
    "  Exit  ",
  };
  size_t menu_size = sizeof(menu_items) / sizeof(char*);

  if (key == key_down)
    *highlight = (*highlight + 1) % menu_size;
  else if (key == key_up)
    *highlight = (*highlight == 0) ? menu_size - 1 : *highlight - 1;

  x = CENTER_X(strlen("Welcome to uni-void!"));
  y = CENTER_Y(2 + menu_size);

  attron(A_BOLD );
  mvprintw(y++, x, "WELCOME TO UNI-VOID!");
  attroff(A_BOLD);

  for (size_t i = 0; i < menu_size; i++) {
    x = CENTER_X(strlen(menu_items[i]));
    if (i == *highlight) {
      attron(A_REVERSE | A_BOLD);
      mvprintw(y++, x, "%s", menu_items[i]);    
      attroff(A_REVERSE | A_BOLD);
    } else {
      mvprintw(y++, x, "%s", menu_items[i]);    
    }
  } 
}

int input_custom_difficulty() {
  char difficulty[10];
  erase();
  char* query = "Enter order of square matrix: ";
  mvprintw(LINES / 2, CENTER_X(strlen(query)), "%s", query);
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

void display_usage() {
  const char* help_msg[] = {
    "left-arrow, h, a : moves cursor left",
    "down-arrow, j, s : moves cursor down",
    "up-arrow,   k, w : moves cursor up",
    "right-arrow,l, d : moves cursor left",
    "u                : undo move",
    "r                : redo move",
    "q                : exit from the game",
    "Enter            : choose selected item",
    "?                : shows this window",
    " ",
    "  Press any key to close this window",
  };

  int h = sizeof(help_msg) / sizeof(char*);
  int w = strlen(help_msg[7]);
  refresh();
  WINDOW* usage_win = newwin(h + 2, w + 2, CENTER_Y(h), CENTER_X(w));
  box(usage_win, 0, 0);

  for (int i = 0; i < h; i++) {
    mvwprintw(usage_win,i + 1, 1, "%s", help_msg[i]);
  }

  wrefresh(usage_win);
  getch();
  werase(usage_win);
  wborder(usage_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(usage_win);
  delwin(usage_win);
}

static inline Mode decode_mode(uint16_t highlight) {
  return (highlight == 0) ? mode_load : highlight + 2; // since game mode start at 3 higlight should be offseted by 2
}

uint16_t choose_mode(struct status_line status) {
  uint16_t highlight = 0;
  Key key = key_invalid;
  print_status_line(status);
  show_menu(key, &highlight);

  while (key != key_exit) {
    key = decode_key(getch());
    switch (key) {
      case key_return :
        if (highlight == mode_exit) goto exit;
        else if (highlight == mode_custom) return input_custom_difficulty();
        else return decode_mode(highlight);
      case key_usage:
        display_usage();
        break;
      case key_up : case key_down : break;
      default : continue;
    } 
    show_menu(key, &highlight);
    refresh();
  }
  exit:
    endwin();
    exit(0);
}

struct status_line status_line(char* msg) {
  return (struct status_line) {
    .moves = 0,
    .key = 0,
    .msg = msg
  };
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  Arena *arena = arena_init(ARENA_128);
  struct game_obj gobj;

  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  cbreak();

  struct status_line status = status_line("press '?' for help");

  gobj.mode = choose_mode(status);
  gobj.gmat = gmatrix_init(arena, gobj.mode);

  bool completed = false;
  Key key;

  status.msg = "sort the matrix!";
  update_matrix_view(gobj.gmat);
  print_status_line(status);

  bool undoing;
  while (key != key_exit) {
    undoing = false;
    key = decode_key(getch());

    if (key == key_invalid || key == key_exit) continue;
    else if(key == key_undo) {
      if ((key = pop_key(gobj.gmat.undo_stack, &gobj.gmat.utop)) == key_invalid) continue;
      push_key(gobj.gmat.redo_stack, &gobj.gmat.rtop, key * -1);
      undoing = true;
    } else if (key == key_redo) {
      if ((key = pop_key(gobj.gmat.redo_stack, &gobj.gmat.rtop)) == key_invalid) continue;
      push_key(gobj.gmat.undo_stack, &gobj.gmat.utop, key * -1);
      undoing = true;
    } else if(key == key_usage) {
      display_usage();
    }
    mov_zero(&gobj.gmat, &status, key, undoing);
    completed = update_matrix_view(gobj.gmat);
    if (completed) {
      status.msg = "You Won! press 'q' to quit!";
      key = key_exit;
    }
    print_status_line(status);
    refresh();
  } 

  if (!completed) {
    status.msg = "interrupted. press 'q' to quit";
    print_status_line(status);
    refresh();
  }
  while(getch() != 'q');

  endwin();
  arena_free(arena);
  return 0;
}
