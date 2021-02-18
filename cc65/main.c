
/* CC65 main program */

#define VERSION 2.1.1

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef UNIX
#  include <unistd.h>
#endif

#include "cclex.h"
#include "cc65.h"
#include "error.h"
#include "code-65.h"

#define pathopen pathopen

/* internal prototypes */
#ifdef pathopen
FILE * pathopen(char * name,char *  ext,int force, char * mode, char * truename);
#else
char * frob_name(char * name,char * ext,char * buf);
#endif

int compile();
void parse();
struct hashent * declare(char * ptyp,int type,struct hashent * sadr);
struct hashent * decl(char ** ptyp);
int getsclass(int lv,int dflt);
int gettype(int dflt,struct hashent ** sptr);
void declenum();
int declstruct(struct hashent * last,int strtype);
void ptype(struct hashent * psym,char * tarray);
void dumplits();
void clout();
void print_usage();

/* external prototypes */
/* version.c */
void version();

/*lexer.c*/
void gettok();


/*stmt1.c*/
void ns();

/* symtab.c */
void addsfld(struct hashent * psym,char * tarray,int offset,struct hashent * sn,struct hashent *son);
int  addstag(struct hashent * psym,int sz);
void addglb(struct hashent * psym,char * tarray,int sc);
void eraseloc();
void dumpglbs();
void dumploc(struct hashent * pfunc);
void dumpnams();
void dumpstruct();
void encode(char *p,int w);
int  decode(char * p);
int  SizeOf(char * tarray);
struct hashent * findsym(char *s);
int hash(register char * s);


/*expr1.c*/
void needtok(int token,char * msg);

/*expr3.c*/
int getlabel();
void needbrack(int btype);

/*io.c*/
void flushout();
void do_kill();

/*cruft.c*/
int cclose(FILE * x);

/*glb.c*/
void constexp(struct expent * lval);
void parseinit(char * tptr);

/*preproc.c*/
void addmac();
/*function.c*/
void newfunc(struct hashent * psym);
void funargs();

/*copyleft.c*/
void print_copyleft();


/*****************/
int atari;
int debug;
int stats;
int htabstats;
int maxloc = 0;
int tmpx, tmpy, tmpz;
extern int outcnt;
int register_base; 
int interrupt;

struct hashent * decl();
struct hashent * declare();

/* char	openerr[] = "Couldn't open: "; */

#ifdef pathopen

/* util function for opening files */
FILE * pathopen(name, ext, force, mode, truename)
char * name;		/* filename */
char * ext;		  /* default extension */
int force;		  /* force extension */
char * mode;	 	/* open mode */
char * truename;	/* resultant pathname */
{
FILE * f;
char * extp;

  strcpy(truename, name);
  if (!(extp = (char *)strchr(truename, '.')))
    strcpy(truename + strlen(truename), ext);
  else if (force)
    strcpy(extp, ext);
  if (!(f = (FILE *)fopen(truename, mode)))
    {
      PError("Can't open ", truename);
//      PError(" mode ", mode);
//      PError(" Giving up");
      exit(-1);
    }
  return(f);
}

#else
char * frob_name(name, ext, buf)
char * name;
char * ext;
char * buf;
{
char *p;

 if(!name)
   return (char *)0;

  p = (char *)strchr(name, '.');


  if (ext)
    {
      if (p)
	p[0] = 0;
    }

  strcpy(buf, name);
  if (ext)
    strcat(buf, ext);
  return(buf);
}

#endif /*ifdef pathopen */

/* static char argbuf[128]; */
static char * argp;
 
char in_name[256];
char out_name[256];

char * symbol; 

void my_exit(void);

