#include "../include/uni-void.h"
#include <threads.h>

#define LEADERBOARD_FILE "essentials/highscores.csv"
#define LEADERBOARD_ENTRIES 8

static Arena* csv_arena;

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
  tok_eof,
} toktype;

struct token {
  toktype type;
  char* lexeme;
};

static inline bool is_whitespace(char ch) { return (ch == ' ') || ch == '\t' || ch =='\r'; }
static inline bool is_digit(char ch) { return (ch >= '0' && ch <= '9'); }
static inline bool is_eol(char ch) { return (ch == '\n' || ch == '\0'); }

struct token token_init(Arena* arena, toktype type, char* lexeme) {
  struct token tok;
  tok.type = type;
  tok.lexeme = arena_alloc(arena, strlen(lexeme) + 1);
  strcpy(tok.lexeme, lexeme);
  return tok;
}

toktype get_toktype(char ch) {
  if (is_digit(ch)) return tok_num;
  else if (is_eol(ch)) return tok_end_record;
  else if (ch == '"') return tok_quote;
  else if (ch == ',') return tok_seperator;
  return tok_invalid;
}

struct token tok_next_token(FILE* fp) {
  static bool quoting = false;

  char buf[100];
  size_t i;
  toktype token_type;

  while ((*buf = fgetc(fp)) != EOF) {
    i = 0;
    while (is_whitespace(*buf)) *buf = fgetc(fp); // skipping white spaces    
    token_type = get_toktype(*buf);
    buf[i + 1] = '\0';
    switch (token_type) {
      case tok_end_record : goto ret;
      case tok_seperator : break;
      case tok_quote : quoting = true; break;
      default :
        if (quoting) {
          while (get_toktype(buf[i]) != tok_quote) buf[++i] = fgetc(fp);
          quoting = false;
        } else {
          while (get_toktype(buf[i]) == token_type) buf[++i] = fgetc(fp);
          fseek(fp, -1, SEEK_CUR);
        }
        buf[i] = '\0';
        goto ret;
    }
  }  
  ret:
    return token_init(csv_arena, (*buf == EOF) ? tok_eof : token_type, buf);
}

static inline void skip_csv_header(FILE* fp) { while (tok_next_token(fp).type != tok_end_record); }

struct leaderboard_record highscore_init(uint16_t order, uint16_t moves, char* name, time_t time) {
  return (struct leaderboard_record) {
    .order = order,
    .moves = moves,
    .player_name = name,
    .time = time,
  };
}

struct leaderboard_record parse_next_record(FILE* fp) {
  int8_t top = -1, total_fields = 5; 
  struct token tok_record[total_fields];
  struct leaderboard_record record = {0, 0, NULL, 0};

  while (++top < total_fields) {
    tok_record[top] = tok_next_token(fp);
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

uint32_t load_leader_board(struct leaderboard_record *records, uint16_t order) {
  FILE* fp = fopen(LEADERBOARD_FILE, "r");
  if (fp == NULL) {
    perror("Failed to open file\n");
    exit(EXIT_FAILURE);
  }

  skip_csv_header(fp);
  struct leaderboard_record tmp_record = parse_next_record(fp);
  uint16_t j = 0;
  if (order == 0) {
    while (tmp_record.order != 0) {
      records[j++] = tmp_record;
      tmp_record = parse_next_record(fp);
    }
  } else {
    while (j < LEADERBOARD_ENTRIES && tmp_record.order != 0) {
      if (tmp_record.order == order) {
        records[j++] = tmp_record;
      }
      tmp_record = parse_next_record(fp);
    }
  }
  fclose(fp);
  return j;
}

static inline bool order_dec(uint16_t a, uint16_t b) { return a < b; }
static inline bool order_asc(uint16_t a, uint16_t b) { return a > b; }

void sort_records_using_moves(struct leaderboard_record* records, size_t size, bool (*order_by)(uint16_t, uint16_t)) {
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

void save_record(const struct leaderboard_record* new_record) {
  struct leaderboard_record *all_records = arena_alloc(csv_arena, sizeof(struct leaderboard_record) * LEADERBOARD_ENTRIES * 4);  
  all_records[0] = *new_record;
  size_t read_entries = load_leader_board(all_records + 1, 0) + 1;

  FILE* fp = fopen(LEADERBOARD_FILE, "w");
  if (fp == NULL) {
    perror("Failed to open leaderboard file for writing");
    return;
  }

  // Writing CSV header
  fprintf(fp, "Order,Moves,Player,Time\n");

  // Writing leaderboard records
  for (size_t i = 0; i < read_entries; i++) {
    fprintf(fp, "%d,%d,%s,%lu\n", all_records[i].order, all_records[i].moves, all_records[i].player_name, all_records[i].time);
  }

  fclose(fp);
}

void display_highscores(const struct game_state* gs, char* name) {
  csv_arena = arena_init(ARENA_1024);

  char* player_name = strdup(name);
  struct leaderboard_record new_record = highscore_init(gs->order, gs->moves, player_name, time(NULL));
  save_record(&new_record);

  struct leaderboard_record *records = arena_alloc(csv_arena, sizeof(struct leaderboard_record) * LEADERBOARD_ENTRIES);

  size_t read_records = load_leader_board(records, gs->order);

  if (read_records) {
    void* ordering;
    if (gs->mode == mode_hard) {
      ordering = order_dec;
    } else {
      ordering = order_asc;
    }
    sort_records_using_moves(records, read_records, ordering);

    for (int i = 0; i < read_records; i++) {
      printf("%d, %d, %s, %lu\n", records[i].order, records[i].moves, records[i].player_name, records[i].time);
    }
  }

  free(player_name);
  arena_free(csv_arena);
}

int main() {
  struct game_state gs = {
    .mode = mode_normal,
    .order = 4,
    .moves = 33
  };
  display_highscores(&gs, "pii");
}
