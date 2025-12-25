/**
 * lexer.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "util.h"

typedef struct {
	const char *name;
	TokenType type;
} Keyword;

Keyword keywords[] = {
    {"int", TOKEN_KEYWORD_INT},
    {"boolean", TOKEN_KEYWORD_BOOLEAN},
    {"string", TOKEN_KEYWORD_STRING},

    {"true", TOKEN_KEYWORD_TRUE},
    {"false", TOKEN_KEYWORD_FALSE},
    {"null", TOKEN_KEYWORD_NULL},

    {"if", TOKEN_KEYWORD_IF},
    {"else", TOKEN_KEYWORD_ELSE},
    {"while", TOKEN_KEYWORD_WHILE},

    {"var", TOKEN_KEYWORD_VAR},
    {"fn", TOKEN_KEYWORD_FN},
    {"return", TOKEN_KEYWORD_RETURN},

    {"and", TOKEN_AND},
    {"or", TOKEN_OR},
    {"not", TOKEN_NOT},

    {NULL, 0} // Para aqui
};

// Valída um lexer
bool lexerValidate(Lexer *l) {
	if (!l) {
		logger(LOG_ERROR, "Lexer error: lexer is NULL\n");
		return false;
	}

	if (!l->content) {
		logger(LOG_ERROR, "Lexer error: lexer content is NULL\n");
		return false;
	}

	if (l->contentSize == 0) {
		logger(LOG_ERROR, "Lexer error: lexer content size is 0\n");
		return false;
	}

	return true;
}

// Retorna true se esta no fim do conteúdo
static bool eof(Lexer *l) {
	if (!lexerValidate(l))
		return true;

	return l->pos >= l->contentSize;
}

// Pula enquanto não tem um caractere
static void skipUntil(Lexer *l, char c) {
	if (!lexerValidate(l))
		return;

	while (!eof(l) && l->content[l->pos] != c) {
		if (l->content[l->pos] == '\n') {
			l->line++;
			l->column = 1;
		} else {
			l->column++;
		}
		l->pos++;
	}
}

// Retorna o caracere atual do content do lexer
static char peek(Lexer *l) {
	if (eof(l))
		return 0;
	return l->content[l->pos];
}

// Incrementa pos e retorna o novo caractere
// static char advance(Lexer* l) {
//  if (eof(l)) return 0;
//  return l->content[++l->pos];
//}

// Retorna o proximo caractere sem incrementar pos
static char next(Lexer *l) {
	if (eof(l))
		return 0;
	if (l->pos + 1 >= l->contentSize)
		return 0;
	return l->content[l->pos + 1];
}

// Cria um lexer
Lexer *lexerCreate(const char *content, size_t contentSize) {
	if (!content) {
		logger(LOG_ERROR, "Lexer error: Content is NULL\n");
		return NULL;
	}

	if (contentSize == 0) {
		logger(LOG_ERROR, "Lexer error: Content size is 0\n");
		return NULL;
	}

	Lexer *l = (Lexer *)malloc(sizeof(Lexer));
	if (!l)
		return NULL;

	l->content = content;
	l->contentSize = contentSize;
	l->pos = 0;
	l->line = 1;
	l->column = 1;

	return l;
}

// Tokeniza um lexer
// Retorna um TokenArray
TokenArray lexerTokenize(Lexer *l) {
	TokenArray tokens = {0};
	if (!lexerValidate(l))
		return tokens;
	tokenInit(&tokens);

	// Parte 1: tudo menos keywords
	while (!eof(l)) {
		char c = peek(l);

		// Quebra de linha
		if (c == '\n') {
			l->line++;
			l->column = 1;
			l->pos++;
			continue;
		}

		// Pular espaço, tab e carriage return
		if (c == ' ' || c == '\t' || c == '\r') {
			l->column++;
			l->pos++;
			continue;
		}

		// Comentarios
		if (c == '#') {
			l->pos++; // Pular #
			skipUntil(l, '\n');

			// Aqui só pode ser \n ou \0
			if (l->content[l->pos] == '\n') {
				l->line++;
				l->column = 1;
				l->pos++;
			} else {
				break;
			}
			continue;
		}

		// Números
		if (isdigit(c)) {
			// Usar strtol para descobrir o final do número
			const char *start = &l->content[l->pos];
			char *integerEnd;

			strtoll(start, &integerEnd, 0);

			// Verificar se é inteiro
			if (integerEnd != start && *integerEnd != '.') {
				Token t;
				t.type = TOKEN_NUMBER_INTEGER;
				t.content = l->content;
				t.start = start;
				t.length = (size_t)(integerEnd - start);
				t.line = l->line;
				t.column = l->column;
				tokenPush(&tokens, t);

				l->pos += t.length;
				l->column += t.length;
				continue;
			}
			// Se chegou aqui provavelmente é float/double
			char *floatingEnd;

			strtod(start, &floatingEnd);

			if (floatingEnd != start) {
				Token t;
				t.type = TOKEN_NUMBER_FLOAT;
				t.content = l->content;
				t.start = start;
				t.length = (size_t)(floatingEnd - start);
				t.line = l->line;
				t.column = l->column;
				tokenPush(&tokens, t);

				l->pos += t.length;
				l->column += t.length;
				continue;
			}
		}

		// Strings
		if (c == '"' || c == '\'') {
			size_t startColumn = l->column;
			l->pos++;
			l->column++;
			const char *start = &l->content[l->pos];

			skipUntil(l, c);

			// Aqui só pode ser '|" ou \0
			if (l->content[l->pos] == c) {
				const char *end = &l->content[l->pos];

				Token t;
				t.type = TOKEN_STRING;
				t.content = l->content;
				t.start = start;
				t.length = (size_t)(end - start);
				t.line = l->line;
				t.column = startColumn;
				tokenPush(&tokens, t);

				l->pos++;
				l->column++;
			} else {
				logger(LOG_ERROR, "Unterminated string at %zu:%zu\n", l->line,
				       l->column);
				break;
			}
			continue;
		}

		// Identificadores
		if (isalpha(c) || c == '_') {
			size_t startColumn = l->column;
			const char *start = &l->content[l->pos];
			l->pos++; // Pular char
			l->column++;

			// Incrementar pos e column enquanto caractere for
			// isalnum ou _
			while (isalnum(peek(l)) || peek(l) == '_') {
				l->pos++;
				l->column++;
			}
			const char *end = &l->content[l->pos];

			// Aqui só pode ser não alfanumérico|_ ou \0
			if (!isalnum(peek(l)) && peek(l) != '_') {
				Token t;
				t.type = TOKEN_IDENTIFIER;
				t.content = l->content;
				t.start = start;
				t.length = (size_t)(end - start);
				t.line = l->line;
				t.column = startColumn;
				tokenPush(&tokens, t);
			} else {
				break;
			}
			continue;
		}

		// Operadores e simbolos
		Token t;
		t.content = l->content;
		t.start = &l->content[l->pos];
		t.length = 1;
		t.line = l->line;
		t.column = l->column;

		switch (c) {
		case '+': {
			t.type = TOKEN_PLUS;
		} break;

		case '-': {
			if (next(l) == '>') {
				t.type = TOKEN_ARROW;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_MINUS;
			}
		} break;

		case '*': {
			t.type = TOKEN_STAR;
		} break;

		case '/': {
			t.type = TOKEN_SLASH;
		} break;

		case '%': {
			t.type = TOKEN_PERCENT;
		} break;

		case '=': {
			if (next(l) == '=') {
				t.type = TOKEN_EQ;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_ASSIGN;
			}
		} break;

		case '!': {
			if (next(l) == '=') {
				t.type = TOKEN_NEQ;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_NOT;
			}
		} break;

		case '<': {
			if (next(l) == '=') {
				t.type = TOKEN_LTE;
				t.length = 2;
				l->pos++;
				l->column++;
			} else if (next(l) == '<') {
				t.type = TOKEN_SHIFT_LEFT;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_LT;
			}
		} break;

		case '>': {
			if (next(l) == '=') {
				t.type = TOKEN_GTE;
				t.length = 2;
				l->pos++;
				l->column++;
			} else if (next(l) == '>') {
				t.type = TOKEN_SHIFT_RIGHT;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_GT;
			}
		} break;

		case '&': {
			if (next(l) == '&') {
				t.type = TOKEN_AND;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_BIT_AND;
			}
		} break;

		case '|': {
			if (next(l) == '|') {
				t.type = TOKEN_OR;
				t.length = 2;
				l->pos++;
				l->column++;
			} else {
				t.type = TOKEN_BIT_OR;
			}
		} break;

		case '^': {
			t.type = TOKEN_BIT_XOR;
		} break;

		case '~': {
			t.type = TOKEN_BIT_NOT;
		} break;

		case '(': {
			t.type = TOKEN_LPAREN;
		} break;

		case ')': {
			t.type = TOKEN_RPAREN;
		} break;

		case '{': {
			t.type = TOKEN_LBRACE;
		} break;

		case '}': {
			t.type = TOKEN_RBRACE;
		} break;

		case '[': {
			t.type = TOKEN_LBRACKET;
		} break;

		case ']': {
			t.type = TOKEN_RBRACKET;
		} break;

		case ';': {
			t.type = TOKEN_SEMICOLON;
		} break;

		case ',': {
			t.type = TOKEN_COMMA;
		} break;

		case '.': {
			t.type = TOKEN_DOT;
		} break;

		case ':': {
			t.type = TOKEN_COLON;
		} break;

		case '?': {
			t.type = TOKEN_QUESTION;
		} break;

		default: {
			logger(LOG_ERROR, "Unknown character '%c' at %zu:%zu\n", c, l->line,
			       l->column);
			l->pos++;
			l->column++;
			continue;
		}
		}

		tokenPush(&tokens, t);
		l->pos++;
		l->column += t.length;
	}

	// Parte 2: transformar identifiers em keywords
	for (size_t i = 0; i < tokens.count; i++) {
		Token *t = &tokens.data[i];
		for (size_t j = 0; keywords[j].name; j++) {
			size_t keywordlen = strlen(keywords[j].name);
			if (t->length != keywordlen)
				continue;

			if (strncmp(t->start, keywords[j].name, keywordlen) == 0) {
				t->type = keywords[j].type;
			}
		}
	}

	return tokens;
}

// Destrói um lexer
void lexerDestroy(Lexer *l) {
	if (!lexerValidate(l))
		return;

	free(l);
}
