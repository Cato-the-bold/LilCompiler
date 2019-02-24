%{
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <stack>
#include <unordered_map>

using namespace std;

char **inputs = NULL;

int lineno = 1;

int PRINT_LEXIME = 1;

int include_stack_ptr = 0;

const int MAX_INCLUDE_DEPTH = 256;
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];


extern "C" int yylex();
extern "C" void yyerror(const char *s);

extern "C" {
    int yywrap() {
        if(--include_stack_ptr>=0){
                fclose(yyin);
                yy_delete_buffer( YY_CURRENT_BUFFER );
                yy_switch_to_buffer(
                     include_stack[include_stack_ptr] );

            return 0;
        }else{
            inputs+=1;
            if(inputs[0]){
                yyin = fopen(inputs[0], "r");
                include_stack_ptr = 0;

                return 0;
            }else{
                return 1;
            }
        }
    }
}

int TOKEN(const char* t){
    if(PRINT_LEXIME){
        printf("File %s Line %d Token '%s' Text '%s'\n", inputs[0], lineno, t, yytext);
    }
    return 0; //without this, the program would never return.
}


int macro_state = 0;

void include_file(){
    if(include_stack_ptr>=MAX_INCLUDE_DEPTH){
        fprintf(stderr, "The include depth cannot exceed 256.\n");
        exit(1);
    }

    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;

    yytext[yyleng-1] = 0;
    yyin = fopen(yytext+1, "r");
    printf("including file: %s\n", yytext+1);
    if(!yyin){
        printf("[Error] cannot open file:%s\n",yytext+1);
    }

    yy_switch_to_buffer(yy_create_buffer( yyin, YY_BUF_SIZE ));
    BEGIN 0;

}

string id;
string replacement;
unordered_map <string, string> macros;

void define(){
   printf("Error in %s line %d:\n\t directive ’%s’ not implemented, ignoring\n", inputs[0], lineno, yytext);

}

void undefine(){
   printf("Error in %s line %d:\n  directive ’%s’ not implemented, ignoring\n", inputs[0], lineno, yytext);

}

void if_define(){
   printf("Error in %s line %d:\n  directive ’%s’ not implemented, ignoring\n", inputs[0], lineno, yytext);

}


%}


%s S_INCLUDE
%s S_DEFINE
%s S_DEFINE_2
%s S_UNDEF

%%

[\t ]                         ;
\n                              lineno++;


"/*"[^*]*"*/"                ;

"//"[^\n]*                  ;


"("	          					TOKEN("L_PAREN");
")"					          	TOKEN("R_PAREN");
"["	          					TOKEN("L_BRACKET");
"]"					          	TOKEN("R_BRACKET");
"{"         						TOKEN("L_BRACE");
"}"					          	TOKEN("R_BRACE");

","				          		TOKEN("COMMA");
"."         						TOKEN("DOT");
";"				          		TOKEN("SEMI");
"?"				          		TOKEN("QUEST");
":"				          		TOKEN("COLON");

"="						          TOKEN("ASSIGN");
"=="				          	TOKEN("EQ");
"!="			          		TOKEN("NE");
"<"				          		TOKEN("LT");
"<="	          				TOKEN("LE");
">"				          		TOKEN("GT");
">="					          TOKEN("GE");

"+"				          		TOKEN("PLUS");
"-"		          				TOKEN("MINUS");
"*"		          				TOKEN("MUL");
"/"				          		TOKEN("DIV");
"%"				          		TOKEN("MOD");
"|"		          				TOKEN("L_OR");
"&"				          		TOKEN("L_AND");
"!"				          		TOKEN("L_NOT");
"||"		          				TOKEN("OR");
"&&"		          				TOKEN("AND");

"++"		          				TOKEN("INCR");
"--"		          				TOKEN("DECR");

"+="		          				TOKEN("ASSIGN_PLUS");
"-="		          				TOKEN("ASSIGN_MINUS");
"*="		          				TOKEN("ASSIGN_MUL");
"/="		          				TOKEN("ASSIGN_DIV");
"%="		          				TOKEN("ASSIGN_MOD");

"~"		          				TOKEN("TILDE");

"for"                       TOKEN("FOR");
"while"                       TOKEN("WHILE");
"do"                       TOKEN("DO");
"if"                       TOKEN("IF");
"else"                       TOKEN("ELSE");
"elif"                       TOKEN("ELIF");
"switch"                       TOKEN("SWITCH");
"case"                       TOKEN("CASE");
"default"                       TOKEN("DEFAULT");
"try"                       TOKEN("TRY");
"catch"                       TOKEN("CATCH");
"break"                       TOKEN("BREAK");
"continue"                       TOKEN("CONTINUE");
"return"				        TOKEN("RETURN");
"include"                       TOKEN("INCLUDE");

"void"				        TOKEN("VOID");

"extern"                        TOKEN("EXTERN");
"define"                        TOKEN("DEFINE");
"undef"                        TOKEN("UNDEF");
"ifdef"				        TOKEN("IFDEF");


[a-zA-Z_][a-zA-Z0-9_]*   TOKEN("IDENTIFIER");
[0-9]+					       TOKEN("INTEGER");
-?(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)     TOKEN("DOUBLE");
\"[^"\n]+\"                     TOKEN("STR");
\'[^"\n]{1,2}\'                     TOKEN("CHR");

^"#include"
<S_INCLUDE>[ \t]*      /* eat the whitespace */
<S_INCLUDE>"<"[^>\n]+">" {    include_file();   }
<S_INCLUDE>\"[^"\n]+\" {    include_file();   }
<S_INCLUDE>\n { BEGIN 0;}

"#include"               define();
"#define"             define();
"#undef"                define();
"#ifdef"               define();
"#ifndef"             define();
"#else"                define();
"#endif"               define();

.                            yytext[0];


%%
int main(int argc, char **argv)
{
	if(argc<=2){
        printf("Usage: compile -l file1 file2 ... \n");
        exit(0);
	}

    inputs = &argv[2];
    yyin = fopen(inputs[0], "r");


    yylex();

}

void yyerror(const char *s)
{

    std::printf("[File:%s @ Line:%d]: %s. \n", inputs[0], lineno, s);
}