/**
 * eval.c
 * Criado por Matheus Leme Da Silva
 * Licença MIT
 */
#include "eval.h"
#include "../lexer/token.h"
#include "../parser/ast.h"
#include "arena.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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
static Value string(const char *start, size_t len) {
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
static Value returnSignal(Value *value) {
  Value v;
  v.type = VALUE_RETURN_SIGNAL;
  v.value.returnValue = value;
  return v;
}

// Retorna um Value error signal
static Value errorSignal(void) {
  Value v;
  v.type = VALUE_ERROR_SIGNAL;
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
  case VALUE_RETURN_SIGNAL: {
    return isTrue(*value.value.returnValue);
  } break;
  }
}

// --------------------------------------

// Executa uma ast
Value eval(AstNode *root, Arena *arena) {
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
    for (size_t i = 0; i < root->data.program.count; i++) {
      v = eval(root->data.program.statements[i], arena);
    }
  } break;
  case NODE_BLOCK_STATEMENT: {
    for (size_t i = 0; i < root->data.blockStatement.count; i++) {
      Value tmp = eval(root->data.blockStatement.statements[i], arena);
      if (tmp.type == VALUE_RETURN_SIGNAL) {
        return tmp;
      }
    }
  } break;
  case NODE_EXPRESSION_STATEMENT: {
    return eval(root->data.expressionStatement.expression, arena);
  } break;
  case NODE_NUMBER: {
    if (root->data.number.isFloat) {
      v = floating(root->data.number.value.floating);
    } else {
      v = integer(root->data.number.value.integer);
    }
  } break;
  case NODE_STRING: {
    v = string(root->data.string.start, root->data.string.length);
  } break;
  case NODE_BOOLEAN: {
    v.type = VALUE_BOOLEAN;
    v.value.boolean = root->data.boolean.value;
  } break;
  case NODE_NULL:
    break; // Ja inicia em NULL;
  case NODE_IDENTIFIER:
    break; // Nada por enquanto, não temos escopo nem nada que precise de
           // identificadores, retorna NULL
  case NODE_BINARYOP: {
    Value left = eval(root->data.binaryOp.left, arena);
    Value right = eval(root->data.binaryOp.right, arena);
    if (root->data.binaryOp.operator == TOKEN_OR) {
      v = boolean(isTrue(left) || isTrue(right));
    } else if (root->data.binaryOp.operator == TOKEN_AND) {
      v = boolean(isTrue(left) && isTrue(right));
    } else if (root->data.binaryOp.operator == TOKEN_EQ) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer == right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating == right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating == (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer == right.value.floating);
      } else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
        if (left.value.string.length != right.value.string.length) {
          v = boolean(false);
        } else {
          v = boolean(strncmp(left.value.string.start, right.value.string.start,
                              left.value.string.length) == 0);
        }
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_NEQ) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer != right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating != right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating != (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer != right.value.floating);
      } else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
        if (left.value.string.length != right.value.string.length) {
          v = boolean(true);
        } else {
          v = boolean(strncmp(left.value.string.start, right.value.string.start,
                              left.value.string.length) != 0);
        }
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_LT) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer < right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating < right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating < (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer < right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_GT) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer > right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating > right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating > (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer > right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_LTE) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer <= right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating <= right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating <= (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer <= right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_GTE) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = boolean(left.value.integer >= right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = boolean(left.value.floating >= right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = boolean(left.value.floating >= (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = boolean((double)left.value.integer >= right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Runtime error: Comparison with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_PLUS) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = integer(left.value.integer + right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = floating(left.value.floating + right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = floating(left.value.floating + (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = floating((double)left.value.integer + right.value.floating);
      } else if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
        size_t newLength = left.value.string.length + right.value.string.length;
        char *start = (char *)arenaAlloc(arena, newLength);
        memcpy(start, left.value.string.start, left.value.string.length);
        memcpy(start + left.value.string.length, right.value.string.start,
               right.value.string.length);
        v = string(start, newLength);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Runtime error: Sum with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_MINUS) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = integer(left.value.integer - right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = floating(left.value.floating - right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = floating(left.value.floating - (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = floating((double)left.value.integer - right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Runtime error: Subtraction with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_STAR) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        v = integer(left.value.integer * right.value.integer);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        v = floating(left.value.floating * right.value.floating);
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        v = floating(left.value.floating * (double)right.value.integer);
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        v = floating((double)left.value.integer * right.value.floating);
      } else {
        v.type = VALUE_ERROR_SIGNAL;
        tokenLogger(LOG_ERROR, *root->token,
                    "Runtime error: Multiplication with incompatible types");
      }
    }

    else if (root->data.binaryOp.operator == TOKEN_SLASH) {
      if (left.type == VALUE_INTEGER && right.type == VALUE_INTEGER) {
        if (right.value.integer == 0) {
          if (root->token) 
          	tokenLogger(LOG_ERROR, *root->token,
                      "Runtime error: Division by zero");
          else 
          	logger(LOG_ERROR, "Runtime error: Division by zero");
          return errorSignal();
        } else {
          return integer(left.value.integer / right.value.integer);
        }
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_FLOATING) {
        if (right.value.floating == 0.0f) {
          if (root->token) 
          	tokenLogger(LOG_ERROR, *root->token,
                      "Runtime error: Division by zero");
          else 
          	logger(LOG_ERROR, "Runtime error: Division by zero");
          return errorSignal();
        } else {
          return floating(left.value.floating / right.value.floating);
        }
      } else if (left.type == VALUE_FLOATING && right.type == VALUE_INTEGER) {
        if (right.value.integer == 0) {
          if (root->token) 
          	tokenLogger(LOG_ERROR, *root->token,
                      "Runtime error: Division by zero");
          else 
          	logger(LOG_ERROR, "Runtime error: Division by zero");
          return errorSignal();
        } else {
          return floating(left.value.floating / (double)right.value.integer);
        }
      } else if (left.type == VALUE_INTEGER && right.type == VALUE_FLOATING) {
        if (right.value.floating == 0.0f) {
          if (root->token) 
          	tokenLogger(LOG_ERROR, *root->token,
                      "Runtime error: Division by zero");
          else 
          	logger(LOG_ERROR, "Runtime error: Division by zero");
          return errorSignal();
        } else {
          return floating((double)left.value.integer / right.value.floating);
        }
      } else {
        tokenLogger(LOG_ERROR, *root->token,
                    "Runtime error: Division with incompatible types");
        return errorSignal();
      }
    }

  } break;
  }

  return v;
}

// Imprime um "value"
void printValue(Value value) {
  switch (value.type) {
  case VALUE_INTEGER:
    logger(LOG_INFO, "%lld\n", value.value.integer);
    break;
  case VALUE_FLOATING:
    logger(LOG_INFO, "%lf\n", value.value.floating);
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
