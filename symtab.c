#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>

#include "utils/hash.h"
#include "symtab.h"       /* Symbol-table definitions.			      */
#include "expr.h"
#include "stmt.h"
#include "parser.tab.h"

extern int CONTEXT_LEVEL;

void yyerror(const char *s);

link L_CHAR = {
        1, 0, { CHAR }, NULL
};

link L_INT = {
        1, 0, { INT }, NULL
};

link L_LONG = {
        1, 0, { LONG }, NULL
};

link L_FLOAT = {
        1, 0, { FLOAT }, NULL
};

link L_DOUBLE = {
        1, 0, { DOUBLE }, NULL
};

link L_STRING = {
        0, 0, {(ARRAY<<2)+TQ_CONSTANT} , &L_CHAR
};

/*----------------------------------------------------------------------*/
HASH_TAB *Symbol_tab;        /* The actual table. */
HASH_TAB *Struct_tab;   /* The actual table.		*/
HASH_TAB *Class_tab;

symbol *Symbol_free = NULL; /* Free-list of recycled symbols.    */
Link *Link_free = NULL; /* Free-list of recycled links.	    */
structdef *Struct_free = NULL; /* Free-list of recycled structdefs. */

#define LCHUNK    10        /* new_link() gets this many nodes at one shot.*/

