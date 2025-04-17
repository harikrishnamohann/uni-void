/*
I used a CSV file to track the leaderboard, mainly because it's more fun
than serializing it. You can easily open and edit the file, cheating
your way to the top. But that’s exactly the point!
The motive behind this is to remind you how meaningless results are
when they can be manipulated so easily. What matters is the
experience of playing, improving, and having fun, rather than just
seeing your name at the top of a leaderboard.

This file contains my implementation of a csv parser.
It is not proper csv, I added a "#" token to indicate comments
just like "//" in c.

here is the structure of csv file:
  order, moves, player name, timestamp
*/
#pragma once 

#include "../lib/uni-void.c"
#include "../lib/arena.c"
#include "csv_parser.c"
#include "utils.c"

// maximum number of lines when printing leaderboard
#define LEADERBOARD_ENTRIES 8

static Arena* csv_arena;

// csv structure
 struct leaderboard_record {
  uint16_t order;
  uint16_t moves;
  char *player_name;
  time_t time;
};

// // initialize leaderboard type
static struct leaderboard_record leader_board_init(uint16_t order, uint16_t moves, char* name, time_t time) {
  return (struct leaderboard_record) {
    .order = order,
    .moves = moves,
    .player_name = name,
    .time = time,
  };
}

// Returns null terminated c string. you have to use free() on returned pointer.
char*  str_to_cstring(const String* s) {
  char* cstring = arena_alloc(csv_arena, (sizeof(char) * s->length) + 1);
  if (cstring == NULL) {
    debug_raise_err(MALLOC_FAILURE, NULL);
  } 
  for (size_t i = 0; i < s->length; i++) cstring[i] = s->str[i];
  cstring[s->length] = '\0';
  return cstring;
}

bool order_dec(uint16_t a, uint16_t b) { return a < b; }
bool order_asc(uint16_t a, uint16_t b) { return a > b; }

// parses next valid record. skips empty-lines and csv header.
// a record means a full row of in the csv file.
static struct leaderboard_record parse_next_leaderboard_entry(String* lexer, Token* tokens, int32_t record_len) {
  if (parse_next_record(lexer, tokens, record_len) == HALT) {
    return (struct leaderboard_record) { 0, 0, NULL, 0 };
  }
  return leader_board_init(
           str_to_int64(tokens[0].lexeme),
           str_to_int64(tokens[1].lexeme),
           str_to_cstring(&tokens[2].lexeme),
           str_to_int64(tokens[3].lexeme)
         );
}

// parse the csv file into memory.
// parses all entries if order is 0. otherwise only parse records with given order.
// this way, a single file can be used to store records of all game modes.
// returns number of records read from file.
static uint32_t load_leaderboard(struct leaderboard_record *records, uint16_t order) {
  FILE* fp = fopen(LEADERBOARD_FILE, "r");
  if (fp == NULL) {
    return 0;
  }

  String file = file_to_str(LEADERBOARD_FILE);
  Token* tokens;
  int record_len = record_init(&file, &tokens);

  struct leaderboard_record tmp_record = parse_next_leaderboard_entry(&file, tokens, record_len);
  uint16_t j = 0;
  if (order == 0) { // parse all records.
    while (tmp_record.order != 0) {
      records[j++] = tmp_record;
      tmp_record = parse_next_leaderboard_entry(&file, tokens, record_len);
    }
  } else {
    while (j < LEADERBOARD_ENTRIES && tmp_record.order != 0) {
      if (tmp_record.order == order) { // add records with specific order
        records[j++] = tmp_record;
      }
      tmp_record = parse_next_leaderboard_entry(&file, tokens, record_len);
    }
  }
  fclose(fp);
  str_free(&file);
  free(tokens);
  return j;
}

// sort the records based on moves. ordering can be specified by passing the
// order_dec or order_asc functions as parameter. implements insertion sort since
// total number of records will be very small.
static void sort_records_using_moves(struct leaderboard_record* records, size_t size, bool (*order_by)(uint16_t, uint16_t)) {
  for (int i = 0; i < size; i++) {
    struct leaderboard_record key = records[i];
    int j = i - 1;
    while (j >= 0 && order_by(records[j].moves , key.moves)) {
      records[j + 1] = records[j];
      j--;
    }
    records[j + 1] = key;
  }
}

