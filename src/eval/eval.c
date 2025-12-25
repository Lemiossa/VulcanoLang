/**
 * eval.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "../lexer/token.h"
#include "../parser/ast.h"
#include "arena.h"
#include "eval.h"

#define KEYBOARD_BUFFER_SIZE 1024

// Funções built-in
Value builtinPrint(Value *args, size_t argc, Arena *arena, Environment *environment) {
	(void)arena;
    (void)environment;
    for (size_t i = 0; i < argc; i++) {
		valuePrint(args[i]);
	    if (i < argc - 1)
            printf(" ");
    }
	return null();
}

Value builtinInput(Value *args, size_t argc, Arena *arena, Environment *environment) {
    builtinPrint(args, argc, arena, environment); 

    char *buffer = arenaAlloc(arena, KEYBOARD_BUFFER_SIZE);  
    if (fgets(buffer, KEYBOARD_BUFFER_SIZE, stdin) == NULL)
        return null();

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = 0;
        len--;
    }

    Value v;
    v.type = VALUE_STRING;
    v.value.string.start = buffer;
    v.value.string.length = len;
    return v;
}

Value builtinLength(Value *args, size_t argc, Arena *arena, Environment	*environment) {
	(void)arena;
	(void)environment;
	if (argc == 0 || argc > 1) {
		logger(LOG_ERROR, "Runtime error: length(): invalid arguments\n");
		return errorSignal();
	}

	// Por enquanto, só strings
	if (args[0].type != VALUE_STRING) {
		logger(LOG_ERROR, "Runtime error: length(): invalid type\n");
		return errorSignal();
	}

	return integer(args[0].value.string.length);
}

// registra builtins caso o env não tenha pai
void registerBuiltins(Environment *environment) {
	if (!environment)
		return;
	if (environment->parent)
		return;

	Object object;
	object.start = "print";
	object.length = 5;
	object.value.type = VALUE_FUNCTION_BUILTIN;
	object.value.value.builtin = builtinPrint;
	environmentPushObject(environment, object);

	object.start = "input";
	object.length = 5;
	object.value.type = VALUE_FUNCTION_BUILTIN;
	object.value.value.builtin = builtinInput;
	environmentPushObject(environment, object);

	object.start = "length";
	object.length = 6;
	object.value.type = VALUE_FUNCTION_BUILTIN;
	object.value.value.builtin = builtinLength;
	environmentPushObject(environment, object);
}

Value evalProgram(AstNode *root, Arena *arena, Environment *environment);
Value evalBlockStatement(AstNode *root, Arena *arena, Environment *environment);
Value evalExpressionStatement(AstNode *root, Arena *arena,
                              Environment *environment);
Value evalReturnStatement(AstNode *root, Arena *arena,
                          Environment *environment);
Value evalIfStatement(AstNode *root, Arena *arena, Environment *environment);
Value evalVarStatement(AstNode *root, Arena *arena, Environment *environment);
Value evalFnStatement(AstNode *root, Arena *arena, Environment *environment);

Value evalNumber(AstNode *root, Arena *arena, Environment *environment);
Value evalString(AstNode *root, Arena *arena, Environment *environment);
Value evalBoolean(AstNode *root, Arena *arena, Environment *environment);
Value evalIdentifier(AstNode *root, Arena *arena, Environment *environment);
Value evalAssignment(AstNode *root, Arena *arena, Environment *environment);
Value evalBinaryOp(AstNode *root, Arena *arena, Environment *environment);
Value evalUnaryOp(AstNode *root, Arena *arena, Environment *environment);
Value evalCall(AstNode *root, Arena *arena, Environment *environment);

// Executa uma ast
Value eval(AstNode *root, Arena *arena, Environment *environment) {
	Value v = null();
	registerBuiltins(environment);

	if (!root) {
		logger(LOG_ERROR,
		       "Internal error: Failed to execute code: no have ast\n");
		v.type = VALUE_INTEGER;
		v.value.integer = -1;
		return v;
	}

	if (!arena) {
		logger(LOG_ERROR,
		       "Internal error: Failed to execute code: no have arena\n");
		v.type = VALUE_INTEGER;
		v.value.integer = -1;
		return v;
	}

	if (!environment) {
		logger(LOG_ERROR,
		       "Internal error: Failed to execute code: no have environment\n");
		v.type = VALUE_INTEGER;
		v.value.integer = -1;
		return v;
	}

	switch (root->type) {
	case NODE_PROGRAM: {
		v = evalProgram(root, arena, environment);
	} break;
	case NODE_BLOCK_STATEMENT: {
		v = evalBlockStatement(root, arena, environment);
	} break;
	case NODE_EXPRESSION_STATEMENT: {
		v = evalExpressionStatement(root, arena, environment);
	} break;
	case NODE_RETURN_STATEMENT: {
		v = evalReturnStatement(root, arena, environment);
	} break;
	case NODE_IF_STATEMENT: {
		v = evalIfStatement(root, arena, environment);
	} break;
	case NODE_VAR_STATEMENT: {
		v = evalVarStatement(root, arena, environment);
	} break;
	case NODE_FN_STATEMENT: {
		v = evalFnStatement(root, arena, environment);
	} break;
	case NODE_NUMBER: {
		v = evalNumber(root, arena, environment);
	} break;
	case NODE_STRING: {
		v = evalString(root, arena, environment);
	} break;
	case NODE_BOOLEAN: {
		v = evalBoolean(root, arena, environment);
	} break;
	case NODE_NULL:
		break; // Ja inicia em NULL;
	case NODE_IDENTIFIER: {
		v = evalIdentifier(root, arena, environment);
	} break;
	case NODE_ASSIGNMENT: {
		v = evalAssignment(root, arena, environment);
	} break;
	case NODE_BINARYOP: {
		v = evalBinaryOp(root, arena, environment);
	} break;
	case NODE_UNARYOP: {
		v = evalUnaryOp(root, arena, environment);
	} break;
	case NODE_CALL: {
		v = evalCall(root, arena, environment);
	} break;
	}

	return v;
}

// Program
Value evalProgram(AstNode *root, Arena *arena, Environment *environment) {
	Value v = null();
	for (size_t i = 0; i < root->data.program.count; i++) {
		v = eval(root->data.program.statements[i], arena, environment);
	}
	return returnSignalToValue(v); // Para caso o usuario use return direto
}

// Block
Value evalBlockStatement(AstNode *root, Arena *arena,
                         Environment *environment) {
	for (size_t i = 0; i < root->data.blockStatement.count; i++) {
		Value tmp =
		    eval(root->data.blockStatement.statements[i], arena, environment);
		if (tmp.type == VALUE_RETURN_SIGNAL) {
			return tmp;
		}
	}
	return null();
}

// Expression
Value evalExpressionStatement(AstNode *root, Arena *arena,
                              Environment *environment) {
	return eval(root->data.expressionStatement.expression, arena, environment);
}

// Return
Value evalReturnStatement(AstNode *root, Arena *arena,
                          Environment *environment) {
	return returnSignal(
	    returnSignalToValue(
	        eval(root->data.returnStatement.statement, arena, environment)),
	    arena);
}

// If
Value evalIfStatement(AstNode *root, Arena *arena, Environment *environment) {
	Value condition =
	    eval(root->data.ifStatement.condition, arena, environment);

	Value ret;
	if (isTrue(condition)) {
		ret = eval(root->data.ifStatement.thenBranch, arena, environment);
	} else {
		ret = eval(root->data.ifStatement.elseBranch, arena, environment);
	}

	return returnSignalToValue(ret);
}

// Var
Value evalVarStatement(AstNode *root, Arena *arena, Environment *environment) {
	Object variable;
	variable.start =
	    (char *)root->data.varStatement.identifier->data.identifier.name;
	variable.length =
	    root->data.varStatement.identifier->data.identifier.length;
	variable.value =
	    eval(root->data.varStatement.expression, arena, environment);

	if (variable.value.type == VALUE_ERROR_SIGNAL)
		return errorSignal();

	if (!environmentPushObject(environment, variable)) {
		logger(LOG_ERROR, "Internal error: Failed to push variable\n");
		return errorSignal();
	}

	return null();
}

// Fn
Value evalFnStatement(AstNode *root, Arena *arena, Environment *environment) {
	(void)arena;
	Object object;
	object.start =
	    (char *)root->data.fnStatement.functionName->data.identifier.name;
	object.length = root->data.fnStatement.functionName->data.identifier.length;
	object.value = function(root);

	if (!environmentPushObject(environment, object)) {
		logger(LOG_ERROR, "Internal error: Failed to push function\n");
		return errorSignal();
	}

	return object.value;
}

// Number
Value evalNumber(AstNode *root, Arena *arena, Environment *environment) {
	(void)arena;
	(void)environment;
	Value v = null();
	if (root->data.number.isFloat) {
		v = floating(root->data.number.value.floating);
	} else {
		v = integer(root->data.number.value.integer);
	}
	return v;
}

// String
Value evalString(AstNode *root, Arena *arena, Environment *environment) {
	(void)arena;
	(void)environment;
	return string(root->data.string.start, root->data.string.length);
}

// Boolean
Value evalBoolean(AstNode *root, Arena *arena, Environment *environment) {
	(void)arena;
	(void)environment;
	return boolean(root->data.boolean.value);
}

// Identifier
Value evalIdentifier(AstNode *root, Arena *arena, Environment *environment) {
	(void)arena;
	Value *value =
	    environmentFindObject(environment, (char *)root->data.identifier.name,
	                          root->data.identifier.length);
	if (!value) {
		tokenLogger(LOG_ERROR, *root->token,
		            "Runtime error: Undefined reference: %.*s\n",
		            root->data.identifier.length, root->data.identifier.name);
		return errorSignal();
	}

	return *value;
}

// Assignment
Value evalAssignment(AstNode *root, Arena *arena, Environment *environment) {
	Value *v = environmentFindObject(
	    environment, (char *)root->data.assigment.target->data.identifier.name,
	    root->data.assigment.target->data.identifier.length);
	if (!v) {
		tokenLogger(LOG_ERROR, *root->data.assigment.target->token,
		            "Runtime error: Undefined reference: %.*s\n",
		            root->data.assigment.target->data.identifier.length,
		            root->data.assigment.target->data.identifier.name);
		return errorSignal();
	}

	*v = eval(root->data.assigment.value, arena, environment);

	return null();
}

// BinaryOp
Value evalBinaryOp(AstNode *root, Arena *arena, Environment *environment) {
	Value v;

	Value left = eval(root->data.binaryOp.left, arena, environment);
	Value right = eval(root->data.binaryOp.right, arena, environment);
	if (root->data.binaryOp.op == TOKEN_OR) {
		v = boolean(isTrue(left) || isTrue(right));
	} else if (root->data.binaryOp.op == TOKEN_AND) {
		v = boolean(isTrue(left) && isTrue(right));
	} else if (root->data.binaryOp.op == TOKEN_BIT_OR) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer | right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise or with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_BIT_XOR) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer ^ right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise xor with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_BIT_AND) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer & right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Bitwise and with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_EQ) {
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
	} else if (root->data.binaryOp.op == TOKEN_NEQ) {
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
	} else if (root->data.binaryOp.op == TOKEN_LT) {
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
	} else if (root->data.binaryOp.op == TOKEN_GT) {
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
	} else if (root->data.binaryOp.op == TOKEN_LTE) {
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
	} else if (root->data.binaryOp.op == TOKEN_GTE) {
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
	} else if (root->data.binaryOp.op == TOKEN_SHIFT_LEFT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer << right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Shift with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_SHIFT_RIGHT) {
		if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
			v = integer(left.value.integer >> right.value.integer);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Shift with "
			            "incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_PLUS) {
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
			char *start = (char *)arenaAlloc(arena, newLength);
			memcpy(start, left.value.string.start, left.value.string.length);
			memcpy(start + left.value.string.length, right.value.string.start,
			       right.value.string.length);
			v = string(start, newLength);
		} else {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Sum with incompatible types");
			return errorSignal();
		}
	} else if (root->data.binaryOp.op == TOKEN_MINUS) {
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
	} else if (root->data.binaryOp.op == TOKEN_STAR) {
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
	} else if (root->data.binaryOp.op == TOKEN_SLASH) {
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
	} else if (root->data.binaryOp.op == TOKEN_PERCENT) {
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
Value evalUnaryOp(AstNode *root, Arena *arena, Environment *environment) {
	Value v = null();
	Value operand = eval(root->data.unaryOp.operand, arena, environment);
	TokenType op = root->data.unaryOp.op;

	if (op == TOKEN_PLUS) {
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
	} else if (op == TOKEN_MINUS) {
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
	} else if (op == TOKEN_BIT_NOT) {
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

// Call
Value evalCall(AstNode *root, Arena *arena, Environment *environment) {
	Value callee = eval(root->data.call.callee, arena, environment);

	Value *args = arenaAlloc(arena, sizeof(Value) * root->data.call.argc);
	for (size_t i = 0; i < root->data.call.argc; i++) {
		args[i] = eval(root->data.call.args[i], arena, environment);
	}

	Value result;

	if (callee.type == VALUE_FUNCTION_DEFINITION) {

		AstNode *fn = callee.value.function;

		if (root->data.call.argc != fn->data.fnStatement.paramCount) {
			tokenLogger(LOG_ERROR, *root->token,
			            "Runtime error: Invalid parameters");
			return errorSignal();
		}

		Environment *functionEnvironment = environmentCreate(8, environment);
		for (size_t i = 0; i < fn->data.fnStatement.paramCount; i++) {
			Object object;
			object.start =
			    (char *)fn->data.fnStatement.params[i]->data.identifier.name;
			object.length =
			    fn->data.fnStatement.params[i]->data.identifier.length;
			object.value = args[i];
			environmentPushObject(functionEnvironment, object);
		}

		result =
		    eval(fn->data.fnStatement.statement, arena, functionEnvironment);

		environmentDestroy(functionEnvironment);
	} else if (callee.type == VALUE_FUNCTION_BUILTIN) {
		result = callee.value.builtin(args, root->data.call.argc, arena, environment);
	} else {
		tokenLogger(LOG_ERROR, *root->token,
		            "Runtime error: Called something that isn't a function");
		return errorSignal();
	}

	return returnSignalToValue(result);
}
