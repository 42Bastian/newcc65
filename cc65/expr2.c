
/*
   Expression parsing, part 2
*/

#include <stdio.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

/*internal prototypes */
int hie7(struct expent * lval);
int hie8(struct expent * lval);
int hie9(struct expent * lval); 
static void hie10a(struct expent * lval,void (*incptr) (),void (*incint) () );
static void hie10b(struct expent * lval,int k,void (*incptr) (),void (*incint) () );
static void hie10c(int tok,struct expent * lval);
int hie10(struct expent * lval);

/* external prototypes */
/*expr1.c*/
int hie_internal(struct op_alist * ops,struct expent * lval,int (*hienext)() );
int  isconst(struct expent * lval1,struct expent * lval2,char * svptr);
void getoprnd(int (*func)(),struct expent * lval);
/*expr3.c*/
int hie11(struct expent * lval);
void exprhs(int k,struct expent *  lval);
void increm(int n);
void decrem(int n);
void scale(int n);
void cscale(struct expent * lval,int n);
void liconst(int cnst);
void needbrack(int btype);
void store(struct expent * lval);
/*main.c*/
struct hashent * declare(char * ptyp,int type,struct hashent * sadr);
int gettype(int dflt,struct hashent ** sptr);
/*symtab.c*/
int SizeOf(char * tarray);
int PSizeOf(char * tptr);
/*lexer.c*/
void gettok(); 

/*
  hie7( lval )
  Parse << and >>.
*/
struct op_alist hie7_ops[] =
{
  {ASL, asl},
  {ASR, asr},
  {0, 0}
};

int hie7(struct expent * lval)
{
  return (hie_internal(hie7_ops, lval, hie8));
}

/*
  hie8( lval )
  Process + and - binary operators.
*/
int hie8(struct expent * lval)
{
  int k;

  k = hie9(lval);
  if ((curtok != PLUS) && (curtok != MINUS))
    {
      return (k);
    }
  else
    {
      struct expent lval2;
      char * svptr;
      char * tptr1;
      char * tptr2;

      svptr = outqi;
      exprhs(k, lval);
      while (1)
	{
	  if (curtok == PLUS)
	    {
	      getoprnd(hie9, &lval2);
	      tptr1 = lval->e_tptr;
	      tptr2 = lval2.e_tptr;
	      if ((tptr1[0] & T_POINTER) &&
		  (tptr2[0] & T_INTEGER))
		{
		  cscale(&lval2, PSizeOf(tptr1));
		}
	      else if ((tptr1[0] & T_INTEGER) &&
		       (tptr2[0] & T_POINTER))
		{
		  swapstk();
		  scale(PSizeOf(tptr2));
		  lval->e_tptr = lval2.e_tptr;
		}
	      if (isconst(lval, &lval2, svptr))
		{
		  liconst(lval->e_const += lval2.e_const);
		}
	      else
		{
		  add();
		  lval->e_flags = E_MEXPR;
		}
	    }
	  else if (curtok == MINUS)
	    {
	      getoprnd(hie9, &lval2);
	      tptr1 = lval->e_tptr;
	      tptr2 = lval2.e_tptr;
	      if ((tptr1[0] & T_POINTER) && (tptr2[0] & T_INTEGER))
		{
		  cscale(&lval2, PSizeOf(tptr1));
		}
	      sub();
	      if ((tptr1[0] & tptr2[0] & T_POINTER) &&
		  (strcmp(tptr1, tptr2) == 0))
		{
		  k = PSizeOf(tptr1);
		  if (k > 1)
		    {
		      push();
		      /*
			immed();
			outdec(k);
		      */
		      konst1(1);  /* ?????? */

		      _div();
		      lval->e_flags = E_MEXPR;
		    }
		  lval->e_tptr = type_int;
		}
	      if (lval->e_flags == E_MCONST && lval2.e_flags == E_MCONST)
		{
		  outqi = svptr;
		  liconst(lval->e_const -= lval2.e_const);
		}
	      else
		{
		  lval->e_flags = E_MEXPR;
		}
	    }
	  else
	    {
	      if (lval->e_flags == E_MCONST)
		{
		  outqi = svptr;
		}
	      else
		{
		  lval->e_flags = E_MEXPR;
		}
	      return (0);
	    }
	}
    }
}

/*
  hie9( lval )
  Process * and / operators.
*/

struct op_alist hie9_ops[] =
{
  {STAR, mult},
  {DIV, _div},
  {MOD, mod},
  {0, 0}
};

int hie9(struct expent * lval)
{
  return (hie_internal(hie9_ops, lval, hie10));
}

/*
  hie10( lval )
  Parse unary operators.
*/

/* various internal functions */
static void hie10a(struct expent * lval,void (*incptr) (),void (*incint) () )	/* guts of inc, dec */
/*
struct expent * lval;
int (*incptr) ();	  for inc'ing ptrs  
int (*incint) ();	  for inc'ing ints  
*/
{
int k;


  gettok();
  if ((k = hie10(lval)) == 0)
    {
      needlval();
      return;
    }
  if (lval->e_flags & E_MEXPR)
    push1();
  exprhs(k, lval);
  if (lval->e_tptr[0] == T_PTR)
    {
      (*incptr) (SizeOf(lval->e_tptr + 1));
    }
  else
    {
      (*incint) ();
    }
  store(lval);
  lval->e_flags = E_MEXPR;
}

