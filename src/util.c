/**
 * util.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "util.h"

#include <stdarg.h>
#include <stdio.h>

// Log
int logger(LogLevel level, const char* format, ...) {
	const char* color;
	const char* label;

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

	printf("%s[%s] ", color, label);

	va_list args;
	va_start(args, format);
	int count = vprintf(format, args);
	va_end(args);

	printf("\033[0m");
	return count;
}

// Pega o tamanho de um arquivo
long fsize(FILE* f) {
	if (!f) return -1;

	// Colocar o offset para o final do arquivo
	if (fseek(f, 0, SEEK_END) < 0) return -1;

	// Pegar tamanho
	long size = ftell(f);
	if (size < 0) return -1;

	rewind(f);  // Resetar posição
	return size;
}
