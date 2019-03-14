#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "utils/hash.h"
#include "expr.h"
#include "stmt.h"


stmt* new_base_stmt(int type, void *arg, stmt *body){
    stmt *e = (stmt *)malloc(sizeof(stmt));
    e->type = type;
    e->next = NULL;

    switch (type){
        case WHILE_STMT:
        case DO_STMT: {
            e->body = body;
            e->e = (expr *) arg;
            break;
        }
        case EXPR_STMT:
        case RETURN_STMT: {
            e->e = (expr *) arg;
        }
    }

    return e;

}

if_stmt* new_ifelse_stmt( expr *e,stmt *body1, stmt *body2 ){
    if_stmt *s = (if_stmt *)malloc(sizeof(if_stmt));
    s->type = IF_STMT;
    s->e = e;
    s->body1 = body1;
    s->body2 = body2;
    return s;

}

for_stmt* new_forloop_stmt(stmt *s1,stmt *s2, expr *s3, stmt *body){
    for_stmt *s = (for_stmt *)malloc(sizeof(for_stmt));
    s->type = FOR_STMT;
    s->s1 = s1;
    s->s2 = s2;
    s->s3 = s3;
    s->body = body;
    return s;

}

void check_return_type(symbol *sym, stmt *stmt){

}

void walk_stmt( stmt *s )
{
//    return;
    switch (s->type){
        case EXPR_STMT:
        case RETURN_STMT:
            Link *t = type_expr(s->e);
            if(t) {
                printf("Expression at line %d has type %s.\n", lineno, type_string(t));
            }
    }

}