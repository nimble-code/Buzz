# Buzz -- a basic model checker interface
# (c) 2015 Gerard J. Holzmann
# California Institute of Technology, Pasadena, CA, USA

CC=gcc
YACC=@yacc
LEX=@lex
YFLAGS=-d
LFLAGS=-t

INSTALL_DIR=/usr/local/bin
BUZZ_DIR=`pwd`

# to use malloc instead of sbrk, add -DUSE_MALLOC
# to change the amount allocated per call to sbrk
# add -DCHUNK_SZ=N (the default N is 1024*1024)

CFLAGS=-g -O3 -Wall -pedantic -std=c99 -I. -Iinclude -D_XOPEN_SOURCE -DBUZZ_DIR=\"${BUZZ_DIR}\"

OBJ=	parse/parse.o \
	parse/chan.o   \
	lex/lex.o     \
	main/main.o   \
	init/init.o   \
	sym/sym.o     \
	trans/trans.o \
	trans/chan.o   \
	hash/bernstein.o \
	hash/fnv1a.o  \
	hash/fnv64.o  \
	hash/hsieh.o \
	hash/jenkins.o \
	hash/murmur3.o \
	hash/murmur.o \
	hash/naive.o  \
	hash/reeds.o  \
	hash/rot.o \
	hash/sdbm.o   \
	hash/simple.o \
	hash/spooky.o \
	hash/spookyshort.o \
	search/dfs.o  \
	search/dfs_claim.o \
	search/bfs.o \
	store/closed_notrace.o \
	store/closed.o \
	store/open.o

all:	buzz pre ${OBJ}

buzz:	buzz.c

pre:
	@cp include/state_orig.h include/state.h

install: all
	cp buzz ${INSTALL_DIR}
	rm buzz

${OBJ}:	include/buzz.h parse/parse.y parse/chan.y

search/bfs.o: search/bfs_common.c

search/dfs.o search/dfs_claim.o: search/dfs_common.c

store/open.o: store/open_common.c

store/closed_notrace.o store/closed.o: store/closed_common.c

parse/parse.c:	parse/parse.y
	${YACC} ${YFLAGS} parse/parse.y
	@mv -f y.tab.c parse/parse.c
	@mv -f y.tab.h include

parse/chan.c:	parse/chan.y
	${YACC} ${YFLAGS} parse/chan.y
	@mv -f y.tab.c parse/chan.c
	@rm -f y.tab.h

lex/lex.c:	lex/lex.l parse/parse.c
	${LEX} ${LFLAGS} lex/lex.l > lex/lex.c

advice:	advice.c

interface:
	@echo "store:"
	@cd store;  uno_local -localonly -extern -I../include $(notdir $(wildcard store/*[^n].c)) | sort | uniq -c
	@echo "search:"
	@cd search; uno_local -localonly -extern -I../include $(notdir $(wildcard search/*[^n].c)) | sort | uniq -c
	@# excludes *common.c

clean:
	@rm -f */*.o *.o buzz y.tab.? y.output lex/lex.yy.c *stackdump _buzz_* buzz_e
	@rm -f parse/parse.c parse/chan.c lex/lex.c lex.c parse.c advice
	@cp include/state_orig.h include/state.h
