all: compile

OBJS = tokens.o  \
       main.o \

CPPFLAGS = -Wall -Wno-unused -Wno-deprecated -Wno-write-strings
LDFLAGS = -lfl
LIBS = -Wall

parser.tab.c: parser.y
	bison -d -o $@ $^

parser.tab.h: parser.tab.c

tokens.cpp: tokens.l parser.tab.h
	flex -o $@ $<

#%.o: %.cpp
#	g++ -c $(CPPFLAGS) -o $@ $<
#%.o: %.c
#	g++ -c $(CPPFLAGS) -o $@ $<

compile: parser.tab.c tokens.cpp main.cpp
	g++ -o $@ parser.tab.c tokens.cpp main.cpp $(LIBS) $(LDFLAGS)

clean:
	$(RM) -rf parser.tab.h parser.tab.c tokens.cpp $(OBJS) compile