/*----------------------------------------------------------------------*/
symbol *new_symbol(char *name, int scope) {
    symbol *sym_p;

    if (!Symbol_free) {    /* Free list is empty.*/
        sym_p = (symbol *) newsym(sizeof(symbol));
        sym_p->defined_line = lineno;
    }
    else                        /* UnLink node from   */
    {                            /* the free list.     */
        sym_p = Symbol_free;
        Symbol_free = Symbol_free->next;
        memset(sym_p, 0, sizeof(symbol));
    }

//    printf("symbol: %s, %d, %d\n", name, b,sym_p);

    strcpy(sym_p->name, name);

    return sym_p;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void free_symbol(symbol *sym) {
    /* Discard a single symbol structure and any attached links and args. Note
     * that the args field is recycled for initializers, the process is
     * described later on in the text (see value.c in the code), but you have to
     * test for a different type here. Sorry about the forward reference.
     */

    if (sym) {
        if (IS_FUNCT(sym->type))
            free_symbol_chain(sym->args);          /* Function arguments. */
//        free_link_chain(sym->type);          /* Discard type chain. */

        sym->next = Symbol_free;              /* Put current symbol */
        Symbol_free = sym;                  /* in the free list.  */
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void free_symbol_chain(symbol *sym) {
    symbol *p = sym;

    while (sym) {
        p = sym->next;
        free_symbol(sym);
        sym = p;
    }
}

symbol *new_var_decl(char *text) {
    symbol *sym;
    if ((sym = (symbol *) findsym(Symbol_tab, text))) {
        printf("Error in %s line %d:\n\tlocal variable %s already declared as:\n\t%s %s (near line %d in file %s)\n",
                                           inputs[0],lineno,sym->name, type_string(sym->type), sym->name, sym->defined_line, inputs[0]);

        return NULL;
    } else if (!sym || sym->level != CONTEXT_LEVEL) {
        return new_symbol(text, CONTEXT_LEVEL);
    }
}

/*----------------------------------------------------------------------*/
Link *new_link() {
    /* Return a new link. It's initialized to zeros, so it's a declarator.
     * LCHUNK nodes are allocated from malloc() at one time.
     */

    Link *p;
    int i;

    if (!Link_free) {
        if (!(Link_free = (Link *) malloc(sizeof(link) * LCHUNK))) {
            yyerror("INTERNAL, new_link: Out of memory\n");
            exit(1);
        }

        for (p = Link_free, i = LCHUNK; --i > 0; ++p)   /* Executes LCHUNK-1 */
            p->next = p + 1;                 /*	       times. */

        p->next = NULL;
    }

    p = Link_free;
    Link_free = Link_free->next;
    memset(p, 0, sizeof(link));
    return p;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void free_link_chain(Link *p) {
    /* Discard all links in the chain. Nothing is removed from the structure
     * table, however. There's no point in freeing the nodes one at a time
     * since they're already linked together, so find the first and last nodes
     * in the input chain and Link the whole list directly.
     */

    Link *start = p;

    if (p) {
        while (p->next)    /* find last node */
            p = p->next;

        p->next = Link_free;
        Link_free = start;
    }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void free_link(Link *p)         /* Discard a single link. */
{
    p->next = Link_free;
    Link_free = p;
}

void free_structdef(structdef *sdef_p) {
    /* Discard a structdef and any attached fields, but don't free linked
     * structure definitions.
     */

    if (sdef_p) {
        free_symbol_chain(sdef_p->fields);
        sdef_p->fields = (symbol *) Struct_free;
        Struct_free = sdef_p;
    }
}

void add_declarator(symbol *sym, int type) {
    /* Add a declarator Link to the end of the chain, the head of which is
     * pointed to by sym->type and the tail of which is pointed to by
     * sym->etype. *head must be NULL when the chain is empty. Both pointers
     * are modified as necessary.
     */

    Link *link_p = new_link();      /* The default class is DECLARATOR. */
    link_p->clazz = DECLARATOR;
    link_p->DCL_TYPE = type;

    if (!sym->type)
        sym->type = sym->etype = link_p;
    else {
        sym->etype->next = link_p;
        sym->etype = link_p;
    }
}

/*----------------------------------------------------------------------*/

int get_sizeof(Link *p) {
    /* Return the size in bytes of an object of the the type pointed to by p.
     * Functions are considered to be pointer sized because that's how they're
     * represented internally.
     */

    int size;

    if (p->clazz == DECLARATOR)
        size = (p->DCL_TYPE == ARRAY) ? p->NUM_ELE * get_sizeof(p->next) : 8;
    else {
        switch (p->TYPE) {
            case CHAR:
                size = 1;
                break;
            case INT:
                size = p->_LONG ? 8 : 4;
                break;
            case STRUCT_T:
                size = p->V_STRUCT->size;
                break;
            case VOID:
                size = 0;
                break;
            case LABEL:
                size = 0;
                break;
        }
    }

    return size;
}

/*----------------------------------------------------------------------*/

symbol *reverse_links(symbol *sym) {
    /* Go through the cross-linked chain of "symbols", reversing the direction
     * of the cross pointers. Return a pointer to the new head of chain
     * (formerly the end of the chain) or NULL if the chain started out empty.
     */

    symbol *previous, *current, *next;

    if (!sym)
        return NULL;

    previous = sym;
    current = sym->next;

    while (current) {
        next = current->next;
        current->next = previous;
        previous = current;
        current = next;
    }

    sym->next = NULL;
    return previous;
}

/*----------------------------------------------------------------------*/

void print_syms(char *filename)    /* Print the entire symbol table to   */
{                    /* of the file (if any) are destroyed.*/
    FILE *fp;

    if (!(fp = fopen(filename, "w")))
        yyerror("Can't open symbol-table file\n");
    else {
        fprintf(fp, "Attributes in type field are:   upel\n");
        fprintf(fp, "    unsigned (. for signed)-----+|||\n");
        fprintf(fp, "    private  (. for public)------+||\n");
        fprintf(fp, "    extern   (. for common)-------+|\n");
        fprintf(fp, "    long     (. for short )--------+\n\n");

        fprintf(fp, "name               rname              lev   next   type\n");
//        ptab(Symbol_tab, (ptab_t) psym, fp, 1);

        fprintf(fp, "\nStructure table:\n\n");
//        ptab(Struct_tab, (ptab_t) pstruct, fp, 1);

        fclose(fp);
    }
}

/*----------------------------------------------------------------------*/

Link *append_type_specifier(Link *spec, int s) {
    if (s == UNSIGNED || s == SIGNED) {
        spec->_UNSIGNED = s;
    } else {
        spec->TYPE = s;
    }

    return spec;
}

Link *append_type_qualifier(Link *spec, int q) {
    spec->QUALIFIER = q;
    return spec;
}


void add_spec_to_decl(Link *p_spec, symbol *decl_chain) {
    /* p_spec is a pointer either to a specifier/declarator chain created
     * by a previous typedef or to a single specifier.
     *
     *
     * Typedefs are handled like this: If the incoming storage class is TYPEDEF,
     * then the typedef appeared in the current declaration and the tdef bit is
     * set at the head of the cloned type chain and the storage class in the
     * clone is cleared; otherwise, the clone's tdef bit is cleared (it's just
     * not copied by clone_type()).
     */

    for (; decl_chain; decl_chain = decl_chain->next) {
        if (!decl_chain->type)              /* No declarators. */
            decl_chain->type = p_spec;
        else
            decl_chain->etype->next = p_spec;

        while (p_spec->next)
            p_spec = p_spec->next;
        decl_chain->etype = p_spec;
    }
}

void add_symbols_to_table(symbol *sym) {
    /* Add declarations to the symbol table.
     *
     * The sym->rname field is modified as if this were a global variable (an
     * underscore is inserted in front of the name). You should add the symbol
     * chains to the table before modifying this field to hold stack offsets
     * in the case of local variables.
     */

    symbol *exists;        /* Existing symbol if there's a conflict.    */
    symbol *curr;

    for (curr = sym; curr; curr = curr->next) {
        exists = (symbol *) findsym(Symbol_tab, curr->name);
        curr->level = CONTEXT_LEVEL;

        if (!exists || exists->level != curr->level) {
            addsym(Symbol_tab, (void *) curr);
        } else {
            fprintf(stderr, "Error in %s line %d:\n\tlocal variable %s already declared as:\n\t%s (in file %s)\n",
                    inputs[0],lineno,exists->name, "des", inputs[0]);
        }
    }

//    print_sym_table(Symbol_tab);
}

void remove_symbols_from_table(symbol *sym) {
    symbol *curr;

    for (curr = sym; curr; curr = curr->next) {
        remove_sym(Symbol_tab, (void *) curr);
    }

//    printf("After removal:");
//    print_sym_table(Symbol_tab);
}

void print_sym(symbol *s){
    char *t = type_string(s->type);
    printf("\t name:(%s) level:%d type: %s", s->name, s->level, t);

    free(t);

    BUCKET *b = (BUCKET *)s- 1;
//    printf("  addr:%d next:%d  prev:%d\n", s, b->next, b->prev);
    printf("\n");
}

char* func_signature(symbol *sym){
    char space[256];
    char *s = space;
    s+= sprintf(s,"%s %s(", type_string(sym->type), sym->name);
    symbol *args = sym->args;
    for(;args;args=args->next){
        s+=sprintf(s, "%s,", type_string(args->type));
    }
    s+=sprintf(s, ")"); *s=0;
    return strdup(space);
}

char* type_string(Link *l){
    static const char * const TYPE_NAMES[] = {
            "void",
            "char",
            "short",
            "int",
            "float",
            "long",
            "double",
            "struct",
            "union",
            "enum",
            "class",
    };

    char space[256] = {0};
    char *str = space;

    Link *e = l;
    while(e->next)e=e->next;
    str+= sprintf(str,"%s", TYPE_NAMES[e->TYPE] );

    link *start = l;
    while(start!=e){
        switch (start->DCL_TYPE) {
            case POINTER:
                str+=sprintf(str,"*");
                break;
            case ARRAY:
                str+=sprintf(str,"[]");
                break;
            case FUNCTION:
                str+=sprintf(str,"()");
                break;
        }
        start = start->next;
    }
    return strdup(space);
}

void print_sym_table( HASH_TAB	*addr )
{
    BUCKET	**p, *bukp ;
    int	i;

    printf("Symbol_tab (%d element table, %d symbols)\n",
           addr->size, addr->numsyms );

    for( p = addr->table, i = 0 ; i < addr->size ; ++p, ++i )
    {
        if( !*p )
            continue;

        for( bukp = *p; bukp ; bukp=bukp->next )
        {
            symbol *s = (symbol *)(bukp+1);
            print_sym(s);
        }
    }
}