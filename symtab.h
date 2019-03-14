#ifndef __SYMTAB_H
#define __SYMTAB_H

#include "utils/hash.h"

#define NAME_MAX  32                /* Maximum identifier length.     */
#define LABEL_MAX 32                /* Maximum output-label length.   */

extern int lineno;
extern char **inputs;

extern HASH_TAB *Symbol_tab;        /* The actual table. */
extern HASH_TAB *Struct_tab;   /* The actual table.		*/
extern HASH_TAB *Class_tab;

typedef struct symbol                /* Symbol-table entry for vars and functions.	      */
{
    char name[NAME_MAX + 1];            /* Input variable name.	      */
    char rname[NAME_MAX + 1];            /* Actual variable name.	      */

    unsigned level     : 13;        /* Declaration lev., field offset.*/
    unsigned implicit  : 1;        /* Declaration created implicitly.*/
    unsigned duplicate : 1;        /* Duplicate declaration.	      */

    struct link *type;            /* First link in declarator chain.*/
    struct link *etype;            /* Last  link in declarator chain.*/
    union {
        struct symbol *args;                /* If a funct decl, the arg list. */
        void *init_val;
    };/* If a var, the initializer.     */

    int defined_line;

    struct symbol *next;            /* Cross link to next variable at */
    /* current nesting level.	      */
} symbol;

#define POINTER        0        /* Values for declarator.type. 	  */
#define ARRAY        1
#define FUNCTION    2

typedef struct declarator {
    unsigned dcl_type  :2;  /* POINTER, ARRAY, or FUNCTION	  */
    unsigned qualifier :2;  /* CONSTANT RESTRICT VOLATILE */
    int num_ele;            /* If class==ARRAY, # of elements */
} declarator;

#define VOID      0        /* default.   */
#define CHAR      1        /* specifier.type. INT has the value 0 so   */
#define SHORT     2        /* specifier.type. INT has the value 0 so   */
#define INT       3        /* specifier.type. INT has the value 0 so   */
#define FLOAT     4        /* specifier.type. INT has the value 0 so   */
#define LONG      5        /* specifier.type. INT has the value 0 so   */
#define DOUBLE    6        /* that an uninitialized structure defaults */
#define STRUCT_T  7
#define UNION_T   8
#define ENUM_T    9
#define CLASS_T   10        /* to int, same goes for EXTERN, below.	    */

#define LABEL     14

/* specifier.sclass			*/
#define SC_NO 0        /*     Default.          */
#define SC_TYPEDEF   1        /*     Typedef.          */
#define SC_EXTERN    2        /*     At a fixed address.		*/
#define SC_STATIC    3        /*     At a fixed address.		*/
#define SC_AUTO      4        /*     Local vars On the stack.		*/
#define SC_REGISTER  5        /*     In a register.			*/

/* type qualifier */
enum type_qualifier {
    TQ_CONSTANT = 1,
    TQ_RESTRICT = 2,
    TQ_VOLATILE = 3,
};

/* Output (C-code) storage class	*/
#define NO_OCLASS 0        /*	No output class (var is auto).  */
#define PUB      1        /*	public				*/
#define PRI      2        /*	private				*/
#define EXT      3        /*	extern				*/
#define COM      4        /*	common				*/

typedef struct specifier {
    unsigned type      :4;    /* VOID CHAR INT DOUBLE STRUCTURE    ...  	 */
    unsigned _unsigned :1;    /* 1=unsigned.  0=signed.	  		 */
    unsigned _long     :2;    /* 1=long, 2=long long	  	 */

    unsigned sclass    :3;    /* TYPEDEF EXTERN STATIC AUTO REGISTER  */
    unsigned qualifier :2;    /* CONSTANT RESTRICT VOLATILE */
    unsigned oclass    :3;    /* Output storage class: PUB PRI COM EXT.  */

    unsigned _constant :1;    /* 1=constant.  0=inconstant.	  		 */
    unsigned _extern   :1;    /* 1=extern keyword found in declarations. */
    unsigned _static   :1;    /* 1=static keyword found in declarations. */

    union {                /* Value if constant:			  */
        int v_int;    /* Int & char values. If a string const., */
        /* is numeric component of the label.	  */
        unsigned int v_uint;  /* Unsigned int constant value.		  */
        long v_long;  /* Signed long constant value.		  */
        unsigned long v_ulong; /* Unsigned long constant value.	  */

        struct structdef *v_struct; /* If this is a struct, points at a	*/
    } const_val;
} specifier;


#define DECLARATOR    0
#define SPECIFIER    1

typedef struct link {
    unsigned clazz   : 1;        /* DECLARATOR or SPECIFIER 	      */
    unsigned tdef    : 1;            /* For typedefs. If set, current link */
    /* chain was created by a typedef.    */
    union {
        specifier s;        /* If class == DECLARATOR	      */
        declarator d;        /* If class == SPECIFIER	      */
    };
    
    struct link *next;               /* Next element of chain.	      */

} Link;

extern link L_CHAR;

extern link L_INT;

extern link L_LONG;

extern link L_FLOAT;

extern link L_DOUBLE;

extern link L_STRING;

/*----------------------------------------------------------------------
 * Use the following p->XXX where p is a pointer to a link structure.
 */

