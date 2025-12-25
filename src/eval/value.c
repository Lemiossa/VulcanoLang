/**
 * value.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "value.h"
#include "../util.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Parsea escapes
char *parseEscapes(const char *start, size_t length, size_t *outLength) {
	char *buffer = malloc(length + 1); // no máximo o tamanho original
	if (!buffer)
		return NULL;

	size_t j = 0;
	for (size_t i = 0; i < length; i++) {
		if (start[i] == '\\' && i + 1 < length) {
			i++;
			switch (start[i]) {
			case 'n':
				buffer[j++] = '\n';
				break;
			case 't':
				buffer[j++] = '\t';
				break;
			case 'r':
				buffer[j++] = '\r';
				break;
			case '\\':
				buffer[j++] = '\\';
				break;
			case '"':
				buffer[j++] = '"';
				break;
			default:
				buffer[j++] = start[i];
				break; // escapa qualquer outro
			}
		} else {
			buffer[j++] = start[i];
		}
	}

	*outLength = j;
	buffer[j] = '\0';
	return buffer;
}

// Imprime um "value"
void valuePrint(Value value) {
	switch (value.type) {
	case VALUE_INTEGER:
		printf("%lld\n", value.value.integer);
		break;
	case VALUE_FLOATING:
		printf("%.6lf\n", value.value.floating);
		break;
	case VALUE_STRING:
		size_t len;
		char *parsed = parseEscapes(value.value.string.start,
		                            value.value.string.length, &len);
		if (parsed) {
			printf("%.*s", (int)len, parsed);
			free(parsed);
		}
		break;
	case VALUE_BOOLEAN:
		printf("%s\n", value.value.boolean ? "true" : "false");
		break;
	case VALUE_NULL:
		printf("null\n");
		break;
	default:
		logger(LOG_ERROR,
		       "Internal error: passed sinal or special value for %s()\n",
		       __func__);
		break;
	}
}

// Retorna um Value integer
Value integer(long long value) {
	Value v;
	v.type = VALUE_INTEGER;
	v.value.integer = value;
	return v;
}

// Retorna um Value float(double)
Value floating(double value) {
	Value v;
	v.type = VALUE_FLOATING;
	v.value.floating = value;
	return v;
}

// Retorna um Value string
Value string(const char *start, size_t len) {
	Value v;
	v.type = VALUE_STRING;
	v.value.string.start = start;
	v.value.string.length = len;
	return v;
}

// Retorna um Value boolean
Value boolean(bool value) {
	Value v;
	v.type = VALUE_BOOLEAN;
	v.value.boolean = value;
	return v;
}

// Retorna um Value null
Value null(void) {
	Value v;
	v.type = VALUE_NULL;
	return v;
}

// Retorna um Value de function definition
Value function(AstNode *f) {
	Value v;
	v.type = VALUE_FUNCTION_DEFINITION;
	v.value.function = f;
	return v;
}

// Retorna um Value return signal
Value returnSignal(Value value, Arena *arena) {
	Value *ret = arenaAlloc(arena, sizeof(Value));
	*ret = value;

	Value v;
	v.type = VALUE_RETURN_SIGNAL;
	v.value.returnValue = ret;
	return v;
}

// Retorna um Value error signal
Value errorSignal(void) {
	Value v;
	v.type = VALUE_ERROR_SIGNAL;
	return v;
}

// Retorna o Value de retorno de um returnSignal
Value returnSignalToValue(Value v) {
	if (v.type == VALUE_RETURN_SIGNAL) {
		return *v.value.returnValue;
	}
	return v;
}

// Retorna true se um Value for verdadeiro
bool isTrue(Value value) {
	switch (value.type) {
	case VALUE_INTEGER: {
		return value.value.integer != 0;
	} break;
	case VALUE_FLOATING: {
		return value.value.floating != 0.0f;
	} break;
	case VALUE_STRING: {
		return value.value.string.length > 0;
	} break;
	case VALUE_BOOLEAN: {
		return value.value.boolean;
	} break;
	case VALUE_NULL: {
		return false;
	} break;
	default: {
		logger(LOG_ERROR, "Internal error: Control signal or special value passed to %s(%d)\n",
				       __func__, value.type);
				return false;
	} break;
	}
}
