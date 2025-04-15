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

#include "../include/uni-void.h"

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

typedef enum {
  tok_invalid,
  tok_num,
  tok_str,
  tok_seperator,
  tok_end_record,
  tok_quote,
  tok_comment,
  tok_eof,
} toktype;

struct token {
  toktype type;
  char* lexeme;
};

static inline bool is_whitespace(char ch) { return (ch == ' ') || ch == '\t' || ch =='\r'; }
static inline bool is_digit(char ch) { return (ch >= '0' && ch <= '9'); }
static inline bool is_eol(char ch) { return (ch == '\n' || ch == '\0'); }
// conditions for ordering
static inline bool order_dec(uint16_t a, uint16_t b) { return a < b; }
static inline bool order_asc(uint16_t a, uint16_t b) { return a > b; }

// token initializer
static struct token token_init(Arena* arena, toktype type, char* lexeme) {
  struct token tok;
  tok.type = type;
  tok.lexeme = arena_alloc(arena, strlen(lexeme) + 1);
  strcpy(tok.lexeme, lexeme);
  return tok;
}

// decodes token type from read character.
static toktype decode_toktype(char ch) {
  if (is_digit(ch)) return tok_num;
  else if (is_eol(ch)) return tok_end_record;
  else if (ch == '"') return tok_quote;
  else if (ch == ',') return tok_seperator;
  else if (ch == '#') return tok_comment;
  return tok_invalid;
}

// returns a function and traverse file pointer.
static struct token tok_next_token(FILE* fp) {
  static bool quoting = false; // used to track whether double-quotes are on or not

  char buf[100]; // buffer for storing current token
  size_t i;
  toktype token_type;

  while ((*buf = fgetc(fp)) != EOF) {
    i = 0;
    while (is_whitespace(*buf)) *buf = fgetc(fp); // skipping white spaces    
    token_type = decode_toktype(*buf); // fetching token type
    buf[i + 1] = '\0';
    switch (token_type) {
      case tok_end_record : goto ret;
      case tok_seperator : break; // skips separators
      case tok_quote : quoting = true; break; // string type is coming.
      case tok_comment : while (!is_eol(*buf)) *buf = fgetc(fp); break; // skips comments
      default :
        if (quoting) { // traverse till next quote and returns tok_str type
          while (decode_toktype(buf[i]) != tok_quote) buf[++i] = fgetc(fp);
          token_type = tok_str;
          quoting = false;
        } else if (token_type == tok_num) { // this should be numbers
          while (decode_toktype(buf[i]) == token_type) buf[++i] = fgetc(fp); // traverse till a non-number character is found.
          fseek(fp, -1, SEEK_CUR); // rewinds file pointer back by one position
        }
        buf[i] = '\0';
        goto ret;
    }
  }  
  ret:
    return token_init(csv_arena, (*buf == EOF) ? tok_eof : token_type, buf);
}

// initialize leaderboard type
static struct leaderboard_record highscore_init(uint16_t order, uint16_t moves, char* name, time_t time) {
  return (struct leaderboard_record) {
    .order = order,
    .moves = moves,
    .player_name = name,
    .time = time,
  };
}

// parses next valid record. skips empty-lines and csv header.
// a record means a full row of in the csv file.
static struct leaderboard_record parse_next_record(FILE* fp) {
  int8_t top = 0, total_fields = 5; 
  struct token tok_record[total_fields];
  struct leaderboard_record record = {0, 0, NULL, 0};

  while (top < total_fields) {
    tok_record[top] = tok_next_token(fp);
    // ignore updating top if an invalid token is recieved as first token. 
    if (top == 0 && tok_record[0].type != tok_num && tok_record[0].type != tok_eof) continue; 
    top++;
  }

  if (tok_record[0].type != tok_eof) {
    record = highscore_init(
                            atoi(tok_record[0].lexeme),
                            atoi(tok_record[1].lexeme),
                            tok_record[2].lexeme,
                            atoi(tok_record[3].lexeme)
                          ); 
  }
  return record;
}

// parse the csv file into memory.
// parses all entries if order is 0. otherwise only parse records with given order.
// this way, a single file can be used to store records of all game modes.
// returns number of records read from file.
static uint32_t load_leaderboard(struct leaderboard_record *records, uint16_t order) {
  if (access(LEADERBOARD_FILE, F_OK) != 0) {
    fclose(fopen(LEADERBOARD_FILE, "w"));
  }
  FILE* fp = fopen(LEADERBOARD_FILE, "r");
  if (fp == NULL) {
  }

  struct leaderboard_record tmp_record = parse_next_record(fp);
  uint16_t j = 0;
  if (order == 0) { // parse all records.
    while (tmp_record.order != 0) {
      records[j++] = tmp_record;
      tmp_record = parse_next_record(fp);
    }
  } else {
    while (j < LEADERBOARD_ENTRIES && tmp_record.order != 0) {
      if (tmp_record.order == order) { // add records with specific order
        records[j++] = tmp_record;
      }
      tmp_record = parse_next_record(fp);
    }
  }
  fclose(fp);
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
  fprintf(fp, "# don't edit this file ok XD\n");
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
  struct leaderboard_record new_record = highscore_init(gs->order, gs->moves, player_name, time(NULL));
  save_record(&new_record);

  struct leaderboard_record *records = arena_alloc(csv_arena, sizeof(struct leaderboard_record) * LEADERBOARD_ENTRIES);

  size_t read_records_count = load_leaderboard(records, gs->order);

  if (read_records_count) {
    sort_records_using_moves(records, read_records_count, (gs->mode == mode_hard) ? order_dec : order_asc);
    uint8_t rank;
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
