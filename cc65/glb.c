
/*	Level 0 parsing		*/

#include <stdio.h>
#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

/* internal prototypes */
void parseinit(char * tptr);
void outzero(int n);
void constexp(struct expent * lval);
void toomany();

/*external prototypes */
/*symtab.c*/
int  SizeOf(char * tarray);
void encode(char *p,int w);
int  decode(char * p);
/*expr1.c*/
int  hie1(struct expent * lval);
int  expr(int (*func) () ,struct expent * lval);
/*expr3.c*/
void needbrack(int btype);
void outconst(int ctype,int cnst,int ischar);
/*lexer.c*/
void gettok();
 /*
  parseinit()
*/
void parseinit(char * tptr)
{
  int count;
/*  int i;*/
  struct expent lval;
  struct hashent * p;
  struct hashent * q;
  char * str;
  int sz;

   switch ((unsigned char)tptr[0])
    {
    case T_INT:
    case T_UINT:
    case T_CHAR:
    case T_UCHAR:
    case T_PTR:
   
      constexp(&lval);
       
/*      if (tptr[0] == T_CHAR) */
       if (char_t_p(tptr))
	      bytepref();
      else
	      wordpref();

/*      outconst(lval.e_flags, lval.e_const, tptr[0] == T_CHAR); */
      outconst(lval.e_flags, lval.e_const, char_t_p(tptr));
       break;
    case T_ARRAY:
      sz = decode(tptr + 1);
      if ((tptr[4] == T_CHAR) && (curtok == SCONST))
	    {
	      str = &litq[curval];
	      count = strlen(str) + 1;
	  /*
	    outdat(count);
	    i = 0;
	    while (++i <= count) outbyte(*str++);
	    */
	     outbv(str, count);

	      litptr -= count;
	      gettok();
	    }
      else
	    {
	      needbrack(LCURLY);
	      count = 0;
	      while (curtok != RCURLY)
	      {
	        parseinit(tptr + 4);
	        ++count;
	        if (curtok != COMMA)
	  	       break;
	        gettok();
	      }
	      needbrack(RCURLY);
	    }
      if (sz == 0)
	    {
	      encode(tptr + 1, count);
	    }
      else if (count < sz)
	    {
	      outzero((sz - count) * SizeOf(tptr + 4));
	    }
      else if (count > sz)
	    {
	      toomany();
	    }
      break;
    default:
      if (tptr[0] & T_STRUCT)
	    {
	      needbrack(LCURLY);

	  /* jrd hacked this */
 
	      q = (struct hashent *) (((int) decode(tptr)) + (int) gblspace);
	  /* decode offset from glbspace*/
  	      p = q->link;
	      while (curtok != RCURLY)
	      {
	        if (p == NULL)
		      {
		        toomany();
		        return;
		      }
	        parseinit(p->tptr.g);
	        p = p->link;
	        if (curtok != COMMA)
		        break;
	        gettok();
	      }
	      needbrack(RCURLY);
	      while (p != NULL)
	      {
	        outzero(SizeOf(p->tptr.g));
	        p = p->link;
	      }  
	    }
      else
	    {
	      fprintf(stderr, "unknown type: %02x\n", tptr[0]);
	    }
    }
}

void outzero(int n)
{
/*
    outdat(n);
    while (--n >= 0)
	{
	outbyte(0);
	}
*/
  ot("\t.blkb\t");
  outdec(n);
  nl();
}

void constexp(struct expent * lval)
{
  expr(hie1, lval);
  if ((lval->e_flags & E_MCONST) == 0)
    {
      Need("constant expression");
    }
}

void toomany()
{
  Error("too many initializers");
}
