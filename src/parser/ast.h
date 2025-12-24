/**
 * ast.h
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "../lexer/token.h"
#include "../util.h"

// Tipo de nó
typedef enum {
	NODE_PROGRAM = 1,
	NODE_BLOCK_STATEMENT,
	NODE_EXPRESSION_STATEMENT,
	NODE_IF_STATEMENT,
	NODE_RETURN_STATEMENT,

	// Literais
	NODE_NUMBER,
	NODE_STRING,
	NODE_BOOLEAN,
	NODE_NULL,

	// Identificadores
	NODE_IDENTIFIER,

	// Expressões
	NODE_BINARYOP,
	NODE_UNARYOP,
	NODE_ASSIGNMENT
} NodeType;

// Nó
typedef struct AstNode {
	NodeType type;
	Token* token;

	union {
		// NODE_PROGRAM
		struct {
			struct AstNode** statements;
			size_t count;
			size_t capacity;
		} program;

		// NODE_BLOCK_STATEMENT
		struct {
			struct AstNode** statements;
			size_t count;
			size_t capacity;
		} blockStatement;

		// NODE_EXPRESSION_STATEMENT
		struct {
			struct AstNode* expression;
		} expressionStatement;

		// NODE_IF_STATEMENT
		struct {
			struct AstNode* condition;
			struct AstNode* thenBranch;
			struct AstNode* elseBranch;
		} ifStatement;

		// NODE_RETURN_STATEMENT
		struct {
			struct AstNode* expression;
		} returnStatement;

		// NODE_NUMBER
		struct {
			bool isFloat;
			union {
				long long integer;
				double floating;
			} value;
		} number;

		// NODE_STRING
		struct {
			const char* start;
			size_t length;
		} string;

		// NODE_BOOLEAN
		struct {
			bool value;
		} boolean;

		// NODE_IDENTIFIER
		struct {
			const char* name;
			size_t length;
		} identifier;

		// NODE_BINARYOP
		struct {
			struct AstNode* left;
			struct AstNode* right;
			TokenType operator;
		} binaryOp;

		// NODE_UNARYOP
		struct {
			struct AstNode* operand;
			TokenType operator;
		} unaryOp;

		// NODE_ASSIGNMENT
		struct {
			struct AstNode* target;
			struct AstNode* value;
		} assigment;
	} data;
} AstNode;

void astDump(AstNode* root, int depth);
void astDestroy(AstNode* root);

AstNode* astProgramCreate(void);
void astProgramPush(AstNode* program, AstNode* statement);

AstNode* astBlockCreate(void);
void astBlockPush(AstNode* program, AstNode* statement);
