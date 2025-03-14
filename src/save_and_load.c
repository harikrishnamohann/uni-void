// this file contains functions to serialize and deserialize
// our struct game_state type.
#include "../include/uni-void.h"

#define STATE_FILE "game_files/game_state.bin"

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
  fwrite(&gs->moves, sizeof(typeof(gs->moves)), 1, state_file);
  fwrite(&gs->count_ctrl, sizeof(typeof(gs->count_ctrl)), 1, state_file);

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
  fread(&gs.moves, sizeof(typeof(gs.moves)), 1, state_file);
  fread(&gs.count_ctrl, sizeof(typeof(gs.count_ctrl)), 1, state_file);

  fread(gs.undo_stack, sizeof(Key), STK_SIZE, state_file);
  fread(gs.redo_stack, sizeof(Key), STK_SIZE, state_file);

  for (int i = 0; i < gs.order; i++) {
    fread(gs.mat[i], sizeof(int), gs.order, state_file);
  }
  fclose(state_file);
  return gs;
}

