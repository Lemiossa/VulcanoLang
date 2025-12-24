/**
 * eval.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "eval.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../lexer/token.h"
#include "../parser/ast.h"
#include "arena.h"

// Imprime um "value"
void printValue(Value value) {
	switch (value.type) {
		case VALUE_INTEGER:
			logger(LOG_INFO, "%lld\n", value.value.integer);
			break;
		case VALUE_FLOATING:
			logger(LOG_INFO, "%.16lf\n", value.value.floating);
			break;
		case VALUE_STRING:
			// Como você tem length explícito, usa %.*s
			logger(LOG_INFO, "%.*s\n", (int)value.value.string.length,
			       value.value.string.start);
			break;
		case VALUE_BOOLEAN:
			logger(LOG_INFO, "%s\n", value.value.boolean ? "true" : "false");
			break;
		case VALUE_NULL:
			logger(LOG_INFO, "null\n");
			break;
		case VALUE_RETURN_SIGNAL:
			logger(LOG_INFO, "<return signal>\n");
			break;
		case VALUE_ERROR_SIGNAL:
			logger(LOG_INFO, "<error signal>\n");
			break;
		default:
			logger(LOG_INFO, "<unknown value>\n");
			break;
	}
}

// Retorna um Value integer
static Value integer(long long value) {
	Value v;
	v.type = VALUE_INTEGER;
	v.value.integer = value;
	return v;
}

// Retorna um Value float(double)
static Value floating(double value) {
	Value v;
	v.type = VALUE_FLOATING;
	v.value.floating = value;
	return v;
}

// Retorna um Value string
static Value string(const char* start, size_t len) {
	Value v;
	v.type = VALUE_STRING;
	v.value.string.start = start;
	v.value.string.length = len;
	return v;
}

// Retorna um Value boolean
static Value boolean(bool value) {
	Value v;
	v.type = VALUE_BOOLEAN;
	v.value.boolean = value;
	return v;
}

// Retorna um Value null
static Value null(void) {
	Value v;
	v.type = VALUE_NULL;
	return v;
}

// Retorna um Value return signal
static Value returnSignal(Value value, Arena* arena) {
	Value* ret = arenaAlloc(arena, sizeof(Value));
	*ret = value;

	Value v;
	v.type = VALUE_RETURN_SIGNAL;
	v.value.returnValue = ret;
	return v;
}

// Retorna um Value error signal
static Value errorSignal(void) {
	Value v;
	v.type = VALUE_ERROR_SIGNAL;
	return v;
}

// Retorna o Value de retorno de um returnSignal
static Value returnSignalToValue(Value v) {
	if (v.type == VALUE_RETURN_SIGNAL) {
		return *v.value.returnValue;
	}
	return v;
}

// Retorna true se um Value for verdadeiro
static bool isTrue(Value value) {
	switch (value.type) {
		case VALUE_INTEGER: {
			return value.value.integer != 0;
		} break;
		case VALUE_FLOATING: {
			return value.value.floating != 0.0f;
		} break;
		case VALUE_STRING: {
			return value.value.string.length > 0;
		} break;
		case VALUE_BOOLEAN: {
			return value.value.boolean;
		} break;
		case VALUE_NULL: {
			return false;
		} break;
		case VALUE_RETURN_SIGNAL:
		case VALUE_ERROR_SIGNAL: {
			logger(LOG_ERROR, "Internal error: control signal passed to %s()\n",
			       __func__);
			return false;
		} break;
		default: {
			return false;
		} break;
	}
}

// --------------------------------------

Value evalProgram(AstNode* root, Arena* arena);
Value evalBlockStatement(AstNode* root, Arena* arena);
Value evalExpressionStatement(AstNode* root, Arena* arena);
Value evalReturnStatement(AstNode* root, Arena* arena);
Value evalIfStatement(AstNode* root, Arena* arena);
Value evalNumber(AstNode* root, Arena* arena);
Value evalString(AstNode* root, Arena* arena);
Value evalBoolean(AstNode* root, Arena* arena);
Value evalBinaryOp(AstNode* root, Arena* arena);
Value evalUnaryOp(AstNode* root, Arena* arena);

// Executa uma ast
Value eval(AstNode* root, Arena* arena) {
	Value v = null();

	if (!root) {
		logger(LOG_ERROR, "Failed to execute code: no have ast\n");
		v.type = VALUE_INTEGER;
		v.value.integer = -1;
		return v;
	}

	if (!arena) {
		logger(LOG_ERROR,
		       "Failed to execute code: no have arena for allocations\n");
		v.type = VALUE_INTEGER;
		v.value.integer = -1;
		return v;
	}

	switch (root->type) {
		case NODE_PROGRAM: {
			v = evalProgram(root, arena);
		} break;
		case NODE_BLOCK_STATEMENT: {
			v = evalBlockStatement(root, arena);
		} break;
		case NODE_EXPRESSION_STATEMENT: {
			v = evalExpressionStatement(root, arena);
		} break;
		case NODE_RETURN_STATEMENT: {
			v = evalReturnStatement(root, arena);
		} break;
		case NODE_IF_STATEMENT: {
			v = evalIfStatement(root, arena);
		} break;
		case NODE_NUMBER: {
			v = evalNumber(root, arena);
		} break;
		case NODE_STRING: {
			v = evalString(root, arena);
		} break;
		case NODE_BOOLEAN: {
			v = evalBoolean(root, arena);
		} break;
		case NODE_NULL:
			break;  // Ja inicia em NULL;
		case NODE_IDENTIFIER:
			break;  // Nada por enquanto, não temos escopo nem nada
			        // que precise de identificadores, retorna NULL
		case NODE_ASSIGNMENT:
			break;
		case NODE_BINARYOP: {
			v = evalBinaryOp(root, arena);
		} break;
		case NODE_UNARYOP: {
			v = evalUnaryOp(root, arena);
		} break;
	}

	return v;
}

// Program
Value evalProgram(AstNode* root, Arena* arena) {
	Value v = null();
	for (size_t i = 0; i < root->data.program.count; i++) {
		v = eval(root->data.program.statements[i], arena);
	}
	return returnSignalToValue(v);  // Para caso o usuario use return direto
}

// Block
Value evalBlockStatement(AstNode* root, Arena* arena) {
	for (size_t i = 0; i < root->data.blockStatement.count; i++) {
		Value tmp = eval(root->data.blockStatement.statements[i], arena);
		if (tmp.type == VALUE_RETURN_SIGNAL) {
			return tmp;
		}
	}
	return null();
}

// Expression
Value evalExpressionStatement(AstNode* root, Arena* arena) {
	return eval(root->data.expressionStatement.expression, arena);
}

// Return
Value evalReturnStatement(AstNode* root, Arena* arena) {
	return returnSignal(eval(root->data.returnStatement.expression, arena),
	                    arena);
}

// If
Value evalIfStatement(AstNode* root, Arena* arena) {
	Value condition = eval(root->data.ifStatement.condition, arena);

	Value ret;
	if (isTrue(condition)) {
		ret = eval(root->data.ifStatement.thenBranch, arena);
	} else {
		ret = eval(root->data.ifStatement.elseBranch, arena);
	}

	return returnSignalToValue(ret);
}

// Number
Value evalNumber(AstNode* root, Arena* arena) {
	(void)arena;
	Value v = null();
	if (root->data.number.isFloat) {
		v = floating(root->data.number.value.floating);
	} else {
		v = integer(root->data.number.value.integer);
	}
	return v;
}

// String
Value evalString(AstNode* root, Arena* arena) {
	(void)arena;
	return string(root->data.string.start, root->data.string.length);
}

// Boolean
Value evalBoolean(AstNode* root, Arena* arena) {
	(void)arena;
	return boolean(root->data.boolean.value);
}

// BinaryOp
Value evalBinaryOp(AstNode* root, Arena* arena) {
	Value v;

	Value left = eval(root->data.binaryOp.left, arena);
	Value right = eval(root->data.binaryOp.right, arena);
	if (root->data.binaryOp.operator == TOKEN_OR) {
		v = boolean(isTrue(left) || isTrue(right));
	} else if (root->data.binaryOp.operator == TOKEN_AND) {
		v = boolean(isTrue(left) && isTrue(right));
	} else if (root->data.binaryOp.operator == TOKEN_BIT_OR) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer | right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise or with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_BIT_XOR) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer ^ right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise xor with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_BIT_AND) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer & right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise and with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_EQ) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer == right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating == right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating == (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer == right.value.floating);
		} else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
			if (left.value.string.length != right.value.string.length) {
				v = boolean(false);
			} else {
				v = boolean(strncmp(left.value.string.start,
				                    right.value.string.start,
				                    left.value.string.length) == 0);
			}
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Comparison with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_NEQ) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer != right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating != right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating != (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer != right.value.floating);
		} else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
			if (left.value.string.length != right.value.string.length) {
				v = boolean(true);
			} else {
				v = boolean(strncmp(left.value.string.start,
				                    right.value.string.start,
				                    left.value.string.length) != 0);
			}
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Comparison with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_LT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer < right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating < right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating < (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer < right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Comparison with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_GT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer > right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating > right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating > (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer > right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Comparison with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_LTE) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer <= right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating <= right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating <= (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer <= right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Comparison with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_GTE) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = boolean(left.value.integer >= right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = boolean(left.value.floating >= right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = boolean(left.value.floating >= (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = boolean((double)left.value.integer >= right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Comparison with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_SHIFT_LEFT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer << right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Shift with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_SHIFT_RIGHT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer >> right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Shift with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_PLUS) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer + right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = floating(left.value.floating + right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = floating(left.value.floating + (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = floating((double)left.value.integer + right.value.floating);
		} else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
			size_t newLength =
			    left.value.string.length + right.value.string.length;
			char* start = (char*)arenaAlloc(arena, newLength);
			memcpy(start, left.value.string.start, left.value.string.length);
			memcpy(start + left.value.string.length, right.value.string.start,
			       right.value.string.length);
			v = string(start, newLength);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Sum with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_MINUS) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer - right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = floating(left.value.floating - right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = floating(left.value.floating - (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = floating((double)left.value.integer - right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Subtraction with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_STAR) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer * right.value.integer);
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			v = floating(left.value.floating * right.value.floating);
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			v = floating(left.value.floating * (double)right.value.integer);
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			v = floating((double)left.value.integer * right.value.floating);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Multiplication with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_SLASH) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			if (right.value.integer == 0) {
				if (root->token)
					tokenLogger(LOG_ERROR, *root->token,
					            "Runtime error: Division by zero");
				return errorSignal();
			} else {
				return integer(left.value.integer / right.value.integer);
			}
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			if (right.value.floating == 0.0f) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Division by zero");
				return errorSignal();
			} else {
				return floating(left.value.floating / right.value.floating);
			}
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			if (right.value.integer == 0) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Division by zero");
				return errorSignal();
			} else {
				return floating(left.value.floating /
				                (double)right.value.integer);
			}
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			if (right.value.floating == 0.0f) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Division by zero");
				return errorSignal();
			} else {
				return floating((double)left.value.integer /
				                right.value.floating);
			}
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Division with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.operator == TOKEN_PERCENT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			if (right.value.integer == 0) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Module by zero");
				return errorSignal();
			} else {
				return integer(left.value.integer % right.value.integer);
			}
		} else if (left.type == VALUE_FLOATING &&
		           right.type == VALUE_FLOATING) {
			if (right.value.floating == 0.0f) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Module by zero");
				return errorSignal();
			} else {
				return floating(
				    fmod(left.value.floating, right.value.floating));
			}
		} else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
			if (right.value.integer == 0) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Module by zero");

				return errorSignal();
			} else {
				return floating(
				    fmod(left.value.floating, (double)right.value.integer));
			}
		} else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
			if (right.value.floating == 0.0f) {
				tokenLogger(LOG_ERROR, *root->token,
				            "Runtime error: Module by zero");
				return errorSignal();
			} else {
				return floating(
				    fmod((double)left.value.integer, right.value.floating));
			}
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Module with incompatible types");
			return errorSignal();
		}
	}

	return v;
}

// UnaryOp
Value evalUnaryOp(AstNode* root, Arena* arena) {
	Value v = null();
	Value operand = eval(root->data.unaryOp.operand, arena);
	TokenType operator = root->data.unaryOp.operator;

	if (operator == TOKEN_PLUS) {
		if (operand.type == VALUE_INTEGER) {
			v = integer(operand.value.integer);
		} else if (operand.type == VALUE_FLOATING) {
			v = floating(operand.value.floating);
		} else {
			tokenLogger(
			    LOG_ERROR, *root->token,
			    "Runtime error: Unary plus operator with incompatible type\n");
			return errorSignal();
		}
	} else if (operator == TOKEN_MINUS) {
		if (operand.type == VALUE_INTEGER) {
			v = integer(-operand.value.integer);
		} else if (operand.type == VALUE_FLOATING) {
			v = floating(-operand.value.floating);
		} else {
			tokenLogger(
			    LOG_ERROR, *root->token,
			    "Runtime error: Unary minus operator with incompatible type\n");
			return errorSignal();
		}
	} else if (operator == TOKEN_BIT_NOT) {
		if (operand.type == VALUE_INTEGER) {
			v = integer(~operand.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Unary bitwise not operator with "
			            "incompatible type\n");
			return errorSignal();
		}
	}

	return v;
}
