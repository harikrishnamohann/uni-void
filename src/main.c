#include "../include/uni-void.h"

struct game_state game_state_init(Arena* arena, int order) {
  struct game_state gs = {
    .order = order,
    .mode = order - MODE_OFFSET,
    .curs_x = -1,
    .curs_y = -1,
    .moves = 0,
    .count_ctrl = count_stop,
    .mat = arena_alloc(arena, sizeof(int*) * order),
    .utop = -1,
    .rtop = -1,
  };
  for (int i = 0; i < order; i++) {
    gs.mat[i] = arena_alloc(arena, sizeof(int) * order);
  }
  return gs;
}

struct status_line status_line_init(char* msg) {
  return (struct status_line) {
    .moves = 0,
    .key = 0,
    .msg = msg
  };
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

void populate_mat(struct game_state* gs) {
  int order = gs->order, rand_arr[order * order], pos = 0;
  do {
    make_radomized_array(rand_arr, order * order);
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

// return 1 if elements are sorted.
bool update_matrix_view(const struct game_state* gs) {
  int count = 0, in_place = 0, n_elements = gs->order * gs->order;
  erase();
  
  for (int i = 0; i < gs->order; i++) {
    move(CENTER_Y(gs->order) + i, CENTER_X(gs->order * 4));
    for (int j = 0; j < gs->order; j++) {
      if (i == gs->curs_x && j == gs->curs_y) {
        printw("    ");
      } else {
        if (count == gs->mat[i][j] - 1) {
          in_place++;
          attron(A_BOLD);
          printw("%3d ", gs->mat[i][j]);
          attroff(A_BOLD);
        } else {
          printw("%3d ", gs->mat[i][j]);
        }
        count++;
      }
    }
    printw("\n");
  }
  return (in_place == n_elements - 1);
}

Counter mov_zero(struct game_state* gs, Key key) {
  int x = gs->curs_x, y = gs->curs_y;
  switch (key) {
    case key_up :
      if (x == 0) return count_stop; else x--; break;
    case key_down :
      if (x == gs->order - 1) return count_stop; else x++; break;
    case key_right :
      if (y == gs->order - 1) return count_stop; else y++; break;
    case key_left :
      if (y == 0) return count_stop; else y--; break;
    default : return count_stop;
  }

  swap(&(gs->mat[x][y]), &(gs->mat[gs->curs_x][gs->curs_y]));
  gs->curs_x = x;
  gs->curs_y = y;
  return gs->count_ctrl;
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

uint16_t choose_mode(struct status_line status) {
  uint16_t highlight = 0;
  Key key = key_invalid;
  print_status_line(status);
  show_menu(key, &highlight);

  while (key != key_exit) {
    key = decode_key(getch());
    switch (key) {
      case key_return : return highlight;
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
  endwin();
  exit(0);
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

  struct status_line status = status_line_init("press '?' for help");

  Mode mode = choose_mode(status);

  switch (mode) {
    case mode_load :
      gs = load_game_state(arena);
      break;
    case mode_hard :
      gs = game_state_init(arena, mode + MODE_OFFSET);
      populate_mat(&gs);
      gs.moves = HARD_MODE_MOVE_LIMIT;
      gs.count_ctrl = count_down;
      break;
    case mode_easy :
    case mode_normal :
      gs = game_state_init(arena, mode + MODE_OFFSET);
      populate_mat(&gs);
      gs.count_ctrl = count_up;
      break;
    case mode_custom :
      gs = game_state_init(arena, atoi(input_str("Enter order of matrix: ")));
      populate_mat(&gs);
      gs.count_ctrl = count_up;
      break;
    case mode_exit : goto exit;
  }

  bool completed = false;
  Key key;
  Counter counter;

  status.msg = "sort the matrix!";
  status.moves = gs.moves;

  update_matrix_view(&gs);
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

    counter = mov_zero(&gs, key);
    completed = update_matrix_view(&gs);

    if (counter != count_stop && !undoing ) {
      push_key(gs.undo_stack, &gs.utop, key * -1);
      update_moves(&gs);
      status.moves = gs.moves;
      status.key = key;
      if (gs.mode == mode_hard && gs.moves == 0) {
        status.msg = "Game over! press 'q' to exit";
        print_status_line(status);
        refresh();
        goto wait_and_exit;
      }
    }

    if (completed) {
      display_leaderboards(&gs, input_str("You won, enter your nickname: "));
      goto wait_and_exit;
    }
    print_status_line(status);
    refresh();
  } 

  if (!completed) {
    save_game_state(&gs);
  }

  wait_and_exit:
  status.msg = "Press 'q' to quit";
  print_status_line(status);
  refresh();
  while(getch() != 'q');

  exit:
    endwin();
    arena_free(arena);
    return 0;
}

