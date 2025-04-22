#pragma once

#include "../lib/strings.c"
#include "../lib/err.c"

#define ONCE 1

typedef enum {
  tok_eof,
  tok_seperator,
  tok_end_record,
  tok_quote,
  tok_val,
} TokType;

typedef struct {
  TokType type;
  String lexeme;
} Token;

#define TOK_EOF (Token) {tok_eof, STR_EMPTY}

bool is_whitespace(char ch) { return (ch == ' ') || ch == '\t' || ch =='\r' || ch == '\v'; }
bool is_end_of_line(char ch) { return (ch == '\n' || ch == '\0'); }

TokType decode_ch(char ch) {
  if (is_end_of_line(ch)) return tok_end_record;
  else if (ch == ',') return tok_seperator;
  else if (ch == '"') return tok_quote;
  else return tok_val;
}

Token parse_next_token(String* lexer) {
  if (lexer->length == 0) return TOK_EOF;

  Token tok;
  bool quoting = false;
  uint32_t span = 0;
  while (lexer->length > 0) {
    while (is_whitespace(*lexer->str)) {
      err_expect(str_err, str_offset(lexer, ONCE));
    }
    tok.type = decode_ch((*lexer->str));
    switch (tok.type) {
      case tok_end_record :
        tok.lexeme = err_expect(str_err, str_slice_head(lexer, STR_BEGIN, 1));
        goto ret;
      case tok_quote :
        quoting = true;
        err_expect(str_err, str_offset(lexer, ONCE));
        break;
      case tok_seperator :
        tok.lexeme = err_expect(str_err, str_slice_head(lexer, STR_BEGIN, 1));
        goto ret;
      default :
        span = 0;
        if (quoting) {
          while (lexer->str[span++] != '"' && span < lexer->length);
          span--;
        tok.lexeme = err_expect(str_err, str_slice_head(lexer, STR_BEGIN, span));
          str_offset(lexer, ONCE);
          quoting = false;
        } else {
          while (tok.type != tok_seperator && tok.type != tok_end_record && span < lexer->length) {
            tok.type = decode_ch(lexer->str[span]);
            span++;
          }
          tok.lexeme = err_expect(str_err, str_slice_head(lexer, STR_BEGIN, span - 1));
          tok.type = tok_val;
        }
        goto ret;
    }
  }    
  ret:
    return tok;
}

uint32_t record_init(String* lexer, Token** dest) {
  TokType tok;
  int32_t cells = 0;
  while ((tok = decode_ch(*lexer->str)) != tok_end_record) {
    if (tok == tok_seperator) cells++;
    err_expect(str_err, str_offset(lexer, ONCE));
  }
  cells++;
  err_expect(str_err, str_offset(lexer, ONCE));

  *dest = malloc(sizeof(Token) * cells);
  if (*dest == NULL) {
    return HALT;
  }
  return cells;
}

int parse_next_record(String* lexer, Token* record, uint32_t record_len) {
  Token tok = parse_next_token(lexer);
  if (tok.type == tok_eof) return HALT;
  uint32_t tok_count = 0;
  while (tok.type != tok_end_record) {
    if (tok.type == tok_seperator) {
      tok = parse_next_token(lexer);
      continue;
    }
    record[tok_count] = tok;
    tok_count++;
    tok = parse_next_token(lexer);
  }

  if (tok_count != record_len) {
    return HALT;
  }
  return OK;
}
