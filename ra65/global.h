
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* global vars for a65 */

//#define DEBUG
#define LIST

#ifdef MAIN
#define ID
#else
#define ID extern
#endif

ID char line[256*2];		/* line buffer */
ID struct parse p;		/* parsed line */
ID int line_nbr;		  /* current line number */
ID int file_nbr;      /* current file number */

ID char * list_name;		/* name of file we're listing to, if any */
ID int  list_pc;			/* pc before read line */
ID char list_pc_p;		/* lsit pc even if no object bytes */
ID char obj[4];			/* generated code */
ID int  obj_count;		/* n valid bytes */
ID char line_listed_p;		/* if text this line has been printed */
ID int  list_v;			/* other value to show up in listing */
ID char list_v_p;		/* use that instead of obj bytes */

ID char page_title[64];		/* page title string */
ID char page_subttl[64];	/* subtitle */
ID int  page_nbr;		/* page number */
ID int  line_in_page;

ID int  errcount;
ID char * CurrentFile; 
ID char output_p;		/* flag for generating output */

/* conditional stuff */

ID char disabled;		/* if we've evaluated an IF that was false */

/* global assembler state */

ID unsigned int pc;		/* pc for assembler */

#define SEG_ABS	0		/* we're doing absolute defs */
#define SEG_REL	1		/* we're doing relative defs */
ID int curr_seg;		/* current state; one of the above values */
ID unsigned int abs_pc, rel_pc;	/* last known pc, each seg */
ID char rel_p;			/* if relocatable output */

ID int  pass;			/* what pass we're doing */
ID int  end_file;		/* if we saw a .end pseudo */
ID char list_p;			/* generating listing */

ID char verbose;			/* various verbosities */

char *frob_name();

ID short asm_var;   /* "@" in a label is replaced by this value */
ID short CurrSegment;         /* which segment we're in */

#define   TEXT  0
#define   DATA  1
#define   BSS   2
#define   BSSZP 3
