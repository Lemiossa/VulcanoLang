/**
 * token.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stddef.h>

#include "../util.h"

// Tipo de token
typedef enum {
	TOKEN_NUMBER_INTEGER = 1,
	TOKEN_NUMBER_FLOAT,
	TOKEN_STRING,
	TOKEN_IDENTIFIER,

	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_SLASH,
	TOKEN_PERCENT,

	TOKEN_ASSIGN,

	TOKEN_EQ,
	TOKEN_NEQ,
	TOKEN_LT,
	TOKEN_GT,
	TOKEN_LTE,
	TOKEN_GTE,

	TOKEN_AND,
	TOKEN_OR,
	TOKEN_NOT,

	TOKEN_BIT_AND,
	TOKEN_BIT_OR,
	TOKEN_BIT_XOR,
	TOKEN_BIT_NOT,
	TOKEN_SHIFT_LEFT,
	TOKEN_SHIFT_RIGHT,

	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,

	TOKEN_SEMICOLON,
	TOKEN_COMMA,
	TOKEN_DOT,
	TOKEN_COLON,

	TOKEN_QUESTION,
	TOKEN_ARROW,

	// Keywords
	TOKEN_KEYWORD_INT,
	TOKEN_KEYWORD_BOOLEAN,
	TOKEN_KEYWORD_STRING,
	TOKEN_KEYWORD_NULL,

	TOKEN_KEYWORD_TRUE,
	TOKEN_KEYWORD_FALSE,

	TOKEN_KEYWORD_IF,
	TOKEN_KEYWORD_ELSE,

    TOKEN_KEYWORD_WHILE,

	TOKEN_KEYWORD_VAR,
	TOKEN_KEYWORD_FN,
	TOKEN_KEYWORD_RETURN
} TokenType;

// Token
typedef struct {
	TokenType type;
	const char *content;
	const char *start;
	size_t length;
	size_t line;
	size_t column;
} Token;

// TokenArray
typedef struct {
	Token *data;
	size_t count;
	size_t capacity;
} TokenArray;

void tokenInit(TokenArray *arr);
void tokenPush(TokenArray *arr, Token t);
void tokenDestroy(TokenArray *arr);
void tokenDump(TokenArray *arr);
int tokenLogger(LogLevel level, Token t, const char *format, ...);
