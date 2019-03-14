#ifndef __EXPR_H
#define __EXPR_H

#include "utils/hash.h"
#include "symtab.h"

#ifdef ALLOC                  /* Allocate variables if ALLOC defined. */
#   define ALLOC_CLS /* empty */
#else
#   define ALLOC_CLS   extern
#endif

typedef struct expr                /* Symbol-table entry for vars and functions.	      */
{
    unsigned type:5;
    short op;
    struct expr *next;

    union{
        struct expr *e1;
        void *const_val;
    };

    union {
        symbol *sym;
        struct expr *e2;
        link *spec;
    };

} expr;

typedef struct cond_expr                /* Symbol-table entry for vars and functions.	      */
{
    unsigned type:5;
    short op;
    struct expr *next;

    expr *cond;
    expr *left;
    expr *right;

} cond_expr;

expr* new_primary_expr(int type, char *yytext);
expr* new_postfix_expr(int op, expr *e1, void *arg);
expr* new_unary_expr(int op, expr *e1);
expr *new_binary_expr(int op, expr *e1, expr *e2);
expr *new_assign_expr(int op, expr *e1, expr *e2);
expr* new_cast_expr(Link *spec, expr *e1);
expr *new_cond_expr(expr* cond, expr *left, expr *right);
void check_function_call(expr *e);

Link* type_expr( expr *e );

#endif /* __EXPR_H */

