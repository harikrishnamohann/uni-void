#pragma once

#include "../lib/strings.c"

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

bool is_whitespace(char ch) { return (ch == ' ') || ch == '\t' || ch =='\r' || ch == '\v'; }
bool is_end_of_line(char ch) { return (ch == '\n' || ch == '\0'); }

/**
 * @brief Determines the TokType of a given character.
 * @param ch The character to decode.
 * @return The corresponding TokType.
 */
TokType decode_ch(char ch) {
  if (is_end_of_line(ch)) return tok_end_record;
  else if (ch == ',') return tok_seperator;
  else if (ch == '"') return tok_quote;
  else return tok_val;
}

/**
 * @brief Parses the next token from the input String.
 * @param lexer Pointer to the String being parsed. The lexer's position is advanced.
 * @return The parsed Token.
 */
Token parse_next_token(String* lexer) {
  if (lexer->length == 0) {
    return (Token) {
    .type = tok_eof,
    .lexeme = (String) { NULL, 0, 0, 0, 0 },
    };
  }

  Token tok;
  bool quoting = false;
  uint32_t span = 0;
  while (lexer->length > 0) {
    while (is_whitespace(*lexer->str)) str_offset(lexer, ONCE);
    tok.type = decode_ch((*lexer->str));
    switch (tok.type) {
      case tok_end_record :
        tok.lexeme = str_owned_slice(lexer, STR_BEGIN, 1);
        goto ret;
      case tok_quote :
        quoting = true;
        str_offset(lexer, ONCE);
        break;
      case tok_seperator :
        tok.lexeme = str_owned_slice(lexer, STR_BEGIN, 1);
        goto ret;
      default :
        span = 0;
        if (quoting) {
          while (lexer->str[span++] != '"' && span < lexer->length);
          span--;
          tok.lexeme = str_owned_slice(lexer, STR_BEGIN, span);
          str_offset(lexer, ONCE);
          quoting = false;
        } else {
          while (tok.type != tok_seperator && tok.type != tok_end_record && span < lexer->length) {
            tok.type = decode_ch(lexer->str[span]);
            span++;
          }
          tok.lexeme = str_owned_slice(lexer, STR_BEGIN, span - 1);
          tok.type = tok_val;
        }
        goto ret;
    }
  }    
  ret:
    return tok;
}

/**
 * @brief Initializes a record by counting cells in the next line.
 * @param lexer Pointer to the String being parsed. The lexer's position is advanced.
 * @param dest Pointer to a Token pointer where the allocated record will be stored.
 * @return The number of cells in the record.
 */
uint32_t record_init(String* lexer, Token** dest) {
  TokType tok;
  int32_t cells = 0;
  while ((tok = decode_ch(*lexer->str)) != tok_end_record) {
    if (tok == tok_seperator) cells++;
    str_offset(lexer, ONCE);
  }
  cells++;
  str_offset(lexer, ONCE);

  *dest = malloc(sizeof(Token) * cells);
  if (*dest == NULL) {
    debug_raise_err(MALLOC_FAILURE, "failed to create record");
  }
  return cells;
}

/**
 * @brief Parses the next record from the lexer into the provided Token array.
 * @param lexer Pointer to the String being parsed. The lexer's position is advanced.
 * @param record Array of Tokens to store the parsed record.
 * @param record_len The expected number of tokens in the record.
 * @return PROCEED on success, HALT on EOF or incorrect record length.
 */
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
  return PROCEED;
}
