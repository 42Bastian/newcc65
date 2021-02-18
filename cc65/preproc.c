
/* C pre-processor functions */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

#define	ch()	(*lptr)

extern void needbrack(int);
 
extern int debug;

char *ExpandIncludePath();

/* extern char	ascii_tab[]; */

inline int  white_p(char c);
int  quote_p(char c);
void doinclude();
inline int  keepch(char c);
void keepstr(char * str);
void doasm();
int  tok_assoc(char * sym,struct tok_elt *  toks);
void preprocess();
void xlateline();
int  pp0(char * curlin);
int  pp1(char * from,char *to);
int  pp2(char * from,char *  to,char * (* pfind)(),void (*prepl) () );
void replmac(char * pdef);
void doundef();
int  doiff(int skip);
int  doifdef(int skip,int  flag);
int  setmflag(int skip,int  flag,int  cond);
void addmac();
char * findmac(char * sname);
char * findarg(char * sname);
int  domcall(char * macptr);
void comment();
void skipblank();
void skipquote(int qchar);
#ifdef why_bother
void IllSym();
#endif
int  macname(char * sname);

/* io.c */
char nch();
char cgch();
char gch();
void do_kill();
int readline();

/* cruft.c */
int is_alpha(int c);
/* lexer.c */
int issym(char * s);
void gettok();
void symname(char * s);

/* cruft.c */
int matchstr(char * line,char * str);
/* symtab.c */
int hash(char * s);
/* glb.c */
void constexp(struct expent * lval);
/* stmt1.c */
void ns();
/****************************************************************/

/* predicate for whitespace */
inline int white_p(char c)
{
  return ((c == ' ') || (c == TABCHAR));
}

/* predicate for quote char */
 
int quote_p(char c)
{
  return ((c == '"') || (c == 39));
}
/*
	doinclude()
	Open an include file.
*/
void doinclude()
{
  char angflag;
  char c;
  char * p;
   
  if (ifile >= MAXFILES - 1)
    {
      Error("too many open files");
      goto done;
    }

  mptr = mline;
  skipblank();
  if ( (angflag = cgch()) == '<' )
    angflag = '>';
  else
    if ( angflag != '"' )
      {
        Missing("quote or <");
        goto done;
      }

  filetab[ifile].f_ln = ln;
  strcpy(p = Gmalloc(strlen(fin) + 1), fin); /* save existing file name */
  filetab[ifile].f_name = p;
  filetab[ifile].f_iocb = inp;
  ++ifile;

  fin = p = fname;

  /* if we've got an angle frob, try to insert include dir */
  if ((angflag == '>') && (incl_dir))
    {
      strcpy(p, incl_dir);
      p += strlen(p);
    }

  while ((c = *lptr) && (c != angflag))
    {
      *p = c;
      ++p;
      ++lptr;
    }
  if (c == '\0')
    {
      Missing("quote or >");
    }
  *p = '\0';

  if(debug)
   fprintf(stderr, "incl_dir: %s\n", incl_dir);
  
  if (angflag == '>' &&  !incl_dir)
  {
    fin = ExpandIncludePath(fin);  /* -Intruder */
  } 

  if (verbose)
    printmsg("including '%s'\n", fin);

  if (!(inp = (FILE *)fopen(fin, "r")))
    {
      inp = filetab[--ifile].f_iocb; /* oops! restore old file */
      fin = filetab[ifile].f_name;
      Error("open failure on include file");
      printf("error %d\n", errno);
    }
  else
    {
      ln = 0;
    }

 done:
  /*
    clear rest of line so next read will come from new file (if open)
  */
  do_kill();
}

/*
  keepch( c )
  Put character c into translation buffer.
*/
inline int keepch(char c)
 
{
  /*
  if (mptr <= mline + mpmax) *mptr++ = c;
  */
  return (*mptr++ = c);
}

/*
  keepstr( str )
  Put string str into translation buffer.
*/
 
void keepstr(char * str)
 
{
  while (*str)
    *mptr++ = *str++;
}


