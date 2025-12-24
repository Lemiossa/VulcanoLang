/**
 * environment.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "environment.h"
#include "../util.h"

#include <stdlib.h>
#include <stdbool.h>

// Cria um novo Environment
Environment* environmentCreate(size_t initial, Environment* parent) {
	Environment* environment = (Environment*)malloc(sizeof(Environment));
	if (!environment) return NULL;

	environment->capacity = initial;
	environment->count = 0;
	environment->parent = parent;

	environment->objects = (Object*)malloc(environment->capacity * sizeof(Object));
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
        Object *newObjects = (Object *)realloc(environment->objects, newCapacity * sizeof(Object));
        if (!newObjects) {
            return false;
        }
        environment->capacity = newCapacity;
        environment->objects = newObjects;
    }

    environment->objects[environment->count++] = object;
    return true;
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
