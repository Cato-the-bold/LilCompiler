%{
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <string>
#include <cstring>

#include "node.h"

using namespace std;

extern int lineno;

extern char **inputs;

extern "C" int yylex();
extern "C" void yyerror(const char *s);

extern Block *programBlock; /* the top level root node of our final AST */

//std::stack<Context> contexts;
//contexts.push(Context());

%}

%define parse.error verbose

%union {
	int token;
	Identifier *ident;
	std::string *string;

	Node *node;

	Expression *expr;
	Statement *stmt;
	Block *block;

	std::vector<Identifier*> *vars;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> IDENTIFIER INTEGER DOUBLE STR CHR
%token <token> ASSIGN EQ NE LT LE GT GE
%token <token> L_AND L_OR L_NOT OR AND TILDE
%token <token> PLUS MINUS MUL DIV MOD
%token <token> INCR DECR  ASSIGN_PLUS ASSIGN_MINUS ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD
%token <token> FOR WHILE DO IF ELSE ELIF BREAK CONTINUE RETURN INCLUDE VOID SWITCH CASE DEFAULT TRY CATCH
%token <token> EXTERN DEFINE UNDEF IFDEF

%type <ident> ident type var_init
%type <block> program  stmts  block  func_decl_args  _func_decl_args
%type <stmt> stmt  var_decl  func_decl  func_parameter func_definition  _stmt  if_else_stmt  for_stmt  while_stmt  do_stmt
%type <expr> expr
%type <vars> vars

/* Operator precedence for mathematical operators */
%left PLUS MINUS
%left MUL DIV MOD

%start program

%%

program : stmts { programBlock = $$ = $1; }
        |
		;

stmts : stmt { $$ = new Block(); $$->statements.push_back($<stmt>1); }
	  | stmts stmt { $$ = $1; $$->statements.push_back($<stmt>2); }
	  ;

stmt: _stmt ';'
     | if_else_stmt    { $$ = NULL; }
     | for_stmt { $$ = NULL; }
     | while_stmt { $$ = NULL; }
     | func_definition
     ;

_stmt :  /*empty*/ { $$ = NULL; }
     | BREAK { $$ = NULL; }
     | CONTINUE { $$ = NULL; }
	 | RETURN expr  { $$ = NULL; }
     | RETURN    { $$ = NULL; }

     | do_stmt { $$ = NULL; }

     | var_decl
     | func_decl

	 | expr_list { $$ = NULL; }
     ;

block : '{' stmts '}' { $$ = $2; }
	  | '{' '}' { $$ = new Block(); }
	  ;

type : ident
     ;

stmt2 : stmt
      | block
      ;

if_else_stmt : IF '(' expr ')' stmt2
        |   IF '(' expr ')' stmt2 ELSE stmt2
        ;

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt2
         ;

while_stmt : WHILE '(' expr ')' stmt2
           ;

do_stmt : DO stmt2  WHILE '(' expr ')'
        ;

var_decl : type vars { $$ = new VariableDeclaration($1, $2); }

vars : var_init  { $$ = new std::vector<Identifier*>(); $$->push_back($1);  }
		 | vars ',' var_init { $$ = $1; $$->push_back($3); }
		 ;

var_init : ident
         | ident ASSIGN expr {  /*TODO: this branch is now dismissed.*/ }
         ;

func_decl : type ident '(' func_decl_args ')'
			{ $$ = new Function($1, $2, $4, NULL); }
		  ;

func_decl_args : /*blank*/  { $$ = new Block(); }
          | VOID    { $$ = new Block(); }
		  | _func_decl_args

func_parameter : type var_init  { std::vector<Identifier*>* tmp = new std::vector<Identifier*>();
                                  tmp->push_back($2);
                                  $$ = new VariableDeclaration($1, tmp); }

_func_decl_args : func_parameter { $$ = new Block(); $$->statements.push_back($<stmt>1); }
		  | _func_decl_args ',' func_parameter {  $$ = $1; $$->statements.push_back($<stmt>3);}
		  ;

func_definition : func_decl block { $$ = $1; ((Function*)$$)->block = $2; programBlock->statements.push_back($$); }
		  ;

ident : IDENTIFIER  { $$ = new Identifier($1); }
	  ;

numeric : INTEGER
		| DOUBLE
		;

opt_expr : expr_list
    |
    ;

expr_list : expr
          | expr_list ',' expr
          ;

expr : numeric
     | ident
     | L_AND ident
     | ident '[' expr ']'
     | ident '(' call_args ')'

     | l_value_expr ASSIGN expr
     | l_value_expr INCR
     | l_value_expr DECR
     | INCR l_value_expr
     | DECR l_value_expr

     | MINUS expr
     | L_NOT expr
     | TILDE expr

     | expr binary_op expr

 	 | expr '?' expr ':' expr

 	 | '(' type ')' expr
     | '(' expr_list ')'
	 ;

l_value_expr : ident
         | ident '[' expr_list ']'
         ;

opt_call_args : /*blank*/
		  | call_args
		  ;

call_args : expr
		  | call_args ',' expr
		  ;

binary_op : EQ | NE | LT | LE | GT | GE |
            PLUS | MINUS | MUL | DIV | MOD |
            L_AND | L_OR | L_NOT | OR | AND |
            ASSIGN_PLUS | ASSIGN_MINUS | ASSIGN_MUL | ASSIGN_DIV | ASSIGN_MOD
;


%%

