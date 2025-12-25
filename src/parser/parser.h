/**
 * parser.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "../lexer/token.h"
#include "ast.h"

// Parser
typedef struct {
	TokenArray tokens;
	size_t pos;
} Parser;

bool parserValidate(Parser *p);
Parser *parserCreate(TokenArray tokens);
AstNode *parserParse(Parser *p);
void parserDestroy(Parser *p);
