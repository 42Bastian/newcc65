
/* global defs for CC65 */

#undef M6502

/* added by jrd.  don't generate arg counts here */
#define NOARGC
/* ... and generate entries with fixed arg counts */
#define FIXARGC

#define cout	Cout
extern int debug;
#define EOL	'\n'
#define TABCHAR	'\t'
/*
#define CRITIC		0x42
#define SDMCTL		0x22F
#define DMACTL		0xD400
#define CH		0x2FC
#define MEMLO		0x2E7
*/

#define HTABSZ		512
#define MACARGSZ	64
#define MAXFILES	6  /* changed from 3 42BS */
#define MAXTYPELEN	30
#define N_IFDEF		5
/* #define LITLAB		1 */

extern int litlab;		/* current one */
extern int litspace;		/* total string space used */

 

/* cross-compiler... */
#define GSPACE		(128*1024)
#define LSPACE		(128*1024)
#define OUTQSZ		(80*1024)

#define printmsg printf


/*
 * storage classes
 */
#define SC_STATIC	  0x00
#define SC_STACK    0x01
#define SC_STORAGE  0x02
#define SC_ONSTACK  0x03
#define SC_INMEM    0x02
#define SC_DEFINED  0x04
#define SC_EXTERN   0x08
#define SC_ENUM     0x10
#define SC_DEFAULT  0x40
#define SC_STRUCT   0x80
#define SC_SFLD     0x80
#define SC_STAG     0x81
/*42BS*/
#define SC_GLOBAL    0x20
#define SC_REGISTER 0x100
#define SC_ZSTATIC  0x200
/*
  data types
*/
#define	T_END		'\0'
#define T_CHAR		0x11
#define T_INT		0x12
#define	T_SHORT		0x13
#define T_LONG		0x14
#define T_ENUM		0x15
#define T_UCHAR		0x19
#define T_UINT		0x1A
#define T_FLOAT		0x25
#define T_DOUBLE	0x26
#define T_FUNC		0x07
#define T_UNSIGNED	0x08
#define T_INTEGER	0x10
#define T_REAL		0x20
#define T_POINTER	0x40
#define T_PTR		0x49
#define T_ARRAY		0x4A
#define T_STRUCT	0x80
#define T_UNION		0xC0
#define T_SMASK		0x3F

#define E_MGLOBAL	0x80
#define E_MLOCAL	0x40
#define E_MCONST	0x20
#define E_MEXPR		0x10
#define E_MEOFFS	0x11
#define E_MCTYPE	0x07
#define E_TCONST	0
#define E_TGLAB		1
#define E_TLIT		2
#define E_TLOFFS	3
#define E_TLLAB   4 /* local static */

/* jrd added these defs as values for the e_test field of
   a struct expent.  The idea is that internals of the expr
   parsers will set the e_test slot of a struct expent to
   one of these values.  0 means no specific test info, do
   it the old way. */

		/* expr has already set cond codes apropos result value */
#define E_CC		1
		/* if expr has NOT set CC, force a test-style access */
#define E_TEST		2
		/* expr has left a logical value (1 or 0) in AX */
#define E_LOGL		4

		/* flip this bit to invert sense of test */
#define E_XINV		8


#define	NAMESIZE 33
#define NAMEMAX  32

#define	wqtabsz		128
#define	wqsiz		6
#define	wqmax		wq+wqtabsz-wqsiz
#define	wqsym		0
#define	wqsp		1
#define	wqloop		2
#define	wqlab		3
#define	wqinc		4
#define	wqstat		5
#define	litabsz		512
#define	litmax		litabsz-1

 
#define	linesize	512	/* hope this is enough :) */

#define	linemax		linesize-1
#define	mpmax		linemax

struct hashent 
{
  struct hashent * ptr;
  struct hashent * typeptr;
  struct hashent * s_ptr;
  
  struct
  {
    short g;
    short l;
  } flag;
  struct
  {
    char * g;
    char * l;
  } tptr;
  struct hashent * link;
  struct
  {
    int g;
    int l;
  }data;
  char name[1];
};

/* used by newpre.c and newcmd.c.  sort of like op_alist... */
struct tok_elt 
{
  char * toknam;
  int toknbr;
};

struct expent 
{
  /* this used to be just an int.  jrd split it into two bytes,
     in order to have an extra field for tests without taking up
     any more space ...
     int		e_flags;	*/
    
  unsigned char e_flags;
  unsigned char e_test;
  char * e_tptr;
  int e_const;
  struct hashent * typeptr;
};

struct filent 
{
#ifdef old_cruft
  int f_iocb;
#else
  FILE * f_iocb;
#endif
  char * f_name;
  int f_ln;
};

extern char		litq[];
extern char		macltab[256];
extern int		wq[wqtabsz];
extern struct hashent * lvtab[128];
extern char		outq[OUTQSZ];
 

extern struct hashent * htab[HTABSZ];
extern char *		macarg[MACARGSZ];
extern char *		machtab[HTABSZ];

extern int	absdecl;
/* extern int	critic; */
extern int	glblbl;

/* extern char	glbspace[GSPACE]; */
extern char *	gblspace;	/* global heap */
extern char *	gblend;		/* gblspace limit */

extern struct hashent * glvptr;
extern char *	gsptr;
extern int	hashval;
extern int	i_ifdef;
extern char	locspace[LSPACE];
extern int	lovptr;
extern char *	lsptr;
extern char	macdef;		/* T if any defines defined */
extern char *	outqi;
extern int	outqsz;
/* extern int	ret_addr; */
extern char	s_ifdef[N_IFDEF];
extern int	tbllen;
extern char *	tblptr;
extern char *	type_char;
extern char *	type_int;
extern char *	type_ifunc;

extern int *	wqptr;
extern char *	line;
extern char *	mline;
extern int	litptr;
extern char *	lptr;
extern char *	mptr;
extern int	nxtlab,
	oursp,
	argstk,
	ncmp,
	errcnt,
	eof,
	glbflag;

extern struct filent	filetab[MAXFILES];
extern int	ifile;

 
extern FILE *	inp;
extern FILE *	output;
 

extern int	ln;
extern char *	fin;

 
extern char	fname[80];
 

extern int	curtok;
extern int	curval;
extern int	nxttok;
extern int	nxtval;

extern char	optimize;	/* optimize flag */
extern char	verbose;	/* verbose flag */
extern int	n_funargs;	/* n args in function def */
extern int	asm_kludge;	/* kludge for #asm processing */
extern char *	incl_dir;	/* dir for include files */

/* extern char	get_test; */	/* flag for getmem */
 
extern char	source;		/* put source in m65 file as comments */
 

char *	Gmalloc();
char *	Lmalloc();


#define dbgprintf(foo,bar) {}
 

/* this struct added by jrd, used in unified code-generation stuff.  see
   exp1, exp2, exp3.  */
struct op_alist
{
  int tok;			/* token representing op */
  void (* gen)();		/* generator function that goes with it */
};


extern int register_base;
#define REGISTER_BASE 8		/* start at $08 with registers */
extern int interrupt;
/* end of global file for cc */
