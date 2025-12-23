/**
 * ast.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "ast.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "util.h"

#define INDENT(d)                                                              \
  for (int i = 0; i < (d); i++)                                                \
    printf(" ");

// Imprime uma ast
void astDump(AstNode *root, int depth) {
  if (!root)
    return;
  INDENT(depth);

  switch (root->type) {
  case NODE_PROGRAM: {
    printf("NODE_PROGRAM: \n");
    for (size_t i = 0; i < root->data.program.count; i++) {
      astDump(root->data.program.statements[i], depth + 1);
    }
  } break;
  case NODE_BLOCK_STATEMENT: {
    printf("NODE_BLOCK_STATEMENT: \n");
    for (size_t i = 0; i < root->data.blockStatement.count; i++) {
      astDump(root->data.blockStatement.statements[i], depth + 1);
    }
  } break;
  case NODE_EXPRESSION_STATEMENT: {
    printf("NODE_EXPRESSION_STATEMENT: \n");
    astDump(root->data.expressionStatement.expression, depth + 1);
  } break;
  case NODE_NUMBER: {
    if (!root->data.number.isFloat) {
      printf("NODE_NUMBER: %lld\n", root->data.number.value.integer);
    } else {
      printf("NODE_NUMBER: %lf\n", root->data.number.value.floating);
    }
  } break;
  case NODE_STRING: {
    printf("NODE_STRING: %.*s\n", (int)root->data.string.length,
           root->data.string.start);
  } break;
  case NODE_BOOLEAN: {
    printf("NODE_BOOLEAN: %s\n", root->data.boolean.value ? "True" : "False");
  } break;
  case NODE_NULL: {
    printf("NODE_NULL\n");
  } break;
  case NODE_IDENTIFIER: {
    printf("NODE_IDENTIFIER: %.*s\n", (int)root->data.identifier.length,
           root->data.identifier.name);
  } break;
  case NODE_BINARYOP: {
    printf("NODE_BINARYOP: \n");
    INDENT(depth + 1);
    printf("LEFT: \n");
    astDump(root->data.binaryOp.left, depth + 2);

    INDENT(depth + 1);
    printf("RIGHT: \n");
    astDump(root->data.binaryOp.right, depth + 2);

    INDENT(depth + 1);
    printf("OPERATOR: %d", root->data.binaryOp.operator);
  } break;

  case NODE_UNARYOP: {
    printf("NODE_UNARYOP: \n");
    INDENT(depth + 1);
    printf("OPERAND: \n");
    astDump(root->data.unaryOp.operand, depth + 2);

    INDENT(depth + 1);
    printf("OPERATOR: %d\n", root->data.unaryOp.operator);
  } break;

  case NODE_ASSIGNMENT: {
    printf("NODE_ASSIGNMENT: \n");

    INDENT(depth + 1);
    printf("TARGET: \n");
    astDump(root->data.assigment.target, depth + 2);

    INDENT(depth + 1);
    printf("VALUE: \n");
    astDump(root->data.assigment.value, depth + 2);
  } break;
  }
  printf("\n");
}

// Destroi uma ast
void astDestroy(AstNode *root) {
  if (!root)
    return;

  switch (root->type) {
  case NODE_PROGRAM: {
    for (size_t i = 0; i < root->data.program.count; i++) {
      astDestroy(root->data.program.statements[i]);
    }
  } break;
  case NODE_BLOCK_STATEMENT: {
    for (size_t i = 0; i < root->data.blockStatement.count; i++) {
      astDestroy(root->data.blockStatement.statements[i]);
    }
  } break;
  case NODE_EXPRESSION_STATEMENT: {
    astDestroy(root->data.expressionStatement.expression);
  } break;
  case NODE_BINARYOP: {
    astDestroy(root->data.binaryOp.left);
    astDestroy(root->data.binaryOp.right);
  } break;

  case NODE_UNARYOP: {
    astDestroy(root->data.unaryOp.operand);
  } break;

  case NODE_ASSIGNMENT: {
    astDestroy(root->data.assigment.target);
    astDestroy(root->data.assigment.value);
  } break;
  default:
    break;
  }

  free(root);
}

// Cria um novo nó program
AstNode *astProgramCreate(void) {
  AstNode *node = (AstNode *)malloc(sizeof(AstNode));
  if (!node)
    return NULL;

  node->type = NODE_PROGRAM;
  node->data.program.count = 0;
  node->data.program.capacity = 0;
  node->data.program.statements = NULL;
  return node;
}

// Adiciona um nó filho em um nó program
void astProgramPush(AstNode *program, AstNode *statement) {
  // Verificação basica
  if (!program || !statement)
    return;
  if (program->type != NODE_PROGRAM)
    return;

  if (program->data.program.count >= program->data.program.capacity) {
    size_t newCapacity = (program->data.program.capacity == 0)
                             ? 8
                             : program->data.program.capacity * 2;

    AstNode **newStatements = realloc(program->data.program.statements,
                                      newCapacity * sizeof(AstNode *));

    if (!newStatements) {
      logger(LOG_ERROR, "Failed to allocate memory for statements\n");
      return;
    }

    program->data.program.capacity = newCapacity;
    program->data.program.statements = newStatements;
  }

  program->data.program.statements[program->data.program.count++] = statement;
}

// Cria um novo nó block
AstNode *astBlockCreate(void) {
  AstNode *node = (AstNode *)malloc(sizeof(AstNode));
  if (!node)
    return NULL;

  node->type = NODE_BLOCK_STATEMENT;
  node->data.blockStatement.count = 0;
  node->data.blockStatement.capacity = 0;
  node->data.blockStatement.statements = NULL;
  return node;
}

// Adiciona um nó filho em um nó block
void astBlockPush(AstNode *block, AstNode *statement) {
  // Verificação basica
  if (!block || !statement)
    return;
  if (block->type != NODE_BLOCK_STATEMENT)
    return;

  if (block->data.blockStatement.count >= block->data.blockStatement.capacity) {
    size_t newCapacity = (block->data.blockStatement.capacity == 0)
                             ? 8
                             : block->data.blockStatement.capacity * 2;

    AstNode **newStatements = realloc(block->data.blockStatement.statements,
                                      newCapacity * sizeof(AstNode *));

    if (!newStatements) {
      logger(LOG_ERROR, "Failed to allocate memory for statements\n");
      return;
    }

    block->data.blockStatement.capacity = newCapacity;
    block->data.blockStatement.statements = newStatements;
  }

  block->data.blockStatement.statements[block->data.blockStatement.count++] =
      statement;
}