int main(argc, argv)
int argc;
char *argv[];
{ 
  char *xmacro[16];
  int n_xmacro = -1;
  
  int i;
#ifdef pathopen
  char fout[256];
#else
  char *fout;
#endif

  atexit(my_exit);
  
  /* set up global heap */
 
  /* cross-compiler */
  gblspace = (char *)malloc(GSPACE);
  gblend = gblspace + GSPACE;

  argp = NULL;
  fin = NULL;
 
  atari = 0;
  debug = 0;
  stats = 0;
  htabstats = 0;
 
  if ( argc == 1 )
    print_usage();

  for (i = 1; i < argc; i++)
    {
      if (*(argp = argv[i]) == '-')
	{
	  switch ( /*toupper*/ (argp[1]) )
	    {
	    case 'd':		/* print debug msgs */
	      debug = 1;
	      break;
	    case 'D':
	   	  ++n_xmacro;
	      xmacro[n_xmacro]=argp+2;
	      break;

	    case 's':		/* print storage statistics */
	      stats = 1;
	      break;
	    case 'h':		/* print hashtab stats */
	      htabstats = 1;
	      break;
	    case 'C':		/* include source as comments */
	      source = 1;
	      break;
	    case 'O':		/* optimize */
	      printf("Warning: -O no longer provided !\n"
	             "Use xopt instead !\n");
	      break;
	    case 'v':		/* verbosity */
	      ++verbose;
	      break;
	    case 'I':		/* where to include <> things */
	      incl_dir = argp + 2;
	      break;
	    case 'N':
		    print_copyleft();
	     	exit(0);
	    case '?':
	    case '-':
		    print_usage();
	    default:
	      printf("Invalid option %s\n", argp);
	    }
	}
      else
	{
#ifdef pathopen
	  inp = pathopen(argp, ".c", 0, "r", (fin = argp));
#else
	if(argc>1 && argp)
	 {
	   fin = frob_name(argp, (strchr(argp, '.') ? NULL : ".c"), in_name);
	   inp = fopen(fin, "r");
         }
        else
         fin=0;
#endif /*ifdef pathopen */
	}
    }
  if (!fin)
    {
      Missing("No input-file !");
/*
      inp = stdin;
      fin = "stdin.c";
*/
    }
#ifdef pathopen
  output = pathopen(fin, ".m65", 1, "w", fout);
#else
  fout = frob_name(fin, ".m65", out_name);
  output = fopen(fout, "w");
#endif
/****
  if (!incl_dir)
    incl_dir = (char * )getenv("CC65INCLUDE");
****/
  if (verbose) version();
    
   
  if (compile(n_xmacro,xmacro) != 0)
  {
    clout();
    return 1;
  }
  clout();
  return 0;
}

void my_exit(void)
{
  if ( gblspace ) free(gblspace);
}

/*
	compile()
	Compiler begins execution here
	inp is input fd,
	output is output fd
	(called multiple times from main)
*/
int compile(int n_xmacro,char *xmacro[])
{
/* char dummy; */
int i;

    litptr =			/* clear literal pool */
    oursp =			/* stack ptr (relative) */
    errcnt =			/* no errors */
    eof =			/* not eof yet */
    ncmp =			/* no open compound states */
    macdef =			/* no macros defined yet */
    ifile =			/* index into include file array */
    lovptr =			/* index into local variable array */
    glblbl =			/* initial global label number */
    ln =			/* initial line number */
    0;				/*  ...all set to zero.... */

    register_base = REGISTER_BASE;
 

  filetab[0].f_iocb = inp;


  gsptr = gblspace;
  lsptr = locspace;
  wqptr = wq;			/* clear while queue */

  outqi = outq;
  outqsz = OUTQSZ;
  glvptr = NULL;

  i_ifdef = -1;
  litlab =			/* hotwire literal pool label */
  nxtlab = 1;

 
  bzero(machtab, 256);
  bzero(macltab, 256);
  bzero(htab, 256);
  
  do_kill();			/* empty input line */
  
 /* 
   do -Dmacro
 */
 
  for (i = n_xmacro ; i>= 0 ; --i)
  {
  int h;
  char * sptr;
  
 	h = hash(xmacro[i]);
  	sptr = Gmalloc(100);
  	strcpy(sptr + sizeof(char *),xmacro[i]);
  	macltab[(int)(*xmacro[i])] = 1;
  	machtab[h] = sptr;
  }
  
  /*
     process ALL input
  */
  parse();

 
  /*
     Dump global symbols
  */
  dumpglbs();

  /*
     Dump struct/unions
  */
  dumpstruct();
 

  /*
     Dump literal pool.
  */
  dumplits();

  /*
     Dump external names.
  */
  dumpnams();

 
  if (stats)
    {
    struct hashent * p;
    int cnt;

      for (cnt = 0, p = glvptr; p != NULL; p = p->link)
	++cnt;
				
      printf("\n%d globals, space used: %d bytes\n", cnt, gsptr - gblspace);
      printf("Max local space used: %d bytes\n", maxloc);
      printf("Literal space used: %d bytes\n", litspace);
      printf("Object file: %d bytes\n", outcnt);
      printf("hash: %d calls, %d bytes\n", tmpx, tmpy);
    }
  if (htabstats)
    {
    int i;
    struct hashent * p;
    char * q;

      printf("\n\nSymbol Hash Table Summary\n");
      for (i = 0; i < HTABSZ; ++i)
	{
	  printf("%3d : ", i);
	  if (htab[i])
    	  {
      	    for (p = htab[i]; p != NULL; p = p->ptr)
	    {	
	      printf("%s ", p->name);
	    }
      	    printf("\n");
    	  }
	else
    	  {
      	  printf("empty\n");
    	  }
	}
      printf("\n\nMacro Hash Table Summary\n");
      for (i = 0; i < HTABSZ; ++i)
	{
	printf("%3d : ", i);
	if (machtab[i])
    	{
      	for (q = machtab[i]; q != NULL; q = *((char ** )q))
	{
	printf("%s ", q + sizeof(char *));
	}
      printf("\n");
    	}
	else
 	{
      	printf("empty\n");
    	}
	}
    }
 
  ersum();
  return errcnt;
}

