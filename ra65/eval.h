
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* defs for the evaluator */

/* these are bits in the 'expr_flags' byte returned from eval */

#define	E_UNDEF     1 /* expr contains an undefined sym */
#define	E_REL       2 /* expr contains a relative reference */
#define E_HI_BYTE   8 /* hi part of some value */
#define E_LO_BYTE  16 /* lo part */
#define E_WORD        /* not used yet */

// prototypes 


int  digit(char c);
char * grok_fixnum(char * str,int base,int *result);
char * eval_q(char * str,int * val,int * expr_flags,SYM ** sym_ref); 
char * eval_product(char *str, int *val,int *expr_flags,SYM **sym_ref);
char * eval_internal(char *str,int * val,int * expr_flags,SYM ** sym_ref);
int  eval(char * str,int * expr_flags,SYM ** sym_ref);

