#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "symtab.h"
#include "expr.h"
#include "stmt.h"
#include "utils/hash.h"
#include "parser.tab.h"

expr* new_primary_expr(int type, char *yytext){
    expr *e = (expr *)malloc(sizeof(expr));
    e->next = NULL;
    e->type = type;

    symbol *s = NULL;

    switch(type) {
        case E_ID: {
            s = (symbol *) findsym(Symbol_tab, yytext);
            if(!s)e->const_val = (void*)yytext;
            else e->sym = s;

            break;
        }
        case CHAR: {
            e->const_val = strdup(yytext);
            e->spec = &L_CHAR;
            break;
        }
        case INT: {
            e->const_val = strdup(yytext);
            e->spec = &L_INT;
            break;
        }
        case FLOAT: {
            e->const_val = strdup(yytext);
            e->spec = &L_FLOAT;
            break;
        }
        case E_STRING: {
            e->const_val = strdup(yytext);
            e->spec = &L_STRING;
            break;
        }
    }

    return e;
}

expr* new_postfix_expr(int op, expr *e1, void *arg){
    expr *e = (expr *)malloc(sizeof(expr));
    e->type = POST_OP;
    e->op = op;
    e->next = NULL;

    symbol *s = NULL;

    switch (op){
        case ARRAY:
        case FUNCTION: {
            e->e1 = e1;
            e->e2 = (expr *) arg;
            break;
        }
        case MEMBER:
        case PTR: {
            e->e1 = e1;

            s = (symbol *) findsym(Symbol_tab, (char *) arg);
            e->sym = s;
            break;
        }
    }

    return e;
}

expr* new_unary_expr(int op, expr *e1){
    expr *e = (expr *)malloc(sizeof(expr));
    e->type = UNARY_OP;
    e->next = NULL;
    e->op = op;
    e->e1 = e1;
    return e;
}

expr *new_binary_expr(int op, expr *e1, expr *e2){
    expr *e = (expr *)malloc(sizeof(expr));
    e->type = BINARY_OP;
    e->next = NULL;
    e->op = op;
    e->e1 = e1;
    e->e2 = e2;
    return e;
}

expr *new_assign_expr(int op, expr *e1, expr *e2){
    expr *e = (expr *)malloc(sizeof(expr));
    e->type = E_ASSIGN;
    e->next = NULL;
    e->op = op;
    e->e1 = e1;
    e->e2 = e2;
    return e;
}

expr* new_cast_expr(Link *spec, expr *e1){
    expr *e = (expr *)malloc(sizeof(expr));
    e->type = E_CAST;
    e->next = NULL;
    e->spec = spec;
    e->e1 = e1;
    return e;
}

expr *new_cond_expr(expr* cond, expr *left, expr *right){
    cond_expr *e = (cond_expr *)malloc(sizeof(cond_expr));
    e->type = TERNARY;
    e->next = NULL;

    e->cond = cond;
    e->left = left;
    e->right = right;

    return (expr*)e;
}

void free_expr(expr *e){
    free(e);
}

Link* type_coercion(Link *_p1, Link *_p2) {
    Link *p1 = _p1;
    Link *p2 = _p2;
    for (; p1 && p2; p1 = p1->next, p2 = p2->next) {
        if (p1->clazz != p2->clazz)
            return 0;

        if (p1->clazz == DECLARATOR) { //both are complex types.
            if ((p1->DCL_TYPE != p2->DCL_TYPE) ||
                (p1->DCL_TYPE == ARRAY && (p1->NUM_ELE != p2->NUM_ELE)))
                return 0;
        } else {   //both are basic types.
            if (CHAR<=p1->TYPE && p1->TYPE<=DOUBLE && CHAR<=p2->TYPE && p1->TYPE<=DOUBLE){
                return p1->TYPE > p2->TYPE? _p1:_p2;
            }
            printf("Not supported!");
            return 0;
        }
    }
    return 0;
}

char* const op_string(int op){
    switch (op){
        case L_AND:    return "&";
        case MUL:    return "*";
        case PLUS:  return "+";
        case MINUS:  return "-";
        case NEGATION:  return "~";
        case L_NOT: return "!";
        case INCR: return "++";
        case DECR: return "--";
    }
}

