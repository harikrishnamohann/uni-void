/*
Hi, myself Harii.
I wrote this small game to kill my free time and to learn ncurses
library.  

This is a sliding tile puzzle game where the player has to sort the
numbers in a matrix in ascending order. The player can move the
void-tile around to sort the matrix.

The code base is split into separate files with some self-explanatory
names.
  - main.c: contains main game logic
  - keymaps.c: contains a mapping of keys used in the game
  - utils.c : some utility functions as well as some core logic functions
  - leaderboard.c : functions for displaying and managing game leaderboard.
  - save_and_load.c : defines functions for serializing and deserializing
                      current game state.

Apart from the game logic, I used an arena-allocator for
managing memory. Leaderboard has separate arena context defined
in leaderboard.c rather than the arena used for game.
Arena definition can be found in lib/arena.c
*/

// This header file contain all structs and macros used in game.
#include "../include/uni-void.h"
#include <ncurses.h>

// initialize game_state data type.
struct game_state game_state_init(Arena* arena, int order) {
  struct game_state gs = {
    .order = order,
    .mode = order - MODE_OFFSET,
    // since easy mode represents order 3 and index of easy mode
    // is 1, easy mode = order - 2 (2 is the MODE_OFFSET).
    .curs_x = -1,
    .curs_y = -1,
    .moves = 0,
    .count_ctrl = count_stop,
    .mat = arena_alloc(arena, sizeof(int*) * order),
    // these are stack pointers for undo(utop) and redo(rtop) stacks.
    .utop = -1,
    .rtop = -1,
  };
  for (int i = 0; i < order; i++) {
    gs.mat[i] = arena_alloc(arena, sizeof(int) * order);
  }
  return gs;
}

// initialize status line with given message.
struct status_line status_line_init(char* msg) {
  return (struct status_line) {
    .moves = 0,
    .key = 0,
    .msg = msg
  };
}

// The below function checks solvability of our puzzle.
// In an even-order puzzle, solvability depends not only on
// the number of inversions but also on the row position of
// the empty tile
// This chat-gpt code. It works as expected.
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

// This function populate our game matrix with a solvable combination
// of natural numbers sorted in random order.
void populate_mat(struct game_state* gs) {
  int order = gs->order, rand_arr[order * order], pos = 0;
  do {
    // below creates an array of whole numbers upto given size limit
    // and arrange them randomly. Defined in utils.c
    make_radomized_array(rand_arr, order * order);
  } while (!is_solvable(rand_arr, order));

  for (int i = 0; i < order; i++) {
    for (int j = 0; j < order; j++) {
      if (rand_arr[pos] == 0) { // 0 is our void-tile.
        gs->curs_x = i; // saves position of zero to start cursor from there.
        gs->curs_y = j;
      }
      gs->mat[i][j] = rand_arr[pos];
      pos++;
    }
  }
}

