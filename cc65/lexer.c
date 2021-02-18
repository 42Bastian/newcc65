
/* lexer.c - lexical analyzer for compiler */

#include <stdio.h>
#include <ctype.h>
 
/* every time I thing some C weenie someplace might have opted to do
   something in a regular fashion, I get disappointed... */
#include <errno.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"

#define ch()	(*lptr)

static int asm_unkludge();
void gettok();
void under();
void symbol();
void ident();
void rettok(int tok);
void dollar();
void plus();
void minus();
void star();
void slash();
void amp();
void bar();
void bang();
void equal();
void langle();
void rangle();
void caret();
void percent();
int  issym(char * s);
void symname(char * s);
void number();
void pstr();
void qstr();
int  parsech(int chr);
void unkn();

/*rwords.c*/
int Search(char * string);
/*cruft.c*/

int is_digit(int c);
int is_alpha(int c);

/*io.c*/
char cgch();
char gch();
void do_kill();
int readline();

/* cruf.c*/
int matchstr(char * line,char * str);

/*symtab.c*/
struct hashent * addsym(char * sym);

/*preproc.c*/
void preprocess();
void doasm();

/**********************/

void (*stab[128]) () =
{
  unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn,
  unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn,
  unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn,
  unkn, unkn, unkn, unkn, unkn, unkn, unkn, unkn,
  unkn,	          /* <blank> */
  bang,			        /* ! */
  qstr,			        /* " */
  unkn,			        /* # */
  dollar,		       /* $ */
  percent,	       /* % */
  amp,			         /* & */
  pstr,			        /* 'character const' */
  (void *)LPAREN,	/* ( */
  (void *)RPAREN,	/* ) */
  star,			        /* * */
  plus,			        /* + */
  (void *)COMMA,	 /* , */
  minus,		        /* - */
  (void *)DOT,		  /* . */
  slash,		        /* / */
  number, number, number, number, number,	/* 0 - 4 */
  number, number, number, number, number,	/* 5 - 9 */
  (void *)COLON,  /* : */
  (void *)SEMI,   /* ; */
  langle,		       /* < */
  equal,		        /* = */
  rangle,		       /* > */
  (void *)QUEST,  /* ? */
  unkn,           /* @ */
  ident, ident, ident, ident, ident, ident, ident, ident,	/* A - H */
  ident, ident, ident, ident, ident, ident, ident, ident,	/* I - P */
  ident, ident, ident, ident, ident, ident, ident, ident,	/* Q - X */
  ident, ident,
  (void *)LBRACK, /* [ */
  unkn,			        /* \ */
  (void *)RBRACK, /* ] */
  caret,		        /* ^ */
  under,		        /* _ */
  unkn,	          /* ` */
  symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol,	/* a - h */
  symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol,	/* i - p */
  symbol, symbol, symbol, symbol, symbol, symbol, symbol, symbol,	/* q - x */
  symbol, symbol,
  (void *)LCURLY,  /* { */
  bar,			          /* | */
  (void *)RCURLY,  /* } */
  (void *)COMP,
  unkn			         /* <DEL> */
};


/*
  gettok()
  Make next token current token.  And read another token from the
  input stream.
*/
static char * pp_line = ";;";
static char * old_lptr;

static int asm_unkludge()	/* return t if still kludged */
{
  if (asm_kludge > 0)
    {
      if (--asm_kludge == 1)	/* down to 1? */
	{
	  lptr = old_lptr;	/* restore old line */
	  doasm();		/* do the asm stuff */
	  do_kill();
	}
      else
	{
	  lptr = pp_line;
	  return (1);
	}
    }
  return (0);
}