Link* unary_expr_coercion(int op, Link *ul){
    if(!ul)return NULL;

    switch (op) {
        case L_AND: {
            Link *link_p = new_link();
            link_p->clazz = DECLARATOR;
            link_p->DCL_TYPE = POINTER;
            link_p->next = ul;
            return link_p;
        }
        case MUL: {
            if(IS_SPECIFIER(ul)){
                return NULL;
            }
            if (IS_ARRAY(ul) || IS_POINTER(ul))
                return ul->next;
            printf("Error in %s line %d:\n\tUnary operator '%s' cannot be applied to expression of type %s.\n",
                   inputs[0], lineno, op_string(op), type_string(ul));
            return NULL;
        }
        case PLUS: case MINUS: case NEGATION: case L_NOT: {
            if (IS_ARRAY(ul) || IS_POINTER(ul)) {
                printf("Error in %s line %d:\n\tUnary operator '%s' cannot be applied to expression of type %s.\n",
                       inputs[0], lineno, op_string(op), type_string(ul));
                return NULL;
            }
            return ul;
        }
        default:
            return ul;
    }
}

Link* type_expr( expr *e )
{
    switch (e->type){
        case INT: case FLOAT: case SHORT: case LONG:
        case CHAR: case E_STRING: {
            char *t = type_string(e->spec);
//            printf("[const] type:%s value:%s\n", t, e->const_val); free(t);
            return e->spec;
        }
        case E_ID: {
//            printf("[id] ");
//            print_sym(e->sym);
            return e->sym?e->sym->type:NULL;
        }
        case BINARY_OP: {
//            printf("[binary expr op:%d \n", e->op);
            Link *l1 = type_expr(e->e1);
            Link *l2 = type_expr(e->e2);

            Link *t = type_coercion(l1 ,l2);
            t = unary_expr_coercion(e->op, t);
            return t;
        }
        case POST_OP: {
            Link *l = type_expr(e->e1);
            switch (e->op) {
                case FUNCTION:{
//                    printf("[function call. Args:\n", e->op);
                    symbol *f = e->e1->sym;
                    if(!f){
                        printf("Error in %s line %d:\n\tNo match for function call %s(...)\n\tNo functions with this name.\n",
                               inputs[0], lineno, (char*)e->e1->const_val );
                        return NULL;
                    }
                    symbol *args = f->args;
                    expr *_e = e->e2;

                    while (args && _e) {
                        Link *l = type_expr(_e);
                        if(args->type != type_coercion(l, args->type)){
                            goto FAIL;
                        }

                        _e = _e->next;
                        args = args->next;
                    }

                    if(args || _e){
FAIL:                   printf("Error in %s line %d:\n\tNo match for function call %s(...)\n\tCandidates are :\n\t\t%s declared in %s near line %d.\n",
                               inputs[0], lineno, f->name, func_signature(f), inputs[0], f->defined_line);
                        return NULL;
                    }
                }
                case ARRAY:{
                    return l->next;
                }
                case INCR: case DECR: {
                    Link *t = type_coercion(l , &L_INT);
                    return t;
                }
            }
            return l;
        }
        case UNARY_OP: {
//            printf("[unary expr op:%d ", e->op);
            Link *ul = type_expr(e->e1);
            return unary_expr_coercion(e->op, ul);

        }
        case TERNARY: {
//            printf("[conditional expr ");
//            printf("\tcond: ");
            Link *_l1 = type_expr(((cond_expr *) e)->cond);

//            printf("\tleft: ");
            Link *_l2 = type_expr(((cond_expr *) e)->left);

//            printf("\tright: ");
            Link *_l3 = type_expr(((cond_expr *) e)->right);

            return _l2;
        }
        case E_ASSIGN: {
            Link *p1 = type_expr(e->e1);
            Link *p2 = type_expr(e->e2);

            if(!p1 || !p2) return NULL;

            if(e->op==ASSIGN){
                return type_coercion(p1 ,p2);
            }else{ /* +=,-=,*=,/=,%=  */
                if(IS_SPECIFIER(p2) && CHAR<=p2->TYPE && p2->TYPE<=DOUBLE){
                    if(IS_SPECIFIER(p1) && CHAR<=p1->TYPE && p1->TYPE<=DOUBLE){
                        return p1->TYPE < p2->TYPE? p2:p1;
                    }else if(IS_POINTER(p1)||IS_ARRAY(p1)){
                        return p1;
                    }
                    return NULL;
                }else{
                    return NULL;
                }
            }

        }
        case E_CAST:
            return e->spec;
    }

}

void check_function_call(expr *e){
    symbol *func = e->e1->sym;
//    symbol *args = func->args;
//    expr *args2 = e->e2;



}