/* added by jrd, as we do asm stuff the easy way */
static char * endasm = "#endasm";
void doasm()
{
  do_kill(); /* zap the line with the #asm on it */
  asm_kludge = 0;		/* say we're done with this one */
  while (1)
    {
      if (readline() == 0)	/* off eof?? */
	{
	  Missing(endasm);
	  break;
	}
      if (matchstr(lptr, endasm))	/* end of asm sequence? */
	break;			/* yup, stop here */
      else
	{
	  ol(lptr);		/* output the line directly */
	  do_kill();		/* zap this line */
	}
    }
  do_kill();			/* so we start reading at next line */
}

void doasm2()
{
  gettok();
  if (curtok != LPAREN)
  {
    Missing("opening bracket");
    return;
  }
  while(nxttok != RPAREN)
  {
    if (nxttok != SCONST)
    {
      Missing(" string constant !");
      break;
    }      
    ot(litq+nxtval);
    litptr -= strlen(litq+nxtval)+1;
    gettok();
  }
  nl();
  
  gettok();
  gettok();
  ns();
}

 
/*
  preprocess()
  C preprocessor.
*/

/* stuff used to bum the keyword dispatching stuff */
#define D_ILGL	0
#define D_DEF	1
#define D_INC	2
#define D_UND	3
#define	D_IFD	4
#define D_IFND	5
#define D_IF	6
#define D_END	7
#define	D_ELS	8
#define D_ASM	9
#define D_ENDASM 10

struct tok_elt pre_toks[] =
{
  {"define", D_DEF},
  {"include", D_INC},
  {"undef", D_UND},
  {"ifdef", D_IFD},
  {"ifndef", D_IFND},
  {"if", D_IF},
  {"endif", D_END},
  {"else", D_ELS},
  {"asm", D_ASM},	/* jrd added this one */
  {"endasm",D_ENDASM},  /* 42BS */
  {0, 0}
};

int tok_assoc(char * sym,struct tok_elt *  toks)
 
{
  while (toks->toknam && strcmp(toks->toknam, sym))
    ++toks;
  return (toks->toknbr);
}

void preprocess()
{
int c;
int skip;
char sname[NAMESIZE];

  /*
    process compiler directives
  */
  lptr = line;
  skip = 0;
  while ((c = ch()) == '\0' || c == '#' || skip)
  {
     
    if (c == '#')
	  {
	    ++lptr;
	    skipblank();
	    if (!issym(sname))
	    {
	      Missing("compiler directive");
	      do_kill();
	    }
	    else
	      switch (tok_assoc(sname, pre_toks))
	      {
	      case D_DEF:
		    {
		      if (skip == 0)
		      {
		        addmac();
		        macdef = 1;
		      }
		    }
		    break;
	      case D_INC:
		    {
		      if (skip == 0)
		        doinclude();
		    }
		    break;
	      case D_UND:
		    {
		      if (skip == 0)
		        doundef();
		    }
		    break;
	      case D_IFD:
		    {
		      skip = doifdef(skip, 1);
		    }
		    break;
	      case D_IFND:
		    {
		      skip = doifdef(skip, 0);
		    }
		    break;
	      case D_IF:
		    {
		      skip = doiff(skip);
		    }
		    break;
	      case D_END:
		    {
		      if (i_ifdef >= 0)
		      {
		        skip = s_ifdef[i_ifdef--] & 1;
		      }
		      else
		      {
		        Error("#endif without matching #if");
		      }
		    }
		    break;
	      case D_ELS:
		    {
		      if (s_ifdef[i_ifdef] & 2)
		      {
		        if (s_ifdef[i_ifdef] & 4)
			      {
			        skip = !skip;
			      }
		        s_ifdef[i_ifdef] ^= 2;
		      }
		      else
		      {
		        Error("#else without matching #if");
		      }
		    }
		    break;
	      case D_ASM:
		    {
/* Oops!  Can't doasm here, as we might be prefetching
   a token while in the middle of something that can't be
   interrupted.  return from preprocess, and let the
   kludge in gettok deal with it.
   			doasm();
			  break;
*/
		      if ( skip == 0 )
		      {
		        lptr = line;
		        return;
		      }
		      break;
		    }
		    case D_ENDASM:
		      if ( skip == 0)
		      {
		        Illegal("#endasm without #asm");
		        do_kill();
		      }
		      break;
	      default:
		    {
		      Illegal("compiler directive");
		      do_kill();
		    }
	      }   			/* end of case on directive */
	  }
    if (readline() == 0)
	  {
	    if (i_ifdef >= 0)
	      Missing("#endif");
	    return;
	  }
  }
  xlateline();
  
  if (debug || verbose)
    printf("LINE %d : %s\n",ln , line);
}

