/**
 * environment.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include "eval.h"

typedef struct Object {
	char* name;
	Value value;
} Object;

typedef struct Environment {
	Object* objects;
	size_t count;
	size_t capacity;
	struct Environment* parent;
} Environment;

Environment* environmentCreate(size_t initial, Environment* parent);
bool environmentPushObject(Environment *environment, Object object);
void environmentDestroy(Environment *environment);
