#ifndef __STMT_H
#define __STMT_H

#include "utils/hash.h"
#include "symtab.h"
#include "expr.h"

#ifdef ALLOC                  /* Allocate variables if ALLOC defined. */
#   define ALLOC_CLS /* empty */
#else
#   define ALLOC_CLS   extern
#endif

typedef struct stmt                /* Symbol-table entry for vars and functions.	      */
{
    unsigned type;
    struct stmt *next;

    union {
        expr *e;                   /*for: CASE statement, RETURN statment*/
        symbol *sym;               /*for: labeled_statement*/
    };
    struct stmt *body;

} stmt;

typedef struct if_stmt                /* Symbol-table entry for vars and functions.	      */
{
    unsigned type;
    stmt *next;

    expr *e;
    stmt *body1;
    stmt *body2;


} if_stmt;

typedef struct for_stmt                /* Symbol-table entry for vars and functions.	      */
{
    unsigned type;
    stmt *next;

    stmt *s1;
    stmt *s2;
    expr *s3;
    stmt *body;


} for_stmt;

stmt* new_base_stmt(int type, void *arg, stmt *body);
if_stmt* new_ifelse_stmt( expr *e,stmt *body1, stmt *body2 );
for_stmt* new_forloop_stmt(stmt *s1,stmt *s2, expr *s3, stmt *body);

void check_return_type(symbol *sym, stmt *stmt);
void walk_stmt( stmt *s );

#endif /* __STMT_H */