/*
	parse()
	Process all input text

	At this level, only static
	declarations,
	defines, includes, and function
	definitions are legal....
*/
void parse()
{
  int comma;
  struct hashent * psym;
  struct hashent * sadr;
  int sc;
  char tarray[MAXTYPELEN];
  int tmpsc;
  int type;

  gettok();			/* "prime" the pump */
  gettok();
  while (curtok != CEOF)
  {
    sc = getsclass(0, SC_GLOBAL | SC_STATIC );

    type = gettype(T_INT, &sadr);
  
    if (curtok == SEMI)
    {
      gettok();
      continue;
    }
    comma = 0;
    while (1)
    {
      absdecl = 0;
      if ((psym = declare(tarray, type, sadr)) == NULL)
      {
        gettok();
        break;
      }
     
      if ( (tarray[0] != T_FUNC) &&
           ((sc & SC_DEFAULT) || !(sc & SC_GLOBAL)) )
      {
        tmpsc = sc | SC_DEFINED | SC_STORAGE ;
      }
      else
      {
        tmpsc = sc;
      }
      addglb(psym, tarray, tmpsc);
      
/* 42BS ... */      
      if ( curtok == AT )
      {
        struct expent lval;
        void constexp(struct expent *);
        
        gettok();
        constexp(&lval);
        psym->flag.g = SC_DEFINED|SC_GLOBAL;
        outabsdecl(psym->name,(int) lval.e_const);
      }
      else
        if (tmpsc & SC_STORAGE && !(tmpsc & SC_EXTERN))
        {
 //         outgblc((char *)psym->data.g);	/* -- jrd */

          if (curtok == ASGN)
          { 
 
		         segdata();
	           outgblc((char *)psym->data.g);
            gettok();
            parseinit(psym->tptr.g);
          }
          else
          {
	           segbss();
	           outgblc((char *)psym->data.g);
            outsp(SizeOf(psym->tptr.g));
	         }
	         segtext();
         
        }
      
        if (curtok != COMMA)
          break;
        gettok();
        comma = 1;
    }

    if (curtok == CEOF)
    {
      break;
    }
    else if (curtok == SEMI)
    {
      gettok();
    }
    else
    {
      /*
        Possible function.
      */
      if (comma)
      {
        Syntax();
      }
      if (tarray[0] != T_FUNC)
      {
        Illegal("function");
      }
      if (psym != NULL)
      {
        newfunc(psym);
      }
      /*
       print local symbols.
      */
      dumploc(psym);

      eraseloc();
    }
  } /* while (curtok != CEOF) */
}

/*
	declare( ptyp, type )
	Construct a type array.
*/
struct hashent * declare(char * ptyp,int type,struct hashent * sadr)
{
struct hashent * psym;

  psym = decl(&ptyp);
   
  if (psym && type & T_STRUCT) // 42BS
    {
/* jrd hacked this part */

      psym->typeptr = sadr;

      encode(ptyp, (int )sadr - (int )gblspace); /* encode offset in glb mem */

//     printf("declare : %s of type: struct %s\n",psym->name,psym->typeptr->name);

      *ptyp |= type;
      ptyp += 3;
    }
  else
    {
      *ptyp++ = type;
    }
  *ptyp = '\0';
  return (psym);
}

/*
	decl()
	Process declarators.
*/
struct hashent * decl(char ** ptyp)
{
struct expent lval;
struct hashent * psym;
int sz;

