#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/arena.h"

#define STK_SIZE 60
#define CENTER_Y(offset) (((LINES - (offset)) / 2))
#define CENTER_X(offset) (((COLS - (offset)) / 2))
#define STATE_FILE "game_state.bin"

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
  key_force_quit,
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

struct game_state {
  uint16_t order;
  uint16_t curs_x;
  uint16_t curs_y;
  int16_t utop;
  int16_t rtop;
  Key undo_stack[STK_SIZE];
  Key redo_stack[STK_SIZE];
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
bool update_matrix_view(const struct game_state gs) {
  int count = 0, in_place = 0, n_elements = gs.order * gs.order;
  erase();
  
  for (int i = 0; i < gs.order; i++) {
    move(CENTER_Y(gs.order) + i, CENTER_X(gs.order * 4));
    for (int j = 0; j < gs.order; j++) {
      if (i == gs.curs_x && j == gs.curs_y) {
        printw("    ");
      } else {
        if (count == gs.mat[i][j] - 1) {
          in_place++;
          attron(A_BOLD);
          printw("%3d ", gs.mat[i][j]);
          attroff(A_BOLD);
        } else {
          printw("%3d ", gs.mat[i][j]);
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
    case 'Q' : return key_force_quit;
    case KEY_RESIZE : return key_resize;
    case '?' : return key_usage;
  }
  return key_invalid;
}

void mov_zero(struct game_state* gs, struct status_line *sl, Key key, bool undoing) {
  int x = gs->curs_x, y = gs->curs_y;
  switch (key) {
    case key_up :
      if (x == 0) return; else x--; break;
    case key_down :
      if (x == gs->order - 1) return; else x++; break;
    case key_right :
      if (y == gs->order - 1) return; else y++; break;
    case key_left :
      if (y == 0) return; else y--; break;
    default : return;
  }

  if (!undoing) {
    push_key(gs->undo_stack, &gs->utop, key * -1);
    sl->moves++;
    sl->key = key;
  }
  swap(&(gs->mat[x][y]), &(gs->mat[gs->curs_x][gs->curs_y]));
  gs->curs_x = x;
  gs->curs_y = y;
}

// In an even-order puzzle, solvability depends not only on
// the number of inversions but also on the row position of
// the empty tile
bool is_solvable(int* list, int order) {
    int inversions = 0;
    int size = order * order;
    int blank_row = 0; // Row index of blank tile (zero)

    for (int i = 0; i < size; i++) {
        if (list[i] == 0) {
            blank_row = i / order;  // Get row position of the blank (zero)
            continue;
        }
        for (int j = i + 1; j < size; j++) {
            if (list[j] && list[i] > list[j]) {
                inversions++;
            }
        }
    }

    if (order % 2 != 0) {
        return (inversions % 2 == 0);
    }
    return ((inversions + blank_row) % 2 == 1);
}

void make_random_array(int* arr, size_t size) {
  uint32_t pos;
  arr[0] = 0;
  for (size_t i = 1; i < size; i++) {
    arr[i] = i;
    pos = rand() % i;
    swap(&arr[pos], &arr[i]);
  }
}

void populate_gmat(struct game_state* gs) {
  int order = gs->order, rand_arr[order * order], pos = 0;
  do {
    make_random_array(rand_arr, order * order);
  } while (!is_solvable(rand_arr, order));

  for (int i = 0; i < order; i++) {
    for (int j = 0; j < order; j++) {
      if (rand_arr[pos] == 0) {
        gs->curs_x = i;
        gs->curs_y = j;
      }
      gs->mat[i][j] = rand_arr[pos];
      pos++;
    }
  }
}

struct game_state game_state_init(Arena* arena, int order) {
  struct game_state gs = {
    .order = order,
    .curs_x = -1,
    .curs_y = -1,
    .mat = arena_alloc(arena, sizeof(int*) * order),
    .utop = -1,
    .rtop = -1,
  };
  for (int i = 0; i < order; i++) {
    gs.mat[i] = arena_alloc(arena, sizeof(int) * order);
  }
  return gs;
}

void print_status_line(struct status_line data) {
  int current_x, current_y;
  getyx(stdscr, current_x, current_y);

  mvprintw(LINES - 1, 1, " moves: %2zu", data.moves);
  if (COLS > 30)
    mvprintw(LINES - 1, CENTER_X(strlen(data.msg) - 8), "%s", data.msg);

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
    "left-arrow, h, a : moves cursor to left",
    "down-arrow, j, s : moves cursor to down",
    "up-arrow,   k, w : moves cursor to up",
    "right-arrow,l, d : moves cursor to left",
    "u                : undo move",
    "r                : redo move",
    "qq               : save & exit",
    "Q                : exit without saving",
    "Enter            : choose selected item",
    "?                : shows this window",
    " ",
    "  Press any key to close this window",
  };

  int h = sizeof(help_msg) / sizeof(char*);
  int w = strlen(help_msg[0]);
  refresh();
  WINDOW* usage_win = newwin(h + 2, w + 3, CENTER_Y(h), CENTER_X(w));
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
      case key_up : case key_down : case key_resize : break;
      default : continue;
    } 
    erase();
    print_status_line(status);
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

void save_game_state(struct game_state* gs) {
  FILE* state_file = fopen(STATE_FILE, "wb");
  if (state_file == NULL) {
    perror("Failed to write to state file");
    exit(EXIT_FAILURE);
  }

  fwrite(&gs->order, sizeof(typeof(gs->order)), 1, state_file);
  fwrite(&gs->curs_x, sizeof(typeof(gs->curs_x)), 1, state_file);
  fwrite(&gs->curs_y, sizeof(typeof(gs->curs_y)), 1, state_file);
  fwrite(&gs->utop, sizeof(typeof(gs->utop)), 1, state_file);
  fwrite(&gs->rtop, sizeof(typeof(gs->rtop)), 1, state_file);

  fwrite(gs->undo_stack, sizeof(Key), STK_SIZE, state_file);
  fwrite(gs->redo_stack, sizeof(Key), STK_SIZE, state_file);

  for (int i = 0; i < gs->order; i++) {
    fwrite(gs->mat[i], sizeof(int), gs->order, state_file);
  }

  fclose(state_file);
}

static size_t get_file_size(FILE* file) {
  fseek(file, 0, SEEK_END);
  size_t fsize = ftell(file);
  rewind(file);
  return fsize;
}

struct game_state load_game_state(Arena* arena) {
  FILE* state_file = fopen(STATE_FILE, "rb");
  if (state_file == NULL) {
    perror("Failed to load state file");
    endwin();
    exit(EXIT_FAILURE);
  }

  size_t file_size = get_file_size(state_file);
  if (file_size < sizeof(uint16_t) * 3 + sizeof(uint16_t) * 2 + (sizeof(Key) * STK_SIZE * 2)) {
    perror("Couldn't find any saved game state");
    fclose(state_file);
    endwin();
    exit(EXIT_FAILURE);
  }

  uint16_t order;
  fread(&order, sizeof(uint16_t), 1, state_file);
  struct game_state gs = game_state_init(arena, order);

  fread(&gs.curs_x, sizeof(typeof(gs.curs_x)), 1, state_file);
  fread(&gs.curs_y, sizeof(typeof(gs.curs_y)), 1, state_file);
  fread(&gs.utop, sizeof(typeof(gs.utop)), 1, state_file);
  fread(&gs.rtop, sizeof(typeof(gs.rtop)), 1, state_file);

  fread(gs.undo_stack, sizeof(Key), STK_SIZE, state_file);
  fread(gs.redo_stack, sizeof(Key), STK_SIZE, state_file);

  for (int i = 0; i < gs.order; i++) {
    fread(gs.mat[i], sizeof(int), gs.order, state_file);
  }
  fclose(state_file);
  return gs;
}

int main(int argc, char* argv[]) {
  srand(time(NULL));
  Arena *arena = arena_init(ARENA_128);

  struct game_state gs;

  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  cbreak();

  struct status_line status = status_line("press '?' for help");

  Mode mode = choose_mode(status);

  if (mode == mode_load) {
    gs = load_game_state(arena);
  } else {
    gs = game_state_init(arena, mode);
    populate_gmat(&gs);
  }

  bool completed = false;
  Key key;

  status.msg = "sort the matrix!";
  update_matrix_view(gs);
  print_status_line(status);

  uint16_t order;
  bool undoing;
  while (key != key_exit) {
    undoing = false;
    key = decode_key(getch());

    switch (key) {
      case key_invalid : case key_exit : continue;
      case key_undo :
        if ((key = pop_key(gs.undo_stack, &gs.utop)) == key_invalid) continue;
        push_key(gs.redo_stack, &gs.rtop, key * -1);
        undoing = true;
        break;
      case key_redo:
        if ((key = pop_key(gs.redo_stack, &gs.rtop)) == key_invalid) continue;
        push_key(gs.undo_stack, &gs.utop, key * -1);
        undoing = true;
        break;
      case key_usage : display_usage();
        break;
      case key_force_quit : goto exit;
      default : break;
    }
    mov_zero(&gs, &status, key, undoing);
    completed = update_matrix_view(gs);
    if (completed) {
      status.msg = "You Won! press 'q' to quit!";
      key = key_exit;
    }
    print_status_line(status);
    refresh();
  } 

  if (!completed) {
    save_game_state(&gs);
    status.msg = "Game saved. Press 'q' to quit";
    print_status_line(status);
    refresh();
  }
  while(getch() != 'q');

  exit:
    endwin();
    arena_free(arena);
    return 0;
}
