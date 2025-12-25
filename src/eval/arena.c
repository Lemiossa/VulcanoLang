/**
 * arena.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "arena.h"

#include <stdlib.h>

// Cria uma arena
Arena *arenaCreate(size_t initial) {
	Arena *a = (Arena *)malloc(sizeof(Arena));
	if (!a)
		return NULL;

	// Inicializar
	a->buffer = malloc(initial);
	a->length = initial;
	a->offset = 0;
	if (!a->buffer) {
		free(a);
		return NULL;
	}

	return a;
}

// Aloca na arena
void *arenaAlloc(Arena *arena, size_t length) {
	if (!arena)
		return NULL;

	if (arena->offset + length > arena->length) {
		// Realocar com o novo tamanho
		size_t newLength = arena->length * 2;

		if (newLength < arena->offset + length)
			newLength = arena->offset + length;

		void *newPtr = realloc(arena->buffer, newLength);
		if (!newPtr)
			return NULL;

		arena->length = newLength;
		arena->buffer = newPtr;
	}

	void *ptr = arena->buffer + arena->offset;
	arena->offset += length;
	return ptr;
}

// Reseta a Arena
void arenaReset(Arena *arena) {
	if (!arena)
		return;
	arena->offset = 0;
}

// Destroí uma Arena
void arenaDestroy(Arena *arena) {
	if (!arena)
		return;
	free(arena->buffer);
	arena->buffer = NULL;
	arena->length = 0;
	arena->offset = 0;
	free(arena);
}