  if (curtok == STAR)
    {
      gettok();
      psym = decl(ptyp);
      *(*ptyp)++ = T_PTR;
      return(psym);
    }
  else if (curtok == LPAREN)
    {
      gettok();
      psym = decl(ptyp);
      needbrack(RPAREN);
    }
  else
    {
      if (absdecl)
	{
	  psym = NULL;
	}
      else if (curtok == IDENT)
	{
	  gettok();
	  psym = (struct hashent * )curval;
	}
      else
	{
	  Syntax();
	  return(NULL);
	}
    }

  while (curtok == LBRACK || curtok == LPAREN)
    {
      if (curtok == LPAREN)
	{
	  gettok();
	  funargs();
	  *(*ptyp)++ = T_FUNC;
	}
      else
	{
	  gettok();
	  sz = 0;
	  if (curtok != RBRACK)
	    {
	      constexp(&lval);
	      sz = lval.e_const;
	    }
	  needbrack(RBRACK);
	  *(*ptyp)++ = T_ARRAY;
	  encode(*ptyp, sz);
	  *ptyp += 3;
	}
    }
  return(psym);
}

/*
	getsclass( lv, dflt )
	Process "<storage-class>"
*/
int getsclass(int lv,int dflt)
{
  switch (curtok)
    {
    case EXTERN:
      {
    	 gettok();
	    return(SC_EXTERN | SC_STATIC );
      }
    case STATIC:
      { 
    	gettok();     
	    return(SC_STATIC);
      }
    case ZSTATIC:
      {
        if ( lv == 0 )
	        Error("zstatic not allowed outside functions !");
		
        gettok();
        return (SC_ZSTATIC);
	    }
    case AUTO:
      {
	    gettok();
	    if (lv == 0)
	    {
	      Warning("auto not allowed here");
	      return(SC_STATIC);
	    }
	    return(SC_STACK);
      }
    case REGISTER:
      {
	    gettok();
	    return (SC_REGISTER);
      }
    default:
      return(dflt | SC_DEFAULT);
    }
}
/*
  gettype( dflt ) Process "<type>"
*/
int gettype(int dflt,struct hashent ** sptr)
{
  int strtype;
  int sz;
  *sptr = NULL;
 
  switch (curtok)
    {
    case CHAR:
      gettok();
      return(T_CHAR);
    case LONG:
      Warning("long == short");
      /* fall through */
    case SHORT:
      if (nxttok == INT)
        gettok();
      /* fall through */
    case INT:
      gettok();
      return(T_INT);
    case UNSIGNED:
      gettok();
      if (curtok == CHAR)
      {
        gettok();
        return(T_UCHAR);
      }
      if (curtok == INT)
        gettok();
      return(T_UINT);
    case STRUCT:
    case UNION:
      strtype = curtok == STRUCT ? T_STRUCT : T_UNION;
      gettok();
      if (curtok == IDENT)
      {
        *sptr = (struct hashent * )curval;
        gettok();
      }
      else
      {
        *sptr = (struct hashent * )Gmalloc(sizeof(struct hashent));
        bzero(*sptr, sizeof(struct hashent));
      }
     
      sz = declstruct(*sptr, strtype);
       
      addstag(*sptr, sz);
  
      return(strtype);
 
    case ENUM:
      gettok();
      needtok(IDENT, "tag");
      declenum();
      return(T_INT);
 
    default:
      if (dflt < 0)
      {
        Missing("type");
      }
      return(dflt);
    }
}

 
/*
	declenum()
	Process body of enum.
*/
void declenum()
{
int enumval;
struct expent lval;
struct hashent * psym;

  if (curtok != LCURLY)
    {
      return;
    }
  gettok();
  enumval = 0;
  while (curtok != RCURLY)
    {
      if (curtok != IDENT)
      {
        Missing("identifier");
        continue;
      }
      psym = (struct hashent * )curval;
      gettok();
      if (curtok == ASGN)
      {
        gettok();
        constexp(&lval);
        enumval = lval.e_const;
      }
      addglb(psym, type_int, SC_ENUM);
      psym->data.g = enumval++;
      if (curtok != COMMA)
	break;
      gettok();
    }
  needbrack(RCURLY);
}
/*
	declstruct()
	Process body of struct/union declaration.
*/
int declstruct(struct hashent * last,int strtype)
{
int offset;
struct hashent * psym = NULL;
struct hashent * sadr = NULL;
struct hashent * sn = NULL; /* struct-name ?? 42B*/
// static int structcnt = 0;

int sz;
char tarray[MAXTYPELEN];
// char *help;
int type;
// struct hashent *ps;

  sn = last;

  if (curtok != LCURLY)
    {
      return(0);
    }
    
  gettok();
  sz = 0;
  while (curtok != RCURLY)
  {
    type = gettype(-1, &sadr);
      
    while (1)
    {
	    absdecl = 0;
	    psym = declare(tarray, type, sadr);

	    if ( psym && psym->flag.g )
	    {
	      struct hashent * psym2 = psym;
	      struct hashent ** hptr;

 //printf("Main:Multiused component (%s)\n",psym->name);

	      hashval = hash(psym->name);
	      psym = (struct hashent *) Gmalloc(sizeof(struct hashent) + strlen(psym2->name));
	      bzero(psym, sizeof(struct hashent));
	      strcpy(psym->name, psym2->name);
	  /*
	  Add a symbol entry to the hash table.  hashval set by findsym above.
	  */
	      hptr = &htab[hashval];
	      psym->ptr = *hptr;
	      *hptr = psym;
	    }
  
//    psym = declare(tarray, type, sadr);

      last->link = psym;
      last = psym;

//    printf("main: %s of struct %s ",psym->name,sn->name);
//    if (sadr) printf(" type struct %s",sadr->name);
//    printf("\n");

      offset = strtype == T_STRUCT ? sz : 0;
      addsfld(psym, tarray, offset,sn,sadr);
      offset = SizeOf(tarray);
      if (strtype == T_STRUCT)
      {
	      sz += offset;
      }
      else
      {
	      if (offset > sz)
		      sz = offset;
      }

      if (curtok != COMMA)
	      break;
      gettok();
	  }
    ns();
  }
  gettok();
  return(sz);
}