#define TYPE        s.type
#define SCLASS        s.sclass
#define _LONG        s._long
#define _CONSTANT        s._constant
#define _UNSIGNED    s._unsigned
#define _EXTERN        s._extern
#define _STATIC        s._static
#define OCLASS        s.oclass
#define QUALIFIER         s.qualifier

#define DCL_TYPE    d.dcl_type
#define NUM_ELE        d.num_ele
#define PTR_QUA         d.qualifier

#define VALUE        s.const_val
#define V_INT        VALUE.v_int
#define V_UINT        VALUE.v_uint
#define V_LONG        VALUE.v_long
#define V_ULONG        VALUE.v_ulong
#define V_STRUCT    VALUE.v_struct
#define V_TYPE    VALUE.type_name

/*--------------------------------------------------------------------*/
/* Use the following XXX(p) where p is a pointer to a link structure. */

#define IS_SPECIFIER(p)  ((p) && (p)->clazz==SPECIFIER )
#define IS_DECLARATOR(p) ((p) && (p)->clazz==DECLARATOR )
#define IS_ARRAY(p)   ((p) && (p)->clazz==DECLARATOR && (p)->DCL_TYPE==ARRAY   )
#define IS_POINTER(p) ((p) && (p)->clazz==DECLARATOR && (p)->DCL_TYPE==POINTER )
#define IS_FUNCT(p)   ((p) && (p)->clazz==DECLARATOR && (p)->DCL_TYPE==FUNCTION)
#define IS_STRUCT(p)  ((p) && (p)->clazz==SPECIFIER  && (p)->TYPE == STRUCTURE )
#define IS_LABEL(p)   ((p) && (p)->clazz==SPECIFIER  && (p)->TYPE == LABEL     )

#define IS_CHAR(p)      ((p) && (p)->clazz == SPECIFIER && (p)->TYPE == CHAR )
#define IS_INT(p)       ((p) && (p)->clazz == SPECIFIER && (p)->TYPE == INT  )
#define IS_UINT(p)    (    IS_INT(p) && (p)->UNSIGNED         )
#define IS_LONG(p)      (    IS_INT(p) && (p)->LONG             )
#define IS_ULONG(p)    (    IS_INT(p) && (p)->LONG && (p)->UNSIGNED     )
#define IS_UNSIGNED(p)    ((p) && (p)->UNSIGNED                 )

#define IS_AGGREGATE(p)     ( IS_ARRAY(p) || IS_STRUCT(p)    )
#define IS_PTR_TYPE(p)     ( IS_ARRAY(p) || IS_POINTER(p)   )

#define    IS_CONSTANT(p)     (IS_SPECIFIER(p) && (p)->SCLASS == CONSTANT    )
#define    IS_TYPEDEF(p)      (IS_SPECIFIER(p) && (p)->SCLASS == TYPEDEF    )
#define    IS_INT_CONSTANT(p) (IS_CONSTANT(p)  && (p)->TYPE   == INT    )

typedef struct structdef {
    char tag[NAME_MAX + 1];  /* Tag part of structure definition.      */
    unsigned char level;        /* Nesting level at which struct declared.*/
    symbol *fields;        /* Linked list of field declarations.     */
    unsigned size;            /* Size of the structure in bytes.	      */

} structdef;


/*type of expressions.*/
enum expr_type {
    E_STRING = DOUBLE+1,
    E_ID, /*from VOID to DOUBLE.*/
    POST_OP,
    UNARY_OP,
    E_CAST,
    BINARY_OP,
    TERNARY,
    E_ASSIGN,
};

/*type of statements.*/
enum stmt_type {
    LABEL_ID,
    LABEL_CASE,
    LABEL_DEFAULT,

    COMPOUND_STMT,
    EXPR_STMT,
    IF_STMT,
    SWITCH_STMT,
    WHILE_STMT,
    DO_STMT,
    FOR_STMT,

    CONTINUE_STMT,
    BREAK_STMT,
    RETURN_STMT,
};

#define CONST_STR(p) tconst_str((p)->type)   /* Simplify tconst_str() calls */
/* with value arguments by     */
/* extracting the type field.  */


extern symbol *new_symbol(char *name, int scope);

extern void free_symbol(symbol *sym);

extern void free_symbol_chain(symbol *sym);

extern symbol *new_var_decl(char *text);

extern Link *new_link(void);

extern void free_link_chain(Link *p);

extern void free_link(Link *p);

extern void add_declarator(symbol *sym, int type);

extern Link *clone_type(Link *tchain, Link **endp);

extern int get_sizeof(Link *p);

extern symbol *reverse_links(symbol *sym);

extern void print_syms(char *filename);

extern Link *append_type_specifier(Link *spec, int s);

extern Link *append_type_qualifier(Link *spec, int q);

extern void add_spec_to_decl(Link *p_spec, symbol *decl_chain);

extern void add_symbols_to_table(symbol *sym);

extern void remove_symbols_from_table(symbol *sym);

extern void print_sym(symbol *s);

extern char* type_string(Link *l);

extern char* func_signature(symbol *);

extern void print_sym_table(HASH_TAB *addr);

//value *new_value();
//
//void free_value(value *p);

#endif /* __SYMTAB_H */

