all: compile

OBJS = symtab.o  \
       hash.o \
       expr.o \
       stmt.o \
       main.o \

CPPFLAGS = -Wall -Wno-unused -Wno-deprecated -Wno-write-strings -fpermissive
LDFLAGS = -lfl
LIBS = -Wall

parser.tab.c: parser.y
	bison -d -o $@ $^

parser.tab.h: parser.tab.c

tokens.cpp: tokens.l parser.tab.h
	flex -o $@ $<

%.o: %.cpp
	g++ -c $(CPPFLAGS) -o $@ $<
%.o: %.c
	g++ -c $(CPPFLAGS) -o $@ $<

compile_o: tokens.cpp parser.tab.c $(OBJS)
	g++ tokens.cpp parser.tab.c $(OBJS) $(LIBS) $(LDFLAGS) -g -o $@

compile: tokens.cpp parser.tab.c symtab.c hash.c expr.c stmt.c main.cpp
	g++ tokens.cpp parser.tab.c symtab.c hash.c expr.c stmt.c main.cpp $(LIBS) $(LDFLAGS) -g -o $@

clean:
	$(RM) -rf parser.tab.h parser.tab.c tokens.cpp $(OBJS) compile
