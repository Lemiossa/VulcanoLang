/**
 * eval.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include "../parser/ast.h"
#include "arena.h"
#include "environment.h"
#include "value.h"

Value eval(AstNode *root, Arena *arena, Environment *environment);
void printValue(Value value);
