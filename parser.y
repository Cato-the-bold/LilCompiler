%{
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <string>
#include <cstring>

#include "utils/hash.h"
#include "symtab.h"
#include "expr.h"
#include "stmt.h"

using namespace std;

extern "C" int yylex();

extern void yyerror(const char *s);


int CONTEXT_LEVEL = 0;

symbol *cross_link = NULL;


%}

%define parse.error verbose

%union {
	int token;
	char *text;

    symbol *sym;
    Link *l;

    expr *e;
    stmt *s;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */

%token <text> STR CHR CONST_INT CONST_FLOAT IDENTIFIER
%token <token> V C S I L F D
%token <token> ASSIGN EQ NE LT LE GT GE
%token <token> OR AND L_AND L_OR L_NOT L_XOR NEGATION
%token <token> PLUS MINUS MUL DIV MOD PTR
%token <token> SHIFT_L SHIFT_R
%token <token> INCR DECR  ASSIGN_PLUS ASSIGN_MINUS ASSIGN_MUL ASSIGN_DIV ASSIGN_MOD
%token <token> FOR WHILE DO IF ELSE ELIF BREAK CONTINUE RETURN INCLUDE SWITCH CASE DEFAULT TRY CATCH
%token <token> AUTO CONSTANT RESTRICT VOLATILE REGISTER EXTERN DEFINE UNDEF IFDEF STATIC UNSIGNED SIGNED SIZEOF ELLIPSIS MEMBER
%token <token> TYPEDEF ENUM UNION STRUCT CLASS

%type <sym>  translation_unit external_declaration declaration_list declaration function_definition init_declarator_list init_declarator
%type <sym>  declarator direct_declarator parameter_type_list parameter_list parameter_declaration
%type <sym>  identifier_list
%type <l> declaration_specifiers pointer
%type <token> storage_class_specifier type_specifier type_qualifier type_qualifier_list unary_operator assignment_operator
%type <s> statement labeled_statement compound_statement statement_list expression_statement selection_statement
%type <s> iteration_statement jump_statement
%type <e>  initializer_list initializer expression assignment_expression conditional_expression _expression
%type <e> constant_expression primary_expression postfix_expression unary_expression cast_expression argument_expression_list

/* Operator precedence for mathematical operators */
%left	OR			         /*	||				    */
%left	AND			         /*	&&				    */
%left	L_OR			     /*	|				    */
%left   L_XOR			     /*	^				    */
%left	L_AND			     /*	&				    */
%left	EQ NE		         /*	==  !=				    */
%left	LT LE GT GE		     /*	<=  >= <  >			    */
%left	SHIFT_L SHIFT_R      /*	>> <<				    */
%left	PLUS  MINUS		     /*	+ -				    */
%left	MUL DIV MOD	         /*	  *  /   %			    */

%nonassoc UPLUS UMINUS

%nonassoc ELSE		/* This gives a high precedence to ELSE to suppress
			 * the shift/reduce conflict error message in:
			 *   s -> IF LP expr RP expr | IF LP expr RP s ELSE s
			 * The precedence of the first production is the same
			 * as RP. Making ELSE higher precedence forces
			 * resolution in favor of the shift.
			 */

%start translation_unit

%%

translation_unit : external_declaration { cross_link = $$ = $1; }
        |  translation_unit  external_declaration {  if($2){
                                                        symbol *end = $2; while(end->next) end = end->next;
                                                        end->next = $1; cross_link = $2;
                                                     }
                                                  }
		;

external_declaration
	: declaration
	| function_definition

declaration
	: declaration_specifiers ';'        { printf("TODO:\n"); $$ = NULL; }
	| declaration_specifiers init_declarator_list ';'  {  if($2){ $1->clazz = SPECIFIER; add_spec_to_decl( $1, $2 ); add_symbols_to_table($2); $$ = $2;}
	                                                      else { $$ = NULL; } }
	;

declaration_specifiers
	: storage_class_specifier                                {  $$ = new_link(); $$->SCLASS=$1; }
	| storage_class_specifier declaration_specifiers         {  $$ = $2; $$->SCLASS=$1; }
	| type_specifier                                         {  $$ = append_type_specifier(new_link(), $1); }
	| type_specifier declaration_specifiers                  {  $$ = append_type_specifier($2, $1); }
	| type_qualifier                                         {  $$ = append_type_qualifier(new_link(), $1); }
	| type_qualifier declaration_specifiers                  {  $$ = append_type_qualifier($2, $1); }
	;


storage_class_specifier
	: TYPEDEF      { $$=SC_TYPEDEF; }
	| EXTERN       { $$=SC_EXTERN; }
	| STATIC       { $$=SC_STATIC; }
	| AUTO         { $$=SC_AUTO; }
	| REGISTER     { $$=SC_REGISTER; }
	;

type_specifier
	: V            { $$=VOID; }
	| C            { $$=CHAR; }
	| S            { $$=SHORT; }
	| I            { $$=INT; }
	| L            { $$=LONG; }
	| F            { $$=FLOAT; }
	| D            { $$=DOUBLE; }
	| SIGNED
	| UNSIGNED
    ;

type_qualifier
	: CONSTANT
	| RESTRICT    /* Not supported. */
	| VOLATILE    /* Not supported. */
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' init_declarator   {  if($3){
	                                                    ($$ = $3)->next = $1;
	                                                }
	                                             }
	;