static void hie10b(struct expent * lval,int k,void (*incptr) (),void (*incint) () )	/* guts of second inc, dec */
{
  gettok();
  if (k == 0)
    {
      needlval();
      return ;
    }
  if (lval->e_flags & E_MEXPR)
    push1();
  exprhs(1, lval);
  save();
  if (lval->e_tptr[0] == T_PTR)
    {
      (*incptr) (SizeOf(lval->e_tptr + 1));
    }
  else
    {
      (*incint) ();
    }
  store(lval);
  rstr();
  lval->e_flags = E_MEXPR;
}

static void hie10c(int tok,struct expent * lval)	/* guts of minus, comp */
{
  int k;

  gettok();
  k = hie10(lval);
  if (lval->e_flags == E_MCONST)
    {
      lval->e_const = (tok == MINUS ? -lval->e_const : ~lval->e_const);
      return ;
    }
  exprhs(k, lval);
  if (tok == MINUS)
    neg();
  else
    com();
  lval->e_flags = E_MEXPR;
  lval->e_test |= E_CC;
}

int hie10(struct expent * lval)
{
  int k;
  char * ptr;
  struct hashent * sadr;
  int type;


  if (curtok != IDENT)
    {
      switch (curtok)
	{
	case INC:
	  {
	    hie10a(lval, increm, inc);
	    return (0);
	  }
	case DEC:
	  {
	    hie10a(lval, decrem, dec);
	    return (0);
	  }
	case MINUS:
	case COMP:
	  {
	    hie10c(curtok, lval);
	    return (0);
	  }
	case BANG:
	  {
	    gettok();
	    k = hie10(lval);	/* decode the expr */
	    exprhs(k, lval);	/* load the value */

#ifdef busted
	    if (lval->e_test)	/* somebody doing a test of this exp? */
	      {
		lval->e_test ^= E_XINV;	/* flip sense of test */
	      }
	    else
	      /* normal expr */
	      {
#ifdef old_cruft
		push();
		konst1(0);
		eq();
#else
		lneg();
		lval->e_test = E_CC;
#endif
	      }
#else
	    lneg();		/* sheesh */
            lval->e_test = E_CC; /* 42BS 26 */
#endif
	    lval->e_flags = E_MEXPR; /* say it's an expr */
	    return (0);		/* expr not storable */
	  }
	case STAR:
	  {
	    gettok();
	    exprhs(hie10(lval), lval);
	    if (((ptr = lval->e_tptr)[0] & T_POINTER) == 0)
	      {
		Illegal("indirection");
	      }
	    else
	      {
		if (ptr[0] == T_ARRAY)
		  {
		    ptr += 4;
		  }
		else
		  {
		    ++ptr;
		  }
		lval->e_tptr = ptr;
	      }
	    lval->e_flags = E_MEXPR;
	    return (1);
	  }
	case AMP:
	  {
	    gettok();
	    k = hie10(lval);
	    if (k == 0)
	      {
		Illegal("address");
		return (0);
	      }
	    ptr = Lmalloc(strlen(lval->e_tptr) + 2);
	    ptr[0] = T_PTR;
	    strcpy(ptr + 1, lval->e_tptr);
	    lval->e_tptr = ptr;
	    return (0);
	  }
	case SIZEOF:
	  {
	    gettok();                 /*        CHAR ............VOID */
	    if ((curtok == LPAREN) && (nxttok >= 20) && (nxttok <= 29))
	      {
		char tarray1[MAXTYPELEN];

		gettok();
		type = gettype(-1, &sadr);
		absdecl = 1;
		declare(tarray1, type, sadr);
		needbrack(RPAREN);
		lval->e_const = SizeOf(tarray1);
	      }
	    else
	      {
		hie10(lval);
		lval->e_const = SizeOf(lval->e_tptr);
	      }
	    lval->e_flags = E_MCONST | E_TCONST;
	    lval->e_tptr = type_int;
	    return (0);
	  }
	default:
/* casting ... */
	  if ((curtok == LPAREN) && (nxttok >= 20) && (nxttok <= 29))
	    {
	      char tarray2[MAXTYPELEN];
	      	      
	
	      gettok();
	      type = gettype(-1, &sadr);
 
	      absdecl = 1;
	      declare(tarray2, type, sadr);
	      needbrack(RPAREN);
	     
	      k = hie10(lval);
	      strcpy(lval->e_tptr = Lmalloc(strlen(tarray2) + 1), tarray2);
	      return (k);
	    }
	}			/* end switch */
    }				/* end curtok != ident */
  k = hie11(lval);
  switch (curtok)
    {
    case INC:
      {
	hie10b(lval, k, increm, inc);
	return (0);
      }
    case DEC:
      {
	hie10b(lval, k, decrem, dec);
	return (0);
      }
    default:
      return (k);
    }
}