/*
  xlateline()
  Translate line.
*/
void xlateline()
{
int cnt;
int done;
char * p;

  if ((macdef == 0) && (pp0(line) == 0))
    {
      return;
    }
  done = pp1(line, mline);
  cnt = 5;
  do
  {
    p = line;
    line = mline;
    mline = p;
    if (done)
      break;
    done = pp2(line, mline, findmac, replmac);
    keepch('\0');
  } while (--cnt);
  lptr = line;
}

/*
  pp0( curlin )
  Convert tabs to blanks and determine whether further pre-processing
  is necessary
*/
/* old stuff, from const, above...*/
 
int pp0(char * curlin)
{
int flag;

  flag = 0;
  while (*curlin)
  {
    switch (*curlin)
	  {
	  case TABCHAR:
	    *curlin = ' ';
	    break;
	  case '\'':
	  case '"':
	    ++flag;
	    break;
	  case '/':
	    if (curlin[1] == '*')
	    {
	      ++flag;
	      ++curlin;
	    }
	    break;
	  }
    ++curlin;
  }
  return (flag);
}


/*
  pp1( from, to )
  Preprocessor pass 1.  Remove whitespace and comments.
*/
int pp1(char * from,char *to)
{
int c;
int done;
char sname[NAMESIZE];

  lptr = from;
  mptr = to;
  done = 1;
  while ( (c = ch()) )
  {
    if (white_p(c))
	  {
	    keepch(' ');
	    skipblank();
	  }
    else if (macdef && is_alpha(c))
	  {
	    symname(sname);
	    if (macltab[(int)sname[0]])
	    {
	      done = 0;
	    }
	    keepstr(sname);
	  }
    else if (quote_p(c))
	  {
	    skipquote(c);
	  }
    else if ((c == '/') && (nch() == '*'))
	  {
	    keepch(' ');
	    comment();
	  }
	  else if ((c == '/') && (nch() == '/'))
	  {
	    keepch(' ');
	    do_kill();
	  }
    else
	  {
//    keepch(gch());
	    *mptr++ = *lptr++;
	  }
  }
  keepch('\0');
  return (done);
}

/*
  pp2( from, to , pfind, prepl )
  Preprocessor pass 2.  Perform macro substitution.
*/
int pp2(char * from,char *  to,char * (* pfind)(),void (*prepl) () )
{
int c;
int no_chg;
char * s;
char sname[NAMESIZE];

  lptr = from;
  mptr = to;
  no_chg = 1;
  while ( (c = ch()) )
  {
    if (is_alpha(c))
	  {
	    symname(sname);
	    if ( (s = (*pfind)(sname)) )
	    {
	      (*prepl) (s);
	      no_chg = 0;
	    }
	    else
	    {
	      keepstr(sname);
	    }
	  }
    else if ( c == '#' && is_alpha(nch()) )	/* macro to string expansion */
    {
      gch();
      symname(sname);
  	  if ( (s = (*pfind)(sname)) )
	    {
	      *mptr++='"';
	      (*prepl) (s);
	      *mptr++='"';
	      no_chg = 0;
	    }
	    else
	    {
	      Missing("macro-name");
	      do_kill();
	      return 0;
	    }
    }else if (quote_p(c))
	  {
	    skipquote(c);
	  }
    else
	  {
//    keepch(gch());
	  *mptr++ = *lptr++;
	  }
  }
  return (no_chg);
}