// saves new_record at the top of the file and deletes last entry if
// number of records exceeds max_records.
static void save_record(const struct leaderboard_record* new_record) {
  uint16_t max_records = LEADERBOARD_ENTRIES * 4;
  struct leaderboard_record *all_records = arena_alloc(csv_arena, sizeof(struct leaderboard_record) * max_records);  
  all_records[0] = *new_record; // first record is new_record.
  size_t read_entries = load_leaderboard(all_records + 1, 0); // parses the file
  if (read_entries < max_records - 1) {
    read_entries++; // makes room for new_record only if total entries in our file doesn't exceeds max_records limit
  }

  FILE* fp = fopen(LEADERBOARD_FILE, "w");
  if (fp == NULL) {
    perror("Failed to open leaderboard file for writing");
    return;
  }

  // saves the records with a new_record entry.
  fprintf(fp, "\"Order\",\"Moves\",\"Player\",\"Time\"\n");
  for (size_t i = 0; i < read_entries; i++) {
    fprintf(fp, "%d,%d,\"%s\",%lu\n", all_records[i].order, all_records[i].moves, all_records[i].player_name, all_records[i].time);
  }
  fclose(fp);
}

// saves new entry to csv file and display leaderboard.
void display_leaderboards(const struct game_state* gs, char* name) {
  csv_arena = arena_init(ARENA_1024); // initializes new arena context

  char* player_name = strdup(name);
  struct leaderboard_record new_record = leader_board_init(gs->order, gs->moves, player_name, time(NULL));
  save_record(&new_record);

  struct leaderboard_record *records = arena_alloc(csv_arena, sizeof(struct leaderboard_record) * LEADERBOARD_ENTRIES);

  size_t read_records_count = load_leaderboard(records, gs->order);

  if (read_records_count) {
    int rank = 0;
    sort_records_using_moves(records, read_records_count, (gs->mode == mode_hard) ? order_dec : order_asc);
    for (int i = 0; i < read_records_count; i++) {
      if (strcmp(records[i].player_name, player_name) == 0 && records[i].moves == new_record.moves) {
        rank = i;
        break;
      }
    }

    char *mode;
    switch (gs->mode) {
      case mode_easy: mode = "Easy"; break;
      case mode_normal: mode = "Normal"; break;
      case mode_hard: mode = "Hard"; break;
      default: mode = "Custom"; break; 
    }

    erase();
    char* template = "#  Moves   Player          Time     ";
    int y = CENTER_Y(read_records_count + 3);
    int x = CENTER_X(strlen("6. 35	   hariii          12-03-2025 21:36:12"));

    attron(A_BOLD);
    mvprintw(y++, x, "Congrats %s! you are #%d", player_name, rank + 1);
    attroff(A_BOLD);
    y++;
    attron(A_DIM);
    mvprintw(y++, x, "Leaderboard (%s){%dx%d}", mode, new_record.order, new_record.order);
    attroff(A_DIM);
    mvchgat(y - 1, x, strlen("Leaderboard"), A_UNDERLINE, 0, NULL);
    mvprintw(y++, x, "%s", template);

    struct tm *ctime;
    for (int i = 0; i < read_records_count; i++) {
      ctime = localtime(&records[i].time);
      mvprintw(y, x, "%d. %d", i + 1, records[i].moves);
      mvprintw(y, x + 11, "%s", records[i].player_name);
      mvprintw(y, x + 26,
               " %02d-%02d-%4d %02d:%02d:%02d",
               ctime->tm_mday,
               ctime->tm_mon + 1,
               ctime->tm_year + 1900,
               ctime->tm_hour,
               ctime->tm_min,
               ctime->tm_sec
             );
      if (i == rank) {
        mvchgat(y, x, -1, A_BOLD, 0, NULL);
      }
      y++;
    }
  }

  free(player_name);
  arena_free(csv_arena);
}
