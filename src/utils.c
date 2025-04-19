#pragma once

#include "../lib/uni-void.c"
#include "../lib/strings.c"

// swaps x and y using xor.
void swap(int *x, int *y) { *x = *x ^ *y; *y = *x ^ *y; *x = *x ^ *y; }

// pushes the key into the undo or redo stack.
void push_key(Key stk[], int16_t *top, Key key) {
  if (*top == STK_SIZE - 1) { // stack full
    for (int i = 1; i < STK_SIZE; i++) {
      stk[i - 1] = stk[i];
    }
    (*top)--;
  }  
  stk[++(*top)] = key;
}

// pop key from stack
Key pop_key(Key stk[], int16_t *top) {
  if (*top == -1) {
    return key_invalid;
  }
  return stk[(*top)--];
}

// query the user and return answer as a char*
// memory should be freed
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

// creates an array of whole numbers up to specified size and arranges them in random order.
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

String file_to_str(char* filename) {
  String file = str_declare(STR_DYNAMIC);
  FILE* fp = fopen(filename, "r");
  if (fp == NULL) {
    return file;
  }

  int ch;

  while ((ch = fgetc(fp)) != EOF) str_insert(&file, STR_END, ch);
  fclose(fp);
  file.mutable = false;
  return file;
}

void str_to_file(char* filename, String content) {
  FILE* fp = fopen(filename, "w");
  if (fp == NULL) {
    return;
  }

  fwrite(content.str, sizeof(char), content.length, fp);

  fclose(fp);
}


