%{
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <string>
#include <stack>
#include <unordered_map>

#include "symtab.h"
#include "expr.h"
#include "stmt.h"
#include "parser.tab.h"

#define DEBUG 0

extern char **inputs;

extern "C" int yylex();
void yyerror(const char *s);

#define SAVE_TOKEN  yylval.text = strdup(yytext);

using namespace std;

int lineno = 1;

int include_stack_ptr = 0;

const int MAX_INCLUDE_DEPTH = 256;
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];

int TOKEN(int t){
    if(DEBUG){
        printf("File %s Line %d Token '%d' Text '%s'\n", inputs[0], lineno, t, yytext);
    }
    return yylval.token = t;
}

void include_file();

void undefine();

%}

Digit			[0-9]
Letter			[a-zA-Z_]
Alpha           [_a-zA-Z0-9]
Hex			    [a-fA-F0-9]
Oct             [0-7]
FS			    (f|F|l|L)
IS			    (u|U|l|L)*

%s S_INCLUDE
%s S_DEFINE
%s S_DEFINE_2
%s S_UNDEF

%%

[\t\v\f ]                    ;
\n                           lineno++;

"/*"[^*]*"*/"                ;

"//"[^\n]*                   ;

"="						        return TOKEN(ASSIGN);
"=="				          	return TOKEN(EQ);
"!="			          		return TOKEN(NE);
"<"				          		return TOKEN(LT);
"<="	          				return TOKEN(LE);
">"				          		return TOKEN(GT);
">="					        return TOKEN(GE);

"||"		          		    return TOKEN(OR);
"&&"		          			return TOKEN(AND);

"++"		          			return TOKEN(INCR);
"--"		          			return TOKEN(DECR);

"+="		          			return TOKEN(ASSIGN_PLUS);
"-="		          			return TOKEN(ASSIGN_MINUS);
"*="		          			return TOKEN(ASSIGN_MUL);
"/="		          			return TOKEN(ASSIGN_DIV);
"%="		       				return TOKEN(ASSIGN_MOD);

"->"		          			return TOKEN(PTR);
">>"		          			return TOKEN(SHIFT_L);
"<<"		          			return TOKEN(SHIFT_R);
"..."			                return(ELLIPSIS);
"."                             return(MEMBER);

"+"				          		return TOKEN(PLUS);
"-"		          				return TOKEN(MINUS);
"*"		          				return TOKEN(MUL);
"/"				          		return TOKEN(DIV);
"%"				          		return TOKEN(MOD);
"|"		          				return TOKEN(L_OR);
"&"				          		return TOKEN(L_AND);
"!"				          		return TOKEN(L_NOT);
"~"				          		return TOKEN(NEGATION);
"^"		          				return TOKEN(L_XOR);

"for"                           return TOKEN(FOR);
"while"                         return TOKEN(WHILE);
"do"                            return TOKEN(DO);
"if"                            return TOKEN(IF);
"else"                          return TOKEN(ELSE);
"elif"                          return TOKEN(ELIF);
"switch"                        return TOKEN(SWITCH);
"case"                          return TOKEN(CASE);
"default"                       return TOKEN(DEFAULT);
"try"                           return TOKEN(TRY);
"catch"                         return TOKEN(CATCH);
"break"                         return TOKEN(BREAK);
"continue"                      return TOKEN(CONTINUE);
"return"				        return TOKEN(RETURN);
"include"                       return TOKEN(INCLUDE);

"auto"				            return TOKEN(AUTO);
"const"				            return TOKEN(CONSTANT);
"volatile"				        return TOKEN(VOLATILE);
"restrict"				        return TOKEN(RESTRICT);
"register"				        return TOKEN(REGISTER);

"define"                        return TOKEN(DEFINE);
"undef"                         return TOKEN(UNDEF);
"ifdef"				            return TOKEN(IFDEF);

"extern"                        return TOKEN(EXTERN);
"static"                        return TOKEN(STATIC);
"unsigned"                      return TOKEN(UNSIGNED);
"signed"                        return TOKEN(SIGNED);
"sizeof"                        return TOKEN(SIZEOF);

"void"				            return TOKEN(V);
"char"				            return TOKEN(C);
"short"			                return TOKEN(S);
"int"				            return TOKEN(I);
"float"				            return TOKEN(F);
"double"				        return TOKEN(D);
"long"				            return TOKEN(L);

"typedef"		                return TOKEN(TYPEDEF);
"enum"				            return TOKEN(ENUM);
"union"				            return TOKEN(UNION);
"struct"				        return TOKEN(STRUCT);
"class"				            return TOKEN(CLASS);


{Letter}{Alpha}*                             {SAVE_TOKEN;  if(DEBUG) printf("%s [id]\n",yytext);  return IDENTIFIER;}
[0-9]+					                                {SAVE_TOKEN;  if(DEBUG) printf("%s [int]\n",yytext); return CONST_INT;}
-?(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)         {SAVE_TOKEN;  if(DEBUG) printf("%s [double]\n",yytext); return CONST_FLOAT;}
\"([^"\n]*)\"                                           {SAVE_TOKEN;  if(DEBUG) printf("%s [string]\n",yytext); return STR;}
\'[^"\n]{1,2}\'                                         {SAVE_TOKEN;  if(DEBUG) printf("%s [char]\n",yytext); return CHR;}


^"#include"
<S_INCLUDE>[ \t]*           /* eat the whitespace */
<S_INCLUDE>"<"[^>\n]+">"    {    include_file();   }
<S_INCLUDE>\"[^"\n]+\"      {    include_file();   }
<S_INCLUDE>\n               { BEGIN 0;}


"#define"             undefine();
"#undef"              undefine();
"#ifdef"              undefine();
"#ifndef"             undefine();
"#else"               undefine();
"#endif"              undefine();

.                     return yytext[0];


%%


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

void undefine(){
    printf("Error in %s line %d:\n\t directive ’%s’ not implemented, ignoring\n", inputs[0], lineno, yytext);

    char c;
    while ((c = yyinput()) != '\n' && c != 0);
    if(c=='\n'){
        unput(c);
    }
}


void yyerror(const char *s)
{

    std::printf("[File:%s @ Line:%d]: %s. \n", inputs[0], lineno, s);
}

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


/**
comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		putchar(c);

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		putchar(c1);
}
*/