void gettok()
{
/* int (*f) (); */

  curtok = nxttok;
  curval = nxtval;
  if (asm_unkludge())
  {
    curtok = SEMI;
    return;
  }
  /*
  * pick up next token from input stream.
  */
  while (1)
  {
    while (ch() == 0)
    {
      if (readline() == 0)
      {
        nxttok = CEOF;
        return;
      }
      preprocess();
/* kludge added by jrd.  if see a #asm directive coming up,
   force a semicolon token, to terminate previous stmt.  This'll
   lose if you do it in the middle of something like a for stmt.
   big deal... */
      if (matchstr(lptr, "#asm")) /* see a #asm dir? */
      {
        asm_kludge = 3;	/* force 2 semicolons */
        old_lptr = lptr;
        lptr = pp_line;
        break;
      }
    }
    if (ch() == ' ')
      ++lptr;
    else
      break;
  }

  if ( (nxttok = (int) stab[(int)ch()]) & ~0xff)
  {
    (*((int (*) ()) nxttok)) ();
  }
  else
  {
    ++lptr;
  }
}

void under()
{
char * q;
char token[NAMESIZE];

  symname(token);
  if (token[1] == '_')
  {
    if (strcmp(token, "__FILE__") == 0)
 	  {
	    nxtval = litptr;
	    q = fin;
	    while (*q)
	      litq[litptr++] = *q++;
	      
	    litq[litptr++] = '\0';
	    nxttok = SCONST;
	    return;
	  }
    else if (strcmp(token, "__LINE__") == 0)
	  {
	    nxttok = ICONST;
	    nxtval = ln;
	    return;
	  }
  }
  nxtval = (int) addsym(token);
  nxttok = IDENT;
}

void symbol()
{
char token[NAMESIZE];

  symname(token);
  if ( (nxttok = Search(token)) )
  {
    return;
  }
  else
  {
    nxtval = (int)addsym(token);
    nxttok = IDENT;
  }
}

void ident()
{
char token[NAMESIZE];

  symname(token);
  nxtval = (int) addsym(token);
  nxttok = IDENT;
}

/* util fun used by dollar().  set nxttok and bump line ptr */
void rettok(int tok)
{
  nxttok = tok;
  ++lptr;
}

void dollar()
{
int c;

  switch (c = *++lptr)
  {
    case '(':
      rettok(LCURLY);
      break;
    case ')':
      rettok(RCURLY);
      break;
    case '-':
      rettok(COMP);
      break;
    default:
      unkn();
  }
}

void plus()
{
int c;

  switch (c = *++lptr)
  {
    case '+':
      rettok(INC);
      break;
    case '=':
      rettok(PASGN);
      break;
    default:
      nxttok =PLUS;
  }
}

void minus()
{
int c;

  switch (c = *++lptr)
    {
    case '-':
      rettok(DEC);
      break;
    case '=':
      rettok(SASGN);
      break;
    case '>':
      rettok(PREF);
      break;
    default:
      nxttok =MINUS;
    }
}

void star()
{
  if (*++lptr == '=')
  {
    rettok(MASGN);
  }
  else
  {
    nxttok =STAR;
  }
}

void slash()
{
  if (*++lptr == '=')
    {
      rettok(DASGN);
    }
  else
    {
      nxttok = DIV;
    }
}

void amp()
{
int c;

  switch (c = *++lptr)
    {
    case '&':
      rettok(DAMP);
      break;
    case '=':
      rettok(AASGN);
      break;
    default:
      nxttok = AMP;
    }
}

void bar()
{
int c;

  switch (c = *++lptr)
    {
    case '|':
      rettok(DBAR);
      break;
    case '=':
      rettok(OASGN);
      break;
    default:
      nxttok = BAR;
    }
}

void bang()
{
  if (*++lptr == '=')
    {
      rettok(NE);
    }
  else
    {
      nxttok = BANG;
    }
}

void equal()
{
  if (*++lptr == '=')
    {
      rettok(EQ);
    }
  else
    {
      nxttok = ASGN;
    }
}

void langle()
{
int c;

  switch (c = *++lptr)
    {
    case '=':
      rettok(LE);
      break;
    case '<':
      if (*++lptr == '=')
	{
	  rettok(SLASGN);
	}
      else
	{
	  nxttok = ASL;
	}
      break;
    default:
      nxttok = LT;
    }
}