/*
  replmac( pdef )
  Function copies definition (pointed to by pdef) of current macro
  symbol to mline.
*/
void replmac(char * pdef)
{
  pdef += strlen(pdef) + 1;
  if (*pdef & 0x80)
  {
    if (domcall(pdef + 1) == 0)
	  {
	    do_kill();
	  }
  }
  else
  {
    keepstr(pdef);
  }
}

/*
  doundef()
  Process #undef directive
*/
void doundef()
{
char * s;
char sname[NAMESIZE];

  skipblank();
  if (macname(sname) && (s = findmac(sname)))
  {
    *s = '\0';
  }
}

/*
  doiff()
  Process #if directive
*/
int doiff(int skip)
{
struct expent lval;
int sv1;
int sv2;

  sv1 = curtok;
  sv2 = nxttok;
  strcat(line, ";;");
  gettok();
  gettok();
  constexp(&lval);
  curtok = sv1;
  nxttok = sv2;
  return (setmflag(skip, 1, lval.e_const != 0));
}

/*
  doifdef( flag )
  Process #ifdef if flag == 1, or #ifndef if flag == 0.
*/
int doifdef(int skip,int  flag)
 
{
char sname[NAMESIZE];

  skipblank();
  if (macname(sname) == 0)
  {
    return (0);
  }
  else
  {
    return (setmflag(skip, flag, findmac(sname) != 0));
  }
}

/*
  setmflag( skip, flag, cond )
*/
int setmflag(int skip,int  flag,int  cond)
{
  if (skip)
  {
    s_ifdef[++i_ifdef] = 3;
    return (1);
  }
  else
  {
    s_ifdef[++i_ifdef] = 6;
    return (flag ^ cond);
  }
}

/*
  addmac()
  Add a macro to the macro table.
*/
void addmac()
{
int h;
char * p;
char * pcnt;
char * saveptr;
char sname[NAMESIZE];
char * sptr;

  skipblank();
  if (macname(sname) == 0)
  {
    return;
  }

  /*
   * Save macro name
   */
  h = hash(sname);
  sptr = Gmalloc(100);
  strcpy(sptr + sizeof(char *), sname);
  p = sptr + sizeof(char *) + strlen(sname) +1;
  macltab[(int)sname[0]] = 1;

  if (ch() == '(')
  {
      /*
       * save dummy arguments
       */
    *p++ = 0x80;
    *(pcnt = p++) = 0;
    gch();
    while (1)
	  {
	    skipblank();
	    if (ch() == ')')
	      break;
	    if (macname(sname) == 0)
	    {
	      return;
	    }
	    strcpy(p, sname);
	    ++*pcnt;
	    p += strlen(sname) + 1;
	    skipblank();
	    if (ch() != ',')
	      break;
	    gch();
	  }
    if (ch() != ')')
	  {
	    Illegal("macro function");
	    do_kill();
	    return;
	  }
    gch();
  }

  /*
   * Save macro value
   */
  *((char **) sptr) = machtab[h];
  machtab[h] = sptr;
  skipblank();
  saveptr = mptr;
  if (pp0(lptr))
    {
      pp1(lptr, p);
    }
  else
    {
      strcpy(p, lptr);
    }
  mptr = saveptr;
  gsptr = p + strlen(p) + 1;
 
/* make sure we push it up to an even boundary! */
/*  gsptr += (((long) gsptr) & 0x1); */
  gsptr += sizeof( char *) - ((long)gsptr & (sizeof(char *) -1));
}

/*
  findmac( sname )
  Look up sname in macro table.
*/
char * findmac(char * sname)
{
int h;
char * p;

  h = hash(sname);
  for (p = machtab[h]; p != NULL; p = *((char **) p))
  {
    if (strcmp(sname, p + sizeof(char *)) == 0)
	  {
	    return (p + sizeof(char *));
	  }
  }
  return (NULL);
}

