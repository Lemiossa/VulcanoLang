/**
 * lexer.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "token.h"

// Lexer
typedef struct {
	const char *content;
	size_t contentSize;
	size_t pos;
	size_t line;
	size_t column;
} Lexer;

bool lexerValidate(Lexer *l);
Lexer *lexerCreate(const char *content, size_t contentSize);
TokenArray lexerTokenize(Lexer *l);
void lexerDestroy(Lexer *l);