// prints the matrix using ncurses.
// return true if all elements are sorted. ie if the game is completd.
bool update_matrix_view(const struct game_state* gs) {
  int count = 0, in_place = 0, n_elements = gs->order * gs->order;
  erase();
  
  for (int i = 0; i < gs->order; i++) {
    move(CENTER_Y(gs->order) + i, CENTER_X(gs->order * 4));
    for (int j = 0; j < gs->order; j++) {
      if (i == gs->curs_x && j == gs->curs_y) {
        printw("    "); // void-tile in place of 0
      } else {
        if (count == gs->mat[i][j] - 1) {
          in_place++;
          attron(A_BOLD); // highlighting numbers that are in correct position
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
  return (in_place == n_elements - 1); // checking if the matrix is sorted.
}

// this function updates position of our 0 (void-tile) based on key input.
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

// displays the bottom status line.
void update_status_line(struct status_line data) {
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

// driver function for menu.
void show_menu(Key key, uint16_t *highlight) {
  int x, y;
  const char* menu_items[] = {
    "Load",
    "Easy",
    "Normal",
    "Hard",
    "Custom",
    "Exit",
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

  attron(A_ITALIC | A_DIM);
  for (size_t i = 0; i < menu_size; i++) {
    x = CENTER_X(strlen(menu_items[i]));
    if (i == *highlight) {
      attroff(A_DIM);
      attron(A_BOLD);
      mvprintw(y, x - 2, "> ");
      mvprintw(y++, x, "%s <", menu_items[i]);    
      attroff( A_BOLD);
      attron(A_DIM);
    } else {
      mvprintw(y++, x, "%s", menu_items[i]);    
    }
  } 
  attroff(A_ITALIC | A_DIM);
}

// displays menu.
// returns index of mode selected from menu.
uint16_t choose_mode(struct status_line status) {
  uint16_t highlight = 0;
  Key key = key_invalid;
  update_status_line(status);
  show_menu(key, &highlight);

  while (key != key_exit) {
    key = decode_key(getch());
    switch (key) {
      case key_enter : return highlight;
      case key_usage: display_usage(); break;
      case key_up : case key_down : case key_resize : break;
      default : continue;
    } 
    erase();
    update_status_line(status);
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

  initscr(); // initilize ncurses 
  noecho(); // Disables automatic echoing of typed characters
  curs_set(0); // set cursor visibility
  keypad(stdscr, TRUE); // enables special keys
  cbreak(); // allows input to be read without pressing return key

  struct status_line status_line = status_line_init("press '?' for help");

  Mode mode = choose_mode(status_line); // loads menu
  int order = mode + MODE_OFFSET; // here MODE_OFFSET is used to calculate order

  switch (mode) {
    case mode_load : // laod previously saved game.
      gs = load_game_state(arena);
      break;
    case mode_hard : // hord mode has limited moves
      gs = game_state_init(arena, order); 
      populate_mat(&gs);
      gs.moves = HARD_MODE_MOVE_LIMIT; // move limit is defined in univoid.h
      gs.count_ctrl = count_down;
      break;
    case mode_easy :
    case mode_normal :
      gs = game_state_init(arena, order);
      populate_mat(&gs);
      gs.count_ctrl = count_up;
      break;
    case mode_custom : // user can specify order of square matrix.
      order = atoi(input_str("Input an order from 2 to 16: "));
      if (order > 16 || order < 2) {
        status_line.msg = "invalid order! press 'q' to quit";
        goto wait_and_exit;
      }
      gs = game_state_init(arena, order);
      populate_mat(&gs);
      gs.count_ctrl = count_up;
      break;
    case mode_exit : goto exit;
  }

  bool completed = false, undoing; // flags to indicate game completion and undo-redo operation
  Key key; // store keyboard input keys
  Counter counter; // to indicate wheather or not to update move count.
  // when void-tile is in any edge, we don't want to count
  // moves that tries to go off that edge.

  status_line.msg = "sort the matrix!";
  status_line.moves = gs.moves;

  update_matrix_view(&gs);
  update_status_line(status_line);

  while (key != key_exit) {
    undoing = false;
    key = decode_key(getch());

    switch (key) {
      case key_invalid : case key_exit : continue;
      case key_undo : // pop from undo stack, push inverse of that key to redo stack
        if ((key = pop_key(gs.undo_stack, &gs.utop)) == key_invalid) continue;
        push_key(gs.redo_stack, &gs.rtop, key * -1);
        undoing = true; // indicator to stop counting moves.
        break;
      case key_redo: // pop redo-stack, push inverse of that key to undo stack
        if ((key = pop_key(gs.redo_stack, &gs.rtop)) == key_invalid) continue;
        push_key(gs.undo_stack, &gs.utop, key * -1);
        undoing = true;
        break;
      case key_usage : display_usage();
        break;
      case key_force_quit : goto exit;
      default : break;
    }

    // updating matrix view
    counter = mov_zero(&gs, key);
    completed = update_matrix_view(&gs);

    if (counter != count_stop && !undoing ) {
      push_key(gs.undo_stack, &gs.utop, key * -1); // pushing inverse key to undo stack
      update_moves(&gs);
      status_line.moves = gs.moves;
      status_line.key = key;
      if (gs.mode == mode_hard && gs.moves == 0) { // hard_mode ends when counter reach 0
        status_line.msg = "Game over! press 'q' to exit";
        goto wait_and_exit;
      }
    }

    if (completed) {
      display_leaderboards(&gs, input_str("You won, enter your nickname: "));
      status_line.msg = "Press 'q' to quit...";
      goto wait_and_exit;
    }
    update_status_line(status_line);
    refresh();
  } 

  if (!completed) {
    save_game_state(&gs);
    status_line.msg = "Game saved. Press 'q' to quit";
  }

  wait_and_exit: // label to printing some message onto status line before exiting.
  update_status_line(status_line);
  refresh();
  while(getch() != 'q');

  exit: // directly end the program
    endwin();
    arena_free(arena);
    return 0;
}