void rangle()
{
int c;

  switch (c = *++lptr)
    {
    case '=':
      rettok(GE);
      break;
    case '>':
      if (*++lptr == '=')
	{
	  rettok(SRASGN);
	}
      else
	{
	  nxttok =ASR;
	}
      break;
    default:
      nxttok = GT;
    }
}

void caret()
{
  if (*++lptr == '=')
    {
      rettok(XOASGN);
    }
  else
    {
      nxttok = XOR;
    }
}

void percent()
{
  if (*++lptr == '=')
    {
      rettok(MOASGN);
    }
  else
    {
      nxttok = MOD;
    }
}

/*
  issym( s )
  Get symbol from input stream or return 0 if not a symbol.
*/
int issym(char * s)
{
/*    extern char	ascii_tab[128]; */

  if (is_alpha(ch()))
    {
      symname(s);
      return (1);
    }
  else
    {
      return (0);
    }
}

/*
  symname( s )
  Get symbol from input stream or return 0 if not a symbol.
*/
void symname(char * s)
 {
/*    extern char	ascii_tab[128]; */
int k;

  k = 0;
  do
  {
  if (k != NAMEMAX)
    {
      ++k;
      *s++ = * lptr++;
    }
  else
    {
      ++lptr;
    }
  } while (is_alpha(ch()) || is_digit(ch()));
  *s = '\0';
}

void number()
{
int base;
char c;
int k;

  k = 0;
  base = 10;

  if (ch() == '0')
  {
   /*
    * gobble 0 and examin next char
    */
    c = *++lptr;
    if (toupper(c) == 'X')
	  {
	    base = 16;
	    ++lptr; /* gobble "x" */
	  }
    else
	  {
	    base = 8;
	  }
  }

  while (1)
  {
    if (is_digit(c = toupper(ch())))
	  {
	    k = k * base + (c - '0');
	  }
    else if (base == 16)
	  {
	    if (c >= 'A' && c <= 'F')
	      c -= 55;
	    else
	      break;		/* not hex */
	    k = k * base + c;
	  }
    else
	    break;			/* not digit */
    ++lptr;			/* gobble char */
  }

  nxtval = k;
  nxttok = ICONST;
}

/*
  pstr( val )
  Parse a character constant.
*/
void pstr()
{
int k;
int c;

  k = 0;
  ++lptr;
  while ((c = cgch()) != 39 && c != '\0')
    {
      k = ((k & 0xFF) << 8) + parsech(c);
    }
  nxtval = k;
  nxttok =CCONST;
}

/*
  qstr( val )
  Parse a quoted string
*/
void qstr()
{
  ++lptr;
  nxtval = litptr;
  nxttok = SCONST;
  while (ch() != '"')
    {
      if (ch() == 0)
	{
	  Error("new-line in string constant");
	  break;
	}
      if (litptr >= 512)
	{
	  fatal("string space exhausted");
	  /*
          while (gch() != '"')
	      if (ch() == 0) break;
          return;
          */
	}
      litq[litptr++] = parsech(gch());
    }
  cgch();
  litq[litptr++] = 0;
}

/* Parse a character.
 * converts \n into EOL, etc.
 */
int parsech(int chr)
{
int c;
int i;
int oct;

  /* pass through unless backslash */
  if (chr != 92)
    return (chr);

  switch (c = ch())
    {
    case 'b':
      c = '\b';
      break;
    case 'f':
      c = '\f';
      break;			/* CLEAR */
    case 'g':
      c = 7;
      break;			/* Bell */
    case 'n':
      c = '\n';
      break;			/* EOL */
    case 't':
      c = '\t';
      break;			/* TAB */
    default:
      if (c >= '0' && c <= '7')
	    {
	    /*
       * octal (yuk) constant
       */
	      oct = c - '0';
	      gch();
	      i = 1;
	      while (i <= 2)
	      {
	        if ((c = ch()) <'0' || c > '7')
		        break;
	        oct = (oct << 3) + c - '0';
	        gch();
	        ++i;
	      }
	    return (oct);
	  }
  }
  cgch();			/* skip code char */
  return (c);
}

void unkn()
{
  fatal("unknown character");
}
