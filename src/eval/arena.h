/**
 * arena.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stddef.h>

typedef struct Arena {
	void* buffer;
	size_t length;
	size_t offset;
} Arena;

Arena* arenaCreate(size_t initial);
void* arenaAlloc(Arena* arena, size_t length);
void arenaReset(Arena* arena);
void arenaDestroy(Arena* arena);