/*
  findarg( sname )
  Look up sname in formal argument table, macarg.  There are fcnt
  symbols in the table.
*/
char * findarg(char * sname)
 
{
int i;
char * p;

  for (i = 0, p = tblptr; i < tbllen; ++i)
  {
    if (strcmp(sname, p) == 0)
	  {
	    return (macarg[i]);
	  }
    p += strlen(p) + 1;
  }
  return (NULL);
}

/*
  domcall( macptr )
  Process a macro call.
*/
int domcall(char * macptr)
{
int acnt;
int c;
int k;
int nparen;
char * p;
char * q;
char * saveptr;
char * sv;

/* char sname[NAMESIZE]; */

  if (ch() != '(')
    {
      Illegal("macro call");
      return (0);
    }
  gch();
  sv = p = q = Gmalloc(100);
  acnt = 0;
  nparen = 0;
  while (1)
  {
    if ((c = ch()) == '(')
	  {
	    *q++ = gch();
	    ++nparen;
	  }
    else if (quote_p(c))
	  {
	    saveptr = mptr;
	    mptr = q;
	    skipquote(c);
	    q = mptr;
	    mptr = saveptr;
	  }
    else if ((c == ',') || (c == ')'))
	  {
	    if (nparen == 0)
	    {
	      gch();
	      *q++ = '\0';
	      macarg[acnt++] = p;
	      p = q;
	      if (c == ')')
	      	break;
	    }
	    else
	    {
	      *q++ = gch();
	      if (c == ')')
		    --nparen;
	    }
	  }
    else if (c == '\0')
	  {
	    if (readline() == 0)
	    {
	      return 0;
	    }
	  }
    else
	  {
	    *q++ = gch();
	  }
  }

  /*
   * compare formal argument count with actual
   */
  if (acnt != (tbllen = *macptr++ & 0xFF))
  {
    if ((tbllen != 0) || (acnt != 1) || (*macarg[0]))
	  {
	    Error("wrong number of arguments");
	    while (acnt < tbllen)
	      macarg[acnt++] = "";
	  }
  }

  /*
   * move pointer past formal argument list
   */
  tblptr = macptr;
  for (k = 0; k < tbllen; ++k)
  {
    while (*macptr++)
	    ;
  }

  saveptr = lptr;
  pp2(macptr, mptr, findarg, keepstr);
  lptr = saveptr;
  gsptr = sv; /* free the temp buffer */
  return (1);
}

/*
  comment()
  Remove comment from line.
*/
void comment()
{
  gch();
  gch();
  while ((ch() != '*') ||(nch() != '/'))
  {
    if (ch() == '\0')
	  {
	    if (readline() == 0)
	    {
	      Error("EOF in comment");
	      return;
	    }
	  }
    else if ( (ch() == '/') && (nch() == '*') )
    {
      Warning("Nested '/*'" );
    }
    else
	  {
	    ++lptr;
	  }
  }
  gch();
  gch();
}


/*
  skipblank()
  Skip blanks and tabs in the input stream.
*/
void skipblank()
{
  while (white_p(ch()))
  {
    gch();
  }
}
/*
  skipquote()
  Copy single or double quoted string from lptr to mptr.
*/
void skipquote(int qchar) 
{
char c;

  keepch(gch());
  while (((c = ch()) !=qchar) && (c != '\0'))
  {
    /* keep escaped char */
    if (c == '\\')
	    keepch(gch());
    keepch(cgch());
  }
  if (c)
  {
    keepch(gch());
  }
}

#ifdef why_bother
/*
  IllSym()
  Print "illegal symbol name" error.
*/
void IllSym()
{
  Illegal("symbol name");
}

#endif

/*
  macname( sname )
  Get macro symbol name.  If error, print message and do_kill line.
*/
int macname(char * sname)
{
#ifdef old_way
  if (issym(sname) == 0)
    {
      IllSym();
      do_kill();
      return (0);
    }
  else
    {
      return (1);
    }
#else
  return ((issym(sname) ||
	   (Illegal("symbol name"), do_kill(), 0)));
#endif
}
