#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "utils/hash.h"


void *newsym(int size) {
    /* Allocate space for a new symbol; return a pointer to the user space. */

    BUCKET *sym;

    if (!(sym = (BUCKET *) malloc(size + sizeof(BUCKET)))) {
        fprintf(stderr, "Can't get memory for BUCKET\n");
        raise(SIGABRT);
        return NULL;
    }
    return (void *) (sym + 1);        /* return pointer to user space */
}

/*----------------------------------------------------------------------*/

HASH_TAB *maketab(unsigned maxsym, unsigned (*hash)(void *), int(*cmp)(void *, void *)) {
    /*	Make a hash table of the indicated size.  */

    HASH_TAB *p;

    if (!maxsym)
        maxsym = 127;
    /*   |<--- space for table ---->|<- and header -->| */
    if ((p = (HASH_TAB *) calloc(1, (maxsym * sizeof(BUCKET *)) + sizeof(HASH_TAB)))) {
        p->size = maxsym;
        p->numsyms = 0;
        p->hash = hash;
        p->cmp = cmp;
    } else {
        fprintf(stderr, "Insufficient memory for symbol table\n");
        raise(SIGABRT);
        return NULL;
    }
    return p;
}


void *addsym(HASH_TAB *tabp, void *isym) {
    /* Add a symbol to the hash table.  */

    BUCKET **pp, *p;
    pp = &(tabp->table)[(*tabp->hash)(isym) % tabp->size];

    BUCKET *sym = ((BUCKET *) isym) - 1;
    p = *pp;
    *pp = sym;
    sym->prev = pp;
    sym->next = p;

//    printf("\taddsym %d next:%d  prev:%d  pp:%d p:%d \n", isym, sym->next, sym->prev, pp, p);

    if (p)
        p->prev = &sym->next;

    tabp->numsyms++;
    return (void *) (sym + 1);
}


void remove_sym(HASH_TAB *tabp, void *isym) {
    /*	Remove a symbol from the hash table. "sym" is a pointer returned from
     *  a previous findsym() call. It points initially at the user space, but
     *  is decremented to get at the BUCKET header.
     */

    BUCKET *sym = (BUCKET *) isym;

    if (tabp && sym) {
        --tabp->numsyms;
        --sym;

        if (*(sym->prev) = sym->next)
            sym->next->prev = sym->prev;
    }
}


void *findsym(HASH_TAB *tabp,
              void *sym) {
    /*	Return a pointer to the hash table element having a particular name
     *	or NULL if the name isn't in the table.
     */

    BUCKET *p;

    if (!tabp)        /* Table empty */
        return NULL;

    p = (tabp->table)[(*tabp->hash)(sym) % tabp->size];

    while (p && (*tabp->cmp)(sym, p + 1))
        p = p->next;

    return (void *) (p ? p + 1 : NULL);
}

/*----------------------------------------------------------------------*/

void *nextsym(HASH_TAB *tabp,
              void *i_last) {
    /* Return a pointer the next node in the current chain that has the same
     * key as the last node found (or NULL if there is no such node). "last"
     * is a pointer returned from a previous findsym() of nextsym() call.
     */

    BUCKET *last = (BUCKET *) i_last;

    for (--last; last->next; last = last->next)
        if ((tabp->cmp)(last + 1, last->next + 1) == 0)   /* keys match */
            return (char *) (last->next + 1);
    return NULL;
}

#define MAXLEN    128            /* Used by pstat(), max number	*/
/* of expected chain lengths. 	*/
/*----------------------------------------------------------------------
 * The following test routines exercise the hash functions by building a table
 * consisting of either 500 words comprised of random letters (if RANDOM is
 * defined) or by collecting keywords and variable names from standard input.
 * Statistics are then printed showing the execution time, the various collision
 * chain lengths and the mean chain length (and deviation from the mean). If
 * you're using real words, the usage is:
 *		cat file ... | hash  	or 	hash < file
 */

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

typedef struct {
    char name[32];    /* hash key			   */
    char str[16];    /* Used for error checking 	   */
    unsigned hval;    /* Hash value of name, also "	   */
    int count;    /* # of times word was encountered */
}
        STAB;


#define NBITS_IN_UNSIGNED    32
#define SEVENTY_FIVE_PERCENT    ((int)( NBITS_IN_UNSIGNED * .75  ))
#define TWELVE_PERCENT        ((int)( NBITS_IN_UNSIGNED * .125 ))
#define HIGH_BITS        ( ~( (unsigned)(~0) >> TWELVE_PERCENT)  )

unsigned hash_pjw(unsigned char *name) {
    unsigned h = 0;            /* Hash value */
    unsigned g;

    for (; *name; ++name) {
        h = (h << TWELVE_PERCENT) + *name;
        if (g = h & HIGH_BITS)
            h = (h ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }
    return h;
}


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//main( argc, argv )
//char	**argv;
//{
//    char	 word[ 80 ];
//    STAB	 *sp;
//    HASH_TAB	 *tabp;
//    struct timeb start_time, end_time ;
//    double	 time1, time2;
//    int		 c;
//
//    /* hash a <list 	for hash_add */
//    /* hash p <list 	for hash_pjw */
//
//    c = (argc > 1) ? argv[1][0] : 'a' ;
//
//    printf( "\nUsing %s\n", c=='a' ? "hash_add" : "hash_pjw" );
//
//    tabp = maketab( 127, c=='a' ? hash_add : hash_pjw , strcmp );
//
//    ftime( &start_time );
//
//    while( getword( word ) )
//    {
//	if( sp = (STAB *) findsym(tabp, word) )
//	{
//	    if( strcmp(sp->str,"123456789abcdef") ||
//					( sp->hval != hash_add( word )) )
//	    {
//		printf("NODE HAS BEEN ADULTERATED\n");
//		exit( 1 );
//	    }
//
//	    sp->count++;
//	}
//	else
//	{
//	    sp = newsym( sizeof(STAB) );
//	    strncpy( sp->name, word, 32 );
//	    strcpy ( sp->str, "123456789abcdef" );
//	    sp->hval = hash_add( word );
//	    addsym( tabp, sp );
//	    sp->count = 1;
//	}
//    }
//
//    ftime( &end_time );
//    time1 = (start_time.time * 1000) + start_time.millitm ;
//    time2 = (  end_time.time * 1000) +   end_time.millitm ;
//    printf( "Elapsed time = %g seconds\n", (time2-time1) / 1000 );
//
//    pstats( tabp );
//
//    /* dptab( tabp ); */
//}
