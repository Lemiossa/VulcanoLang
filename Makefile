# Makefile
# Criado por Matheus Leme Da Silva
# Licença MIT
SRCDIR := $(CURDIR)/src
BUILDDIR := $(CURDIR)/build
OBJDIR := $(BUILDDIR)/obj
DEPDIR := $(BUILDDIR)/dep
BINDIR :=$(BUILDDIR)/bin

CC ?= gcc
CFLAGS := -O2 -g -Wall -Wextra -I $(SRCDIR)

TEMPDIRS := $(BUILDDIR) $(OBJDIR) $(DEPDIR) $(BINDIR)

SOURCE := \
			 $(SRCDIR)/main.c \
			 $(SRCDIR)/util.c \
			 $(SRCDIR)/lexer/token.c \
			 $(SRCDIR)/lexer/lexer.c \
			 $(SRCDIR)/parser/ast.c \
			 $(SRCDIR)/parser/parser.c \
			 $(SRCDIR)/eval/eval.c \
			 $(SRCDIR)/eval/arena.c 

OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCE))
DEP := $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SOURCE))

TARGET ?= $(BINDIR)/vul

all: $(TEMPDIRS) $(TARGET)

debug: CFLAGS := -O0 -g3 -Wall -Wextra -I $(SRCDIR)
debug: clean
debug: $(TEMPDIRS) $(TARGET)

clean:
	rm -rf $(BUILDDIR)
	find . -type f -name "*.un~" -delete

example: $(TARGET)
	$(TARGET) example.vul

format:
	find . -name "*.c" -o -name "*.h"  | xargs clang-format -i --style=LLVM 

$(TEMPDIRS):
	mkdir $@

$(TARGET): $(OBJ) | $(DEPDIR)
	$(CC) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(DEPDIR)
	mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	$(CC) $(CFLAGS) -MMD -MF $(DEPDIR)/$*.d -c $< -o $@

.PHONY: all debug clean example format
-include $(DEP)