init_declarator
	: declarator
	| declarator ASSIGN initializer    {  $$ = $1; $$->init_val = $3; }
	;

declarator
	: pointer direct_declarator  {  add_spec_to_decl($1, $$ = $2); }
	| direct_declarator          {  $$ = $1; }
	;

direct_declarator
	: IDENTIFIER                                            { $$ = new_var_decl($1);  }
	| '(' declarator ')'                                    { $$ = $2;}
	| direct_declarator '[' CONST_INT ']'                   { add_declarator( $1, ARRAY ); $1->etype->NUM_ELE = atoi($3); $$ = $1;/*TODO: num of elements*/}
	| direct_declarator '[' ']'                             { add_declarator( $1, ARRAY ); $1->etype->NUM_ELE = 0; $$ = $1; }
	| direct_declarator '(' parameter_type_list ')'         { add_declarator( $1, FUNCTION ); $$->args = $3; $$ = $1;  }
	| direct_declarator '(' identifier_list ')'             { add_declarator( $1, FUNCTION ); $$ = $1; }
	| direct_declarator '(' ')'                             { add_declarator( $1, FUNCTION ); $$ = $1; }
	;

pointer
	: MUL                                { $$ = new_link(); $$->DCL_TYPE = POINTER; }
	| MUL type_qualifier_list            { $$ = new_link(); $$->DCL_TYPE = POINTER; $$->PTR_QUA = $2;}
	| MUL pointer                        { $$ = new_link(); $$->DCL_TYPE = POINTER; $$->next = $2; }
	| MUL type_qualifier_list pointer    { $$ = new_link(); $$->DCL_TYPE = POINTER; $$->PTR_QUA = $2; $$->next = $3; }
	;

type_qualifier_list   //TODO:
	: type_qualifier
	| type_qualifier_list type_qualifier {$$ = $2;}
	;

initializer
	: primary_expression
	| '{' initializer_list '}'           {/*TODO: */ $$ = $2;}
	| '{' initializer_list ',' '}'       {/*TODO: */ $$ = $2;}
	;

initializer_list   //TODO:
	: initializer
	| initializer_list ',' initializer
	;

parameter_type_list
	: parameter_list
	| parameter_list ',' ELLIPSIS   {/*TODO: */  $$ = $1; }
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration    {  ($$ = $3)->next = $1; }
	;

parameter_declaration
	: declaration_specifiers declarator  {  $1->clazz = SPECIFIER; add_spec_to_decl( $1, $$ = $2 );  }
	| declaration_specifiers             {  /*TODO:*/ }
	;

identifier_list
	: IDENTIFIER
	| identifier_list ',' IDENTIFIER
	;

function_definition
	: declaration_specifiers declarator { $1->clazz = SPECIFIER; add_spec_to_decl( $1, $2); CONTEXT_LEVEL++;
	                                      add_symbols_to_table($2->args); CONTEXT_LEVEL--; }
	            compound_statement      { remove_symbols_from_table ( $2->args ); add_symbols_to_table($2); check_return_type($2,$4); $$ = $2; }
	| declarator compound_statement     //TODO:
	;

declaration_list
	: declaration
	| declaration_list  declaration      {  if($2)($$ = $2)->next = $1; }
	;

statement_list
	: statement                   { walk_stmt($1);   $$ = $1; }
	| statement_list statement    { walk_stmt($2);   if($2){$2->next = $1; $$ = $2;}  }
	;

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement  //TODO:
	: IDENTIFIER ':' statement
	| CASE constant_expression ':' statement
	| DEFAULT ':' statement
	;

compound_statement
    : '{' { CONTEXT_LEVEL++;}  statement_list '}'  {CONTEXT_LEVEL--; $$ = $3; }
	| '{' { CONTEXT_LEVEL++;}  declaration_list  statement_list '}' {CONTEXT_LEVEL--; remove_symbols_from_table($3); $$ = $4; }
	;

selection_statement //TODO:
	: IF '(' expression ')' statement
	| IF '(' expression ')' statement ELSE statement
	| SWITCH '(' expression ')' statement
	;

iteration_statement //TODO:
	: WHILE '(' expression ')' statement
	| DO statement WHILE '(' expression ')' ';'
	| FOR '(' expression_statement expression_statement ')' statement
	| FOR '(' expression_statement expression_statement expression ')' statement
	;