/*
	ptype( psym, tarray )
	Output translation of type array.
*/
void ptype(struct hashent * psym,char * tarray)
{
  char * p;

  printf("%s: ", psym ? psym->name : "<>");
  for (p = tarray; *p != '\0'; ++p)
  {
    if (*p & T_STRUCT)
    {
/* jrd here ...
 printf("struct/union of %s\n",
 ((struct hashent * )decode(p))->name);	*/
      printf("struct/union of %s\n",psym->typeptr->name);
//        ((struct hashent * )(((int) decode(p) + (int) gblspace)))->name);
      p += 2;
    }
    else
    {
      if (*p & T_UNSIGNED)
      {
        printf("unsigned ");
      }
      switch (*p)
      {
      case T_CHAR:
      case T_UCHAR:
        printf("char\n");
        break;
      case T_INT:
      case T_UINT:
        printf("int\n");
        break;
      case T_SHORT:
        printf("short\n");
        break;
      case T_LONG:
        printf("long\n");
        break;
      case T_FLOAT:
        printf("float\n");
        break;
      case T_DOUBLE:
        printf("double\n");
        break;
      case T_PTR:
        printf("ptr to ");
        break;
      case T_ARRAY:
        printf("array[%d] of ", decode(p + 1));
        p += 3;
        break;
      case T_FUNC:
        printf("function returning ");
        break;
      default:
        printf("unknown type: %X\n", *p);
      }
    }
  }
}
/*
   Dump the literal pool
*/
void dumplits()
{
/* int k; */

  /* if nothing there, exit...*/
  if (litptr == 0) return;
  
  /* print literal label */
  segdata();
  outcdf(litlab);
  
  /* data for next n bytes */
/*	outdat(litptr);	*/

  /* init an index... */
  /*	k = 0; */
  /*	while (k < litptr)	*/ /* 	to loop with */
  /*	    outbyte(litq[k++]);				*/
  outbv(litq, litptr);
  segtext();

  litspace += litptr; /* account for space */
  litlab = getlabel();		/* get next lit pool label */
  litptr = 0;
}

/*
	clout()
	Flush the output buffer and close the output file.
*/
void clout()
{
/*
    char *	p;

    for (p = outq; p != outqi; ++p) cout(*p);
*/
  flushout();
/*    fclose(output); */
  cclose(output);
}

void print_usage()
{
  printf("\nCC65 v1.1p4 - (Ported to *nix by Intruder  1993)\n"
           "     v2     - cleaned version by 42Bastian Schick 1996/97\n");
  version();
  
  printf("\n  -d        Debugging on\n"
         "  -s        Print storage info\n"
         "  -h        Print hashtab stats\n"
         "  -C        Include source as comment\n"
//         "	-O   	  Optimize code\n"
         "  -Dsymbol  define Symbol\n" 
         "  -v        Verbose mode\n"
         "  -I <fn>   Specify include directory\n"
         "  -?        This help message\n"
         "  -N        print copyright notice\n");
exit(0);
}
