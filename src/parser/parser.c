/**
 * parser.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "parser.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "lexer/token.h"
#include "util.h"

// Helpers

// Retorna true se está no ultimo token da lista
static bool atEnd(Parser *p) { return (p->pos >= p->tokens.count); }

// Retorna o token atual
static Token *peek(Parser *p) {
  if (!p)
    return NULL;
  if (!p->tokens.data)
    return NULL;
  if (p->pos > p->tokens.count)
    return NULL;

  return &p->tokens.data[p->pos];
}

// Avança ponteiro de tokens e retorna o token novo
static Token *advance(Parser *p) {
  if (!p)
    return NULL;
  if (!p->tokens.data)
    return NULL;
  if (atEnd(p))
    return NULL;

  return &p->tokens.data[++p->pos];
}

// Retorna o token anterior se ele existir
static Token *previous(Parser *p) {
  if (!p)
    return NULL;
  if (!p->tokens.data)
    return NULL;
  if (p->pos == 0)
    return NULL;

  return &p->tokens.data[p->pos - 1];
}

// Verifica se o tipo do token atual bate sem avançar o ponteiro
static bool check(Parser *p, TokenType type) {
  Token *t = peek(p);
  if (!t)
    return false;
  return t->type == type;
}

// Verifica se o tipo do atual bate, se bater, avança o ponteiro
static bool match(Parser *p, TokenType type) {
  if (check(p, type)) {
    advance(p);
    return true;
  }

  return false;
}

AstNode *parseLiteral(Parser *p);
AstNode *parsePrimary(Parser *p);
AstNode *parseUnary(Parser *p);
AstNode *parseMultiplication(Parser *p);
AstNode *parseAdition(Parser *p);
AstNode *parseComparison(Parser *p);
AstNode *parseEquality(Parser *p);
AstNode *parseLogicalAnd(Parser *p);
AstNode *parseLogicalOr(Parser *p);
AstNode *parseAssignment(Parser *p);
AstNode *parseExpression(Parser *p);

AstNode *parseExpressionStatement(Parser *p);
AstNode *parseBlockStatement(Parser *p);

AstNode *parseStatement(Parser *p);

// Valída um parser
bool parserValidate(Parser *p) {
  // Parser NULL
  if (!p) {
    logger(LOG_ERROR, "Parser error: parser is NULL\n");
    return false;
  }

  // Tokens NULL
  if (!p->tokens.data) {
    logger(LOG_ERROR, "Parser error: tokens is NULL\n");
    return false;
  }

  // Nenhum token
  if (p->tokens.count == 0) {
    logger(LOG_ERROR, "Parser error: no have tokens\n");
    return false;
  }

  return true;
}

// Cria um parser
Parser *parserCreate(TokenArray tokens) {
  Parser *p = (Parser *)malloc(sizeof(Parser));
  if (!p)
    return NULL;

  if (!tokens.data || tokens.count == 0)
    return NULL;

  p->tokens = tokens;
  p->pos = 0;

  return p;
}

// Parsea um parser
// Retorna um AstNode
AstNode *parserParse(Parser *p) {
  if (!parserValidate(p))
    return NULL;

  // Começar criando o nó program
  AstNode *root = astProgramCreate();
  if (!root)
    return NULL;

  // Começar parsing
  while (!atEnd(p)) {
    AstNode *statement = parseStatement(p);
    if (!statement)
      break;
    astProgramPush(root, statement);
  }

  return root;
}

// Destrói um parser
void parserDestroy(Parser *p) {
  if (!p) {
    logger(LOG_ERROR, "Parser error: parser is NULL\n");
    return;
  }

  free(p);
}

// Literal
AstNode *parseLiteral(Parser *p) {
  // Number
  if (check(p, TOKEN_NUMBER_INTEGER) || check(p, TOKEN_NUMBER_FLOAT)) {
    Token *t = peek(p);
    advance(p);

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->token = t;
    if (!node)
      return NULL;

    char *end;
    if (t->type == TOKEN_NUMBER_INTEGER) {
      long long value = strtoll(t->start, &end, 0);
      if (end == t->start) {
        free(node);
        return NULL;
      }

      node->type = NODE_NUMBER;
      node->data.number.value.integer = value;
      node->data.number.isFloat = false;
      return node;
    } else {
      double value = strtod(t->start, &end);
      if (end == t->start) {
        free(node);
        return NULL;
      }

      node->type = NODE_NUMBER;
      node->data.number.value.floating = value;
      node->data.number.isFloat = true;
      return node;
    }
  }

  // String
  if (check(p, TOKEN_STRING)) {
    Token *t = peek(p);
    advance(p);
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->token = t;
    if (!node)
      return NULL;

    node->type = NODE_STRING;
    node->data.string.start = t->start;
    node->data.string.length = t->length;
    return node;
  }

  // Boleano
  if (check(p, TOKEN_KEYWORD_TRUE) || check(p, TOKEN_KEYWORD_FALSE)) {
    Token *t = peek(p);
    advance(p);
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->token = t;
    if (!node)
      return NULL;

    node->type = NODE_BOOLEAN;
    node->data.boolean.value = t->type == TOKEN_KEYWORD_FALSE ? false : true;
    return node;
  }

  // Null
  if (check(p, TOKEN_KEYWORD_NULL)) {
    Token *t = peek(p);
    advance(p);

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->token = t;
    if (!node)
      return NULL;

    node->type = NODE_NULL;
    return node;
  }

  return NULL;
}

// Primary
AstNode *parsePrimary(Parser *p) {
  // "(" expression ")"
  if (check(p, TOKEN_LPAREN)) {
    advance(p);
    AstNode *expression = parseExpression(p);

    if (!check(p, TOKEN_RPAREN)) {
      tokenLogger(LOG_ERROR, *(peek(p)), "Expected ')' after expression");
      return NULL;
    }

    advance(p);
    return expression;
  }

  // identifiers
  if (check(p, TOKEN_IDENTIFIER)) {
    Token *t = peek(p);
    advance(p);
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));

    if (!node)
      return NULL;

    node->type = NODE_IDENTIFIER;
    node->data.identifier.name = t->start;
    node->data.identifier.length = t->length;
    return node;
  }

  return parseLiteral(p);
}

// Unary
AstNode *parseUnary(Parser *p) {
  if (match(p, TOKEN_NOT) || match(p, TOKEN_BIT_NOT) || match(p, TOKEN_MINUS)) {
    Token *operator = previous(p);
    AstNode *right = parseUnary(p);

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_UNARYOP;
    node->data.unaryOp.operator = operator->type;
    node->data.unaryOp.operand = right;
    return node;
  }

  return parsePrimary(p);
}

// Multiplication
AstNode *parseMultiplication(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseUnary(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_STAR) || match(p, TOKEN_SLASH) ||
         match(p, TOKEN_PERCENT)) {
    Token *operator = previous(p);
    AstNode *right = parseUnary(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->token = operator;
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Adition
AstNode *parseAdition(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseMultiplication(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_PLUS) || match(p, TOKEN_MINUS)) {
    Token *operator = previous(p);
    AstNode *right = parseMultiplication(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Comparison
AstNode *parseComparison(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseAdition(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_LT) || match(p, TOKEN_GT) || match(p, TOKEN_LTE) ||
         match(p, TOKEN_GTE)) {
    Token *operator = previous(p);
    AstNode *right = parseAdition(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Equality
AstNode *parseEquality(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseComparison(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_EQ) || match(p, TOKEN_NEQ)) {
    Token *operator = previous(p);
    AstNode *right = parseComparison(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Logical AND
AstNode *parseLogicalAnd(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseEquality(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_AND)) {
    Token *operator = previous(p);
    AstNode *right = parseEquality(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Logical OR
AstNode *parseLogicalOr(Parser *p) {
  if (atEnd(p))
    return NULL;

  AstNode *left = parseLogicalAnd(p);
  if (!left)
    return NULL;

  while (match(p, TOKEN_OR)) {
    Token *operator = previous(p);
    AstNode *right = parseLogicalAnd(p);
    if (!right)
      break;

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_BINARYOP;
    node->data.binaryOp.left = left;
    node->data.binaryOp.right = right;
    node->data.binaryOp.operator = operator->type;
    left = node;
  }

  return left;
}

// Assignment
// Por enquanto:
// TOKEN_IDENTIFIER "="
AstNode *parseAssignment(Parser *p) {
  Token *firstToken = peek(p);
  AstNode *target = parseLogicalOr(p);

  // Se é '=' depois da expressão
  if (match(p, TOKEN_ASSIGN)) {
    if (target->type != NODE_IDENTIFIER) {
      tokenLogger(LOG_ERROR, *firstToken, "Syntax error: Expected identifier");
      return NULL;
    }

    AstNode *value = parseAssignment(p);

    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    if (!node)
      return NULL;

    node->type = NODE_ASSIGNMENT;
    node->data.assigment.target = target;
    node->data.assigment.value = value;
    return node;
  }

  return target;
}

// Expression
AstNode *parseExpression(Parser *p) { return parseAssignment(p); }

// Expression statement
// expression ";"
AstNode *parseExpressionStatement(Parser *p) {
  AstNode *expression = parseExpression(p);
  if (!expression) {
    return NULL;
  }

  if (!check(p, TOKEN_SEMICOLON)) {
    tokenLogger(LOG_ERROR, *(peek(p)),
                "Syntax error: Expected ';' after expression");
    return NULL;
  }
  advance(p);

  AstNode *statement = (AstNode *)malloc(sizeof(AstNode));
  statement->type = NODE_EXPRESSION_STATEMENT;
  statement->data.expressionStatement.expression = expression;
  return statement;
}

// Block statement
// "{" statement* "}"
AstNode *parseBlockStatement(Parser *p) {
  if (!check(p, TOKEN_LBRACE))
    return NULL;
  Token *t = peek(p);
  advance(p); // {

  AstNode *block = astBlockCreate();
  if (!block) {
    tokenLogger(LOG_ERROR, *t, "Failed to create block\n");
    return NULL;
  }

  AstNode *statement = NULL;
  // statement*
  while (!atEnd(p) && peek(p)->type != TOKEN_RBRACE) {
    statement = parseStatement(p);
    if (statement)
      astBlockPush(block, statement);
  }

  if (!check(p, TOKEN_RBRACE)) {
    tokenLogger(LOG_ERROR, *t, "Syntax error: Unclosed block\n");
    return NULL;
  }
  advance(p); // }

  return block;
}

// Statement
AstNode *parseStatement(Parser *p) {
  if (atEnd(p))
    return NULL;

  if (check(p, TOKEN_LBRACE))
    return parseBlockStatement(p);

  // Processar NODE_IF_STATEMENT e outros aqui quando prontos....

  return parseExpressionStatement(p);
}
