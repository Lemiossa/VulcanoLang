/**
 * util.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stdio.h>

typedef enum { LOG_ERROR, LOG_SUCCESS, LOG_WARNING, LOG_INFO } LogLevel;

long fsize(FILE* f);
int logger(LogLevel level, const char* format, ...);
