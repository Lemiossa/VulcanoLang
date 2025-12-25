/**
 * value.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum {
	VALUE_INTEGER = 1,
	VALUE_FLOATING,
	VALUE_STRING,
	VALUE_BOOLEAN,
	VALUE_NULL,
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
	} value;
} Value;