jump_statement
	: CONTINUE ';'             { $$ = new_base_stmt(CONTINUE_STMT, 0, 0); }
	| BREAK ';'                { $$ = new_base_stmt(BREAK_STMT, 0, 0); }
	| RETURN ';'               { $$ = new_base_stmt(RETURN_STMT, 0, 0); }
	| RETURN expression ';'    { $$ = new_base_stmt(RETURN_STMT, $2, 0); }
	;

expression_statement
	: ';'   { $$ = NULL; }
	| expression ';'  { $$ = new_base_stmt(EXPR_STMT, $1, 0); }
	;

expression
	: assignment_expression
	| expression ',' assignment_expression        { $1->next = $3;  $$=$1; }
	;

assignment_expression
	: conditional_expression
	| unary_expression assignment_operator assignment_expression   { $$ = new_assign_expr($2, $1, $3); }
	;

assignment_operator
    : ASSIGN
    | ASSIGN_PLUS
    | ASSIGN_MINUS
    | ASSIGN_MUL
    | ASSIGN_DIV
    | ASSIGN_MOD
    ;

conditional_expression
	: _expression
	| _expression '?' expression ':' conditional_expression        { $$ = new_cond_expr($1, $3, $5); }
	;

_expression
    : unary_expression
    | _expression OR _expression             { $$ = new_binary_expr(OR, $1, $3); }
    | _expression AND _expression            { $$ = new_binary_expr(AND, $1, $3); }

    | _expression L_OR _expression           { $$ = new_binary_expr(L_OR, $1, $3); }
    | _expression L_AND _expression          { $$ = new_binary_expr(L_AND, $1, $3); }

    | _expression NE _expression             { $$ = new_binary_expr(NE, $1, $3); }

    | _expression LT _expression             { $$ = new_binary_expr(LT, $1, $3); }
    | _expression LE _expression             { $$ = new_binary_expr(LE, $1, $3); }
    | _expression GT _expression             { $$ = new_binary_expr(GT, $1, $3); }
    | _expression GE _expression             { $$ = new_binary_expr(GE, $1, $3); }

    | _expression SHIFT_L _expression        { $$ = new_binary_expr(SHIFT_L, $1, $3); }
    | _expression SHIFT_R _expression        { $$ = new_binary_expr(SHIFT_R, $1, $3); }

    | _expression PLUS _expression           { $$ = new_binary_expr(PLUS, $1, $3); }
    | _expression MINUS _expression          { $$ = new_binary_expr(MINUS, $1, $3); }

    | _expression MUL _expression            { $$ = new_binary_expr(MUL, $1, $3); }
    | _expression DIV _expression            { $$ = new_binary_expr(DIV, $1, $3); }
    | _expression MOD _expression            { $$ = new_binary_expr(MOD, $1, $3); }
    ;

constant_expression
	: IDENTIFIER            { $$ = new_primary_expr(E_ID, $1); }
	| CONST_INT             { $$ = new_primary_expr(INT, $1); }
	| CONST_FLOAT           { $$ = new_primary_expr(FLOAT, $1); }
	| CHR                   { $$ = new_primary_expr(CHAR, $1); }
   	| STR                   { $$ = new_primary_expr(E_STRING, $1); }
	;

primary_expression
    : constant_expression
	| '(' expression ')'    {  $$ = $2; }
	;

postfix_expression
	: primary_expression
	| postfix_expression '[' expression ']'                 { $$ = new_postfix_expr(ARRAY,$1,$3); }
	| postfix_expression '(' ')'                            { $$ = new_postfix_expr(FUNCTION,$1,NULL); }
	| postfix_expression '(' argument_expression_list ')'   { $$ = new_postfix_expr(FUNCTION,$1,$3);   }
	| postfix_expression MEMBER IDENTIFIER                  { $$ = new_postfix_expr(MEMBER,$1,$3); }
	| postfix_expression PTR IDENTIFIER                     { $$ = new_postfix_expr(PTR,$1,$3); }
	| postfix_expression INCR                               { $$ = new_postfix_expr(INCR,$1,0); }
	| postfix_expression DECR                               { $$ = new_postfix_expr(DECR,$1,0); }
	;

unary_expression
	: postfix_expression
	| INCR unary_expression                 { $$ = new_unary_expr(INCR,$2); }
	| DECR unary_expression                 { $$ = new_unary_expr(DECR,$2); }
    | unary_operator cast_expression        { $$ = new_unary_expr($1,$2); }
	| SIZEOF unary_expression               { $$ = new_unary_expr(SIZEOF,$2); }
	| SIZEOF '(' declaration_specifiers ')'
	;

unary_operator
	: L_AND      /* & */
	| MUL        /* * */
	| PLUS
	| MINUS
	| NEGATION   /* ~ */
	| L_NOT      /* ! */
	;

cast_expression
	: unary_expression
	| '(' declaration_specifiers ')' cast_expression  {  $$ = new_cast_expr($2,$4); }
	;

argument_expression_list
	: assignment_expression
	| assignment_expression  ','  argument_expression_list  { $1->next = $3; $$ = $1; }
	;

;


%%