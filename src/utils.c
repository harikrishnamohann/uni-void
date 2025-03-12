#include "../include/uni-void.h"

void swap(int *x, int *y) { *x = *x ^ *y; *y = *x ^ *y; *x = *x ^ *y; }

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

char* input_str(const char* query) {
  char difficulty[50];
  erase();
  mvprintw(LINES / 2, CENTER_X(strlen(query)), "%s", query);
  echo();
  nocbreak();
  curs_set(1);
  refresh();
  getstr(difficulty);
  noecho();
  cbreak();
  curs_set(0);
  return strdup(difficulty);
}

void make_radomized_array(int* arr, size_t size) {
  uint32_t pos;
  arr[0] = 0;
  for (size_t i = 1; i < size; i++) {
    arr[i] = i;
    pos = rand() % i;
    swap(&arr[pos], &arr[i]);
  }
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

void update_moves(struct game_state* gs) {
  if (gs->count_ctrl == count_up) {
    gs->moves++;
  } else if (gs->count_ctrl == count_down){
    gs->moves--;
  }
}

