/**
 * value.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include "../parser/ast.h"
#include "arena.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Environment Environment;

typedef enum {
	// Literais
	VALUE_INTEGER = 1,
	VALUE_FLOATING,
	VALUE_STRING,
	VALUE_BOOLEAN,
	VALUE_NULL,

	// Especiais
	VALUE_FUNCTION_DEFINITION,
	VALUE_FUNCTION_BUILTIN,

	// Sinais
	VALUE_RETURN_SIGNAL,
	VALUE_ERROR_SIGNAL
} ValueType;

typedef struct Value {
	ValueType type;
	union {
		long long integer;
		double floating;
		struct {
			const char *start;
			size_t length;
		} string;
		bool boolean;
		struct Value *returnValue;
		AstNode *function;
		struct Value (*builtin)(struct Value *, size_t, Arena *, Environment *);
	} value;
} Value;

void valuePrint(Value value);
Value integer(long long value);
Value floating(double value);
Value string(const char *start, size_t len);
Value boolean(bool value);
Value null(void);
Value function(AstNode *f);
Value returnSignal(Value value, Arena *arena);
Value errorSignal(void);
Value returnSignalToValue(Value v);
bool isTrue(Value value);
