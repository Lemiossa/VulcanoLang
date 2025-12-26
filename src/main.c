/**
 * main.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval/arena.h"
#include "eval/environment.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/ast.h"
#include "parser/parser.h"
#include "util.h"

// Imprime help
void help(char *argv0) {
	logger(LOG_INFO, "Usage: %s <FILE | commands>\n", argv0);
	logger(LOG_INFO, "Commands: help, version\n", argv0);
}
// Func principal
int main(int argc, char **argv) {
	if (argc < 2) {
		logger(LOG_ERROR, "File is required\n");
		
		return 1;
	}

	if (strcmp(argv[1], "help") == 0) {
		help(argv[0]);
		exit(0);
	} else if (strcmp(argv[1], "version") == 0) {
		printf("%s v%s\n", argv[0], VERSION_STRING);
		exit(0);
	}

	char *filename = argv[1];
	FILE *f = fopen(filename, "r");

	if (!f) { // Ver se falhou ao abrir o arquivo
		logger(LOG_ERROR, "Failed to open %s: %s\n", filename, strerror(errno));
		return 1;
	}

	long filesize = fsize(f);

	char *content = (char *)malloc(filesize);
	if (!content) { // Ver se falhou ao alocar memoria para arquivo
		logger(LOG_ERROR, "Failed to alloc memory for the file\n");
		fclose(f);
		return 1;
	}

	// Ler essa merda
	if (fread(content, 1, filesize, f) != (size_t)filesize) {
		logger(LOG_ERROR, "Failed to read file: %s:%s\n", filename,
		       strerror(errno));
		free(content);
		return 1;
	}

	Lexer *lexer = lexerCreate(content, filesize);
	if (!lexerValidate(lexer)) {
		logger(LOG_ERROR, "Failed to create lexer\n");
		free(content);
		fclose(f);
		return 1;
	}
	TokenArray tokens = lexerTokenize(lexer);

	Parser *parser = parserCreate(tokens);
	if (!parserValidate(parser)) {
		logger(LOG_ERROR, "Failed to create parser\n");
		lexerDestroy(lexer);
		free(content);
		fclose(f);
		return 1;
	}

	AstNode *root = parserParse(parser);
	if (!root) {
		logger(LOG_ERROR, "Failed to parse\n");
		parserDestroy(parser);
		lexerDestroy(lexer);
		free(content);
		fclose(f);
		return 1;
	}

	Arena *arena = arenaCreate(16 * 1024);
	if (!arena) {
		logger(LOG_ERROR, "Failed to create arena allocator\n");
		parserDestroy(parser);
		lexerDestroy(lexer);
		free(content);
		fclose(f);
		return 1;
	}

	Environment *environment = environmentCreate(32, NULL);
	if (!environment) {
		logger(LOG_ERROR, "Failed to create environment\n");
		arenaDestroy(arena);
		parserDestroy(parser);
		lexerDestroy(lexer);
		free(content);
		fclose(f);
		return 1;
	}

	Value ret = eval(root, arena, environment);

	environmentDestroy(environment);
	arenaDestroy(arena);
	astDestroy(root);
	tokenDestroy(&tokens);
	lexerDestroy(lexer);
	fclose(f);

	return ret.type == VALUE_INTEGER ? (int)ret.value.integer : 0;
}
