/**
 * token.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "token.h"

#include <stdarg.h>
#include <stdlib.h>

#include "util.h"

// Inicia um TokenArray
void tokenInit(TokenArray *arr) {
	arr->count = 0;
	arr->capacity = 64;
	arr->data = (Token *)malloc(arr->capacity * sizeof(Token));

	if (!arr->data) {
		logger(LOG_ERROR, "Failed to alocate token array data\n");
		return;
	}
}

// Adiciona um token a um TokenArray
void tokenPush(TokenArray *arr, Token t) {
	if (!arr) {
		logger(LOG_ERROR, "Failed to push token: array is NULL\n");
		return;
	}

	if (arr->count >= arr->capacity) {
		arr->capacity *= 2;
		arr->data = (Token *)realloc(arr->data, arr->capacity * sizeof(Token));
		if (!arr->data) {
			logger(LOG_ERROR, "Failed to realloc token array data");
			return;
		}
	}

	arr->data[arr->count++] = t;
}

// Limpa um TokenArray
// Invalída TokenArray
void tokenDestroy(TokenArray *arr) {
	if (!arr) {
		return;
	}

	free(arr->data);
	arr->data = NULL;
	arr->count = 0;
	arr->capacity = 0;
}

// Token Dump
void tokenDump(TokenArray *arr) {
	if (!arr || !arr->data) {
		return;
	}

	for (size_t i = 0; i < arr->count; i++) {
		Token t = arr->data[i];
		logger(LOG_INFO,
		       "Token %zu: type=%d, lexeme=%.*s, line=%zu, column=%zu\n", i,
		       t.type, (int)t.length, t.start, t.line, t.column);
	}
}

// Retorna o ponteiro do inicio de uma linha espeçífica
const char *getLine(const char *content, size_t line, size_t *length) {
	if (!content)
		return NULL;
	if (!length)
		return NULL;

	while (*content && line > 0) {
		if (*content == '\n') {
			line--;
		}

		content++;
	}

	if (line != 0)
		return NULL;

	const char *start = content;
	*length = 0;

	while (*content && *content != '\n') {
		(*length)++;
		content++;
	}

	return start;
}

// Faz um log de token
int tokenLogger(LogLevel level, Token t, const char *format, ...) {
	const char *color;
	const char *label;

	switch (level) {
	case LOG_ERROR: {
		color = "\033[91m";
		label = "ERROR";
	} break;

	case LOG_SUCCESS: {
		color = "\033[92m";
		label = "SUCCESS";
	} break;

	case LOG_WARNING: {
		color = "\033[93m";
		label = "WARNING";
	} break;

	case LOG_INFO:
	default: {
		color = "\033[94m";
		label = "INFO";
	} break;
	}

	// Imprimir erro
	printf("%s[%s] in line %zu, column %zu: ", color, label, t.line, t.column);

	va_list args;
	va_start(args, format);
	int count = vprintf(format, args);
	va_end(args);

	printf("\n");

	size_t lineLength = 0;
	const char *line = getLine(t.content, t.line - 1, &lineLength);

	// Apontar erro
	if (!line || lineLength == 0)
		goto end;
	int tabs = 0;

	for (size_t i = 0; i < lineLength; i++) {
		if (line[i] == '\t') {
			printf("    ");
			tabs++;
		} else {
			putchar(line[i]);
		}
	}

	putchar('\n');

	for (size_t i = 0; i < t.column - 1; i++) {
		putchar(' ');
	}

	for (size_t i = 0; i < t.length; i++) {
		putchar('^');
	}

end:
	printf("\033[0m\n");
	return count;
}
