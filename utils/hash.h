#ifndef __HASH_H
#define __HASH_H

typedef struct BUCKET {
    struct BUCKET **prev;
    struct BUCKET *next;

} BUCKET;


typedef struct hash_tab_ {
    int size;        /* Max number of elements in table	 */
    int numsyms;        /* number of elements currently in table */
    unsigned (*hash)(void *);       /* hash function		 	 */
    int (*cmp)(void *, void *); /* comparison funct, cmp(name,bucket_p); */
    BUCKET *table[1];        /* First element of actual hash table	 */

} HASH_TAB;

typedef void( *ptab_t )(void *, ...);    /* print argument to ptab */

extern HASH_TAB *maketab(unsigned maxsym, unsigned (*hash)(void *), int(*cmp)(void *, void *));

extern void *newsym(int size);

extern void freesym(void *sym);

extern void *addsym(HASH_TAB *tabp, void *sym);

extern void *findsym(HASH_TAB *tabp, void *sym);

extern void *nextsym(HASH_TAB *tabp, void *last);

extern void remove_sym(HASH_TAB *tabp, void *sym);

unsigned hash_pjw(unsigned char *name);    /* in hashpjw.c */

#endif /* __HASH_H */
