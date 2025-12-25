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
LIBS := -lm
FORMATSTYLE := "{BasedOnStyle: LLVM, UseTab: ForIndentation, IndentWidth: 4, TabWidth: 4}"

TEMPDIRS := $(BUILDDIR) $(OBJDIR) $(DEPDIR) $(BINDIR)

SOURCE := \
			 $(SRCDIR)/main.c \
			 $(SRCDIR)/util.c \
			 $(SRCDIR)/lexer/token.c \
			 $(SRCDIR)/lexer/lexer.c \
			 $(SRCDIR)/parser/ast.c \
			 $(SRCDIR)/parser/parser.c \
			 $(SRCDIR)/eval/eval.c \
			 $(SRCDIR)/eval/arena.c \
			 $(SRCDIR)/eval/environment.c 

OBJ := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCE))
DEP := $(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SOURCE))

TARGET ?= $(BINDIR)/vul

all: release

debug: CFLAGS := -O0 -g3 -Wall -Wextra -DDEBUG -I $(SRCDIR)
debug: $(BUILDDIR)/.debug $(TARGET)

release: CFLAGS := -O2 -g -Wall -Wextra -DNDEBUG -I $(SRCDIR)
release: $(BUILDDIR)/.release $(TARGET)

clean:
	@echo "  RM        $(BUILDDIR)"
	@rm -rf $(BUILDDIR)

example: $(TARGET)
	@echo "  CAT       $(CURDIR)/example.vul"
	@cat example.vul
	@echo "  RUN       $(CURDIR)/example.vul"
	@$(TARGET) example.vul

format:
	@echo "  FORMAT    *.c *.h"
	@find . -name "*.c" -o -name "*.h"  | xargs clang-format -i --style=$(FORMATSTYLE) 

$(BUILDDIR)/.debug: 
	@echo "  DEBUG     BUILD"
	@if [ -f $(BUILDDIR)/.release ]; then rm -rf $(BUILDDIR); fi
	@mkdir -p $(BUILDDIR) $(OBJDIR) $(DEPDIR) $(BINDIR)
	@touch $@

$(BUILDDIR)/.release: 
	@echo "  RELEASE   BUILD"
	@if [ -f $(BUILDDIR)/.debug ]; then rm -rf $(BUILDDIR); fi
	@mkdir -p $(BUILDDIR) $(OBJDIR) $(DEPDIR) $(BINDIR)
	@touch $@

$(TARGET): $(OBJ) | $(DEPDIR)
	@echo "  LINK      $(TARGET)"
	@$(CC) -o $@ $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) $(DEPDIR)
	@echo "  CC        $<"
	@mkdir -p $(dir $@) $(dir $(DEPDIR)/$*.d)
	@$(CC) $(CFLAGS) -MMD -MF $(DEPDIR)/$*.d -c $< -o $@

.PHONY: all release debug clean example format
-include $(DEP)
