/**
 * environment.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include "value.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct Object {
	char *start;
	size_t length;
	Value value;
} Object;

typedef struct Environment {
	Object *objects;
	size_t count;
	size_t capacity;
	struct Environment *parent;
} Environment;

Environment *environmentCreate(size_t initial, Environment *parent);
bool environmentPushObject(Environment *environment, Object object);
Value *environmentFindObject(Environment *environment, char *start,
                             size_t length);
void environmentDestroy(Environment *environment);
