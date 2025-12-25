/**
 * environment.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "environment.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"

// Cria um novo Environment
Environment *environmentCreate(size_t initial, Environment *parent) {
	Environment *environment = (Environment *)malloc(sizeof(Environment));
	if (!environment)
		return NULL;

	environment->capacity = initial ? initial : 1;
	environment->count = 0;
	environment->parent = parent;

	environment->objects =
	    (Object *)calloc(environment->capacity, sizeof(Object));
	if (!environment->objects) {
		free(environment);
		return NULL;
	}

	return environment;
}

// Cria um novo objeto num environment
bool environmentPushObject(Environment *environment, Object object) {
	if (!environment) {
		return false;
	}

	if (environment->count >= environment->capacity) {
		size_t newCapacity = environment->capacity * 2;
		Object *newObjects = (Object *)realloc(environment->objects,
		                                       newCapacity * sizeof(Object));
		if (!newObjects) {
			return false;
		}

		// Inicializa os novos objetos
		for (size_t i = environment->capacity; i < newCapacity; i++) {
			newObjects[i].start = NULL;
			newObjects[i].length = 0;
			newObjects[i].value.type = VALUE_NULL;
		}

		environment->capacity = newCapacity;
		environment->objects = newObjects;
	}

	environment->objects[environment->count++] = object;
	return true;
}

// Procura um objeto num environment
// Retorna o ponteiro direto para o Value do objeto
Value *environmentFindObject(Environment *environment, char *start,
                             size_t length) {
	if (!environment || !start || length == 0)
		return NULL;

	Environment *e = environment;
	while (e) {
		for (size_t i = e->count; i > 0; i--) {
			Object *object = &e->objects[i - 1];
			if (!object->start)
				continue;
			if (object->length == length &&
			    strncmp(start, object->start, length) == 0) {
				return &object->value;
			}
		}
		e = e->parent;
	}

	return NULL;
}

// Destroi um environment
void environmentDestroy(Environment *environment) {
	if (!environment)
		return;

	environment->capacity = 0;
	environment->count = 0;
	free(environment->objects);
	free(environment);
}
