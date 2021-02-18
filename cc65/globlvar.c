
/*
   CC65 global var declarations.  These are extern'ed
   in cc65.h
*/
#include <stdio.h>
#include "cc65.h"

 

int curtok;   /* current token seen by parser */
int curval;		/* something or other, depending on curtok */
int nxttok; 
int nxtval;

int absdecl;
/* int	critic;		*/
int glblbl;
char * gsptr;
int hashval;
int lovptr;
char * lsptr;
char macdef;		/* T if any defines defined */
char * outqi;
int outqsz;
int tbllen;
char * tblptr;
int * wqptr;		                /* ptr to next entry */
int litptr;	                 	/* ptr to next entry in literal pool */

char * lptr;
char * mptr;		                /* ptrs into each */


char * gblspace;          	   /* global heap */
char * gblend;		              /* global space limit */

struct hashent * glvptr;
int i_ifdef;
char locspace[LSPACE];
char outq[OUTQSZ];    	        /* buf for asm stuff being output */
char s_ifdef[N_IFDEF];

int litlab;		                  /* current lit pool labl */
int litspace = 0;              /* total lit space used */

/*	Misc storage	*/

int nxtlab,		                  /* next avail label # */
    oursp,                     /* compiler relative stk ptr */
    argstk,	                   /* function arg sp */
    ncmp,                      /* # open compound statements */
    errcnt,                    /* # errors in compilation */
    eof,                       /* set non-zero on final input eof */
    glbflag;                   /* non-zero if internal globals */

struct filent filetab[MAXFILES];
int ifile;

FILE * inp;
FILE * output;

int ln;
char * fin;

char fname[80];

/*
  Canned typespecs
*/
/* char * type_char = "\021"; unused? */
char * type_int = "\022";
char * type_ifunc = "\007\022";

char linebuf[linesize];
char * line = linebuf;
char mlinebuf[linesize];
char * mline = mlinebuf;
char macltab[256];
int wq[wqtabsz]; /* while queue */
char litq[litabsz];
struct hashent * lvtab[128];
int * kwptr;
 
struct hashent * htab[HTABSZ];
char * macarg[MACARGSZ];
char * machtab[HTABSZ];


char optimize = 0;		   /* optimize flag */
char verbose = 0;		    /* verbose flag */
int n_funargs = 0;		   /* for use by funarg parser */
int asm_kludge = 0;		  /* see preproc, lex */
char * incl_dir = 0;	 	/* dir for include files */

/* char get_test = 0; */
/* flag for doif etc to tell getmem to generate a test, not a load */

char source = 0;	/* include source in m65 */

