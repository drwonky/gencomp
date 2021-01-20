include Rules.make

CFLAGS = -g -DDEBUG=0 

SRC = sp_def_lex.c sp_def_parse.c tokens.c parse.c main.c symtab.c machine.c code.c stack.c
HDR = code.h datatypes.h globals.h machine.h opcodes.h sp_parse.h stack.h symtab.h template_tokens.h
OBJ = $(SRC:.c=.o)
PROG = genc
BIN_DIR = $(GLOBAL_TOOLS_DIR)

all: $(PROG)

depend: $(SRC) $(HDR) dep

dep:	$(SRC)
	$(GCC) -M $(CFLAGS) $^ > .depend

sp_def_lex.c: sp_parse.l
	$(LEX) -t $^ | sed -e 's/yy/pr/g' > $@

sp_def_parse.c: sp_gram.y
	$(YACC) -btmp $^
	sed -e 's/yy/pr/g' -e 's/YYSTYPE/PRSTYPE/g' < tmp.tab.c > $@
	rm -f tmp.tab.c

parse.c: parse.y
	$(YACC) -d -btmp $^
	mv tmp.tab.c $@

template_tokens.h: parse.c
	mv tmp.tab.h $@

tokens.c: tokens.l
	$(LEX) -t $^ > $@

genc: $(OBJ)
	$(CC) -o $@ $^ 

install: 
	$(INSTALL_BIN) $(PROG) $(BIN_DIR)

rmbin:
	rm -f $(PROG)

clean: 
	rm -f $(OBJ) lex.yy.c sp_def_lex.c sp_def_parse.c tokens.c parse.c template_tokens.h

rmrcs:
	rm -f *.[hyl]

binclean: clean rmrcs

realclean: clean rmrcs rmbin

ifeq (.depend, $(wildcard .depend))
include .depend
endif
