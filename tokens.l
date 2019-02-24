%{
#include <string>

#include "node.h"
#include "parser.tab.h"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    ( yylval.token = t )

extern char **inputs;

extern "C" {
    int yywrap() {
        inputs+=1;
        if(inputs[0]){
            yyin = fopen(inputs[0], "r");
            return 0;
        }else{
            return 1;
        }
    }
}
extern "C" int yylex();
extern "C" void yyerror(const char *s);

int lineno = 1;

int PRINT_LEXIME = 0;

%}


%s COMMENT
%s COMMENT_

%%

[\t ]                         ;
\n                              lineno++;


"/*"[^*]*"*/"                ;

"//"[^\n]*                  ;


"="						          return TOKEN(ASSIGN);
"=="				          	return TOKEN(EQ);
"!="			          		return TOKEN(NE);
"<"				          		return TOKEN(LT);
"<="	          				return TOKEN(LE);
">"				          		return TOKEN(GT);
">="					          return TOKEN(GE);

"+"				          		return TOKEN(PLUS);
"-"		          				return TOKEN(MINUS);
"*"		          				return TOKEN(MUL);
"/"				          		return TOKEN(DIV);
"%"				          		return TOKEN(MOD);
"|"		          				return TOKEN(L_OR);
"&"				          		return TOKEN(L_AND);
"!"				          		return TOKEN(L_NOT);
"||"		          				return TOKEN(OR);
"&&"		          				return TOKEN(AND);

"++"		          				return TOKEN(INCR);
"--"		          				return TOKEN(DECR);

"+="		          				return TOKEN(ASSIGN_PLUS);
"-="		          				return TOKEN(ASSIGN_MINUS);
"*="		          				return TOKEN(ASSIGN_MUL);
"/="		          				return TOKEN(ASSIGN_DIV);
"%="		          				return TOKEN(ASSIGN_MOD);

"~"		          				return TOKEN(TILDE);

"for"                       return TOKEN(FOR);
"while"                       return TOKEN(WHILE);
"do"                       return TOKEN(DO);
"if"                       return TOKEN(IF);
"else"                       return TOKEN(ELSE);
"elif"                       return TOKEN(ELIF);
"switch"                       return TOKEN(SWITCH);
"case"                       return TOKEN(CASE);
"default"                       return TOKEN(DEFAULT);
"try"                       return TOKEN(TRY);
"catch"                       return TOKEN(CATCH);
"break"                       return TOKEN(BREAK);
"continue"                       return TOKEN(CONTINUE);
"return"				        return TOKEN(RETURN);
"include"                       return TOKEN(INCLUDE);

"void"				        return TOKEN(VOID);

"extern"                        return TOKEN(EXTERN);
"define"                        return TOKEN(DEFINE);
"undef"                        return TOKEN(UNDEF);
"ifdef"				        return TOKEN(IFDEF);


[a-zA-Z_][a-zA-Z0-9_]*  {SAVE_TOKEN;  if(PRINT_LEXIME) printf("%s [string]\n",yytext);  return IDENTIFIER;}
[0-9]+					        {SAVE_TOKEN;  if(PRINT_LEXIME) printf("%s [int]\n",yytext); return INTEGER;}
-?(([0-9]+)|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?)       {SAVE_TOKEN;  if(PRINT_LEXIME) printf("%s [double]\n",yytext); return DOUBLE;}
\"([^"\n]*)\"                     {SAVE_TOKEN; return STR;}
\'([^"\n])\'                     {SAVE_TOKEN; return CHR;}

.                            return yytext[0];


%%

void yyerror(const char *s)
{

    std::printf("[File:%s @ Line:%d]: %s. \n", inputs[0], lineno, s);
}