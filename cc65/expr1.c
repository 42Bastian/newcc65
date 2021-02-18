
/*
   expression parser, part 1
*/
/* last changes :
   YY/MM/DD   who
   97/12/10   42BS    type in expression (testax instead of tstax :( )

*/
#include <stdio.h>
#include <string.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

extern char * lsptr;

/* internal prototypes */
void needtok(int token,char * msg);
void needcol();
void getoprnd(int (*func)(),struct expent * lval);
int  isconst(struct expent * lval1,struct expent * lval2,char * svptr);
int  const0(struct expent * lval);
void expression(int testing);
int  expr(int (*func) () ,struct expent * lval);
int  hie0(struct expent * lval);
int  hie1(struct expent * lval);
void opeq(void (*func)(),struct expent * lval,int k);
int  hieQuest(struct expent * lval);
int  hieOr(struct expent * lval);
int  hieAnd(struct expent * lval);
static int kcalc(int tok,int val1,int val2);
int  op_assoc(int tok,struct op_alist * ops);
int  hie_internal(struct op_alist * ops,struct expent * lval,int (*hienext)() );
int  hie2(struct expent * lval);
int  hie3(struct expent * lval);
int  hie4(struct expent * lval);
int  hie5(struct expent * lval);
static void hie6_internal(int isptr1,struct expent * lval,void (*gen) () );
int  hie6(struct expent * lval);

/* external prototypes */

/* expr2.c*/
int hie7(struct expent * lval);
/*expr3.c*/
void exprhs(int k,struct expent *  lval);
void scale(int n);
void liconst(int cnst);
void store(struct expent * lval);
int getlabel();

/*symtab.c*/
int SizeOf(char * tarray);
/*lexer.c*/
void gettok();
/*io.c*/
void flushout();
  
/*
  needtok( token, msg )
  Token is expected next.  Print "missing <msg>" if not found.
*/
void needtok(int token,char * msg)
{
  if (curtok == token)
  {
    gettok();
    return;
  }
  else
    Missing(msg);
}

/*
  needcol()
  Colon enforcer.
*/
void needcol()
{
  needtok(COLON, "colon");
}

/*
  getoprnd( func, lval )
  Subroutine of commonly executed sequence of statements;
*/
void getoprnd(int (*func)(),struct expent * lval)
{
  gettok();
  push();
  exprhs((*func) (lval), lval);
}

/*
  isconst( lval1, lval2 )
  Test to see if both lval1 and lval2 are constant expressions.
*/
int isconst(struct expent * lval1,struct expent * lval2,char * svptr)
{
  if (lval1->e_flags == E_MCONST && lval2->e_flags == E_MCONST)
    {
      popsp();
      outqi = svptr;
      return (1);
    }
  else
    {
      return (0);
    }
}

/* added by jrd.  return T if value describes constant 0 */
int const0(struct expent * lval)
{
  return ((lval->e_flags == E_MCONST) && (lval->e_const == 0));
}

/*
  expression(int testing)
  testing = 1 => we need CC !
  P = value of expression.

*/
void expression(int testing)
{
struct expent lval;

  bzero(&lval, sizeof(lval));
  exprhs(expr(hie0, &lval), &lval);
  if (! testing )
    return;

  if (!(lval.e_test & E_CC) || (lval.e_test & E_TEST) )
    ol("\tjsr\ttstax\t;42BS hacked it");
}

/*
  expr( func, lval )
  Expression parser; func is either hie0 or hie1.
*/
int expr(int (*func) () ,struct expent * lval)
{
int k;
char * locptr;
int savsp;

  savsp = oursp;
  locptr = lsptr;

/*  flushout(); */ /*42BS*/

  k = (*func) (lval);
  lsptr = locptr;
 
  if (savsp != oursp)
    {
      char errbuf[100];

      sprintf(errbuf, "oursp [%d] != savsp [%d]", oursp, savsp);
      Error(errbuf);
    }
  return (k);
}

/*
  hie0( lval )
  Parse comma operator.
*/
int hie0(struct expent * lval)
{
int k;

  k = hie1(lval);
  while (curtok == COMMA)
    {
      gettok();
      k = hie1(lval);
    }
  return (k);
}

/*
  hie1( lval )
  Parse first level of expression hierarchy.
*/
int hie1(struct expent * lval)
{
void (*func) ();
int k;

  k = hieQuest(lval);
    switch (curtok)
    {
    case RPAREN:
    case SEMI:
      return (k);
    case ASGN:
      {
    	struct expent lval2;

    	gettok();
    	if (k == 0)
	    {
	      needlval();
	      return (0);
	    }
	    if (lval->e_flags & E_MEXPR)
	      push();

    	exprhs(hie1(&lval2), &lval2);

	    store(lval);
	    lval->e_flags = E_MEXPR;

/* if either the source expression of the target generated condition
   codes, set the cond-codes bit in the returned expr */
/*	lval->e_test |= (lval2.e_test & E_CC);*/
    	return (0);
    }
    case PASGN:
      func = add;
      break;
    case SASGN:
      func = sub;
      break;
    case MASGN:
      func = mult;
      break;
    case DASGN:
      func = _div;
      break;
    case MOASGN:
      func = mod;
      break;
    case SLASGN:
      func = asl;
      break;
    case SRASGN:
      func = asr;
      break;
    case AASGN:
      func = and;
      break;
    case XOASGN:
      func = xor;
      break;
    case OASGN:
      func = or;
      break;
    default:
      return (k);
    }
  opeq(func, lval, k);
  return (0);
}

/*
  opeq( func, lval, k)
  Process "op=" operators.
*/
void opeq(void (*func)(),struct expent * lval,int k)
{
/* char * ptr;*/
struct expent lval2;

  gettok();
  if (k == 0)
    {
      needlval();
      return;
    }
  if (lval->e_flags & E_MEXPR)
    push1();

  exprhs(k, lval);
  push();
  exprhs(hie1(&lval2), &lval2);

  if (func == add || func == sub)
    if (lval->e_tptr[0] == T_PTR)
	    scale(SizeOf(lval->e_tptr + 1));

  (*func) ();
  store(lval);

  lval->e_flags = E_MEXPR;
}

/*
  hieQuest( lval )
  Parse "lvalue ? exp : exp"
*/
int hieQuest(struct expent * lval)
{
  int k;

  k = hieOr(lval);
  if (curtok != QUEST)
    {
      return (k);
    }
  else
    {
      int labf;
      int labt;

      gettok();   
      exprhs(k, lval);
      if (! (lval->e_test & E_CC) || (lval->e_test & E_TEST) )
      	tst();

      labf = getlabel();
      falsejump(labf, 0);
      expression(0);
      labt = getlabel();
      needcol();
      jump(labt);
      outcdf(labf);
      expression(0);
      outcdf(labt);
      lval->e_flags = E_MEXPR;
      return (0);
    }
}

/*
  hieOr( lval )
  Process "exp || exp".
*/
int hieOr(struct expent * lval)
{
  int k;

  k = hieAnd(lval);		/* do higher-precedence exprs */
  if (curtok != DBAR)		/* if not ||, return */
    {
      return (k);
    }
  else
    {
      int lab1;			/* labl to exit expr w/ 1 */
      int lab2;			/* labl to exit expr w/ 0 */
      struct expent lval2;
      int loadk;		/* flag if load 1 at end */

      loadk = ((lval->e_test & E_LOGL) == 0); /* need to load 1 if not already */
      exprhs(k, lval);		/* get first expr */
      lab2 = getlabel();	/* get internal label.  for	tests, this is the only label */
      truejump(lab1 = getlabel(), 0); /* get lab and gen 1st bne to it */
      while (curtok == DBAR)	/* while there's more expr */
    	{
	      gettok();		/* skip the || */
	      exprhs(hieAnd(&lval2), &lval2); /* get a subexpr */
	      loadk |= ((lval2.e_test & E_LOGL) == 0); /* need to load 1 if not already */
	      if (curtok == DBAR)	/* if still more, ... */
	      {
	        truejump(lab1, lval->e_test & E_XINV); /* bne out */
	      }
	      else
	      {
	        if (loadk)	/* only generate branch around load if we're loading */
		        falsejump(lab2, lval->e_test & E_XINV);	/* else beq out */
	      }
	    }
      outcdf(lab1);		/* define lab1 */
      if (loadk)		/* if need a load-const 1 ... */
	    {
	      konst1(1);		/*  ... do it */
      }  
      outcdf(lab2);		/* define lab2 */
      lval->e_flags = E_MEXPR;
      return (0);
    }
}

/*
  hieAnd( lval )
  Process "exp && exp"
*/
int hieAnd(struct expent * lval)
{
  int k;


  k = hie2(lval);
  if (curtok != DAMP)
    {
      return (k);
    }
  else
    {
      int lab;
      struct expent lval2;

      exprhs(k, lval);
      falsejump(lab = getlabel(), 0); 
      while (curtok == DAMP)
	    {
	      gettok();
	      exprhs(hie2(&lval2), &lval2);
	      falsejump(lab, 0);	/* zzz finish later */
	    }
      konst1(1);  /* = ldax #1 */

      outcdf(lab);
      lval->e_flags = E_MEXPR;
      return (0);
    }
}

/* utils used by hie2, 3, and 4.  it's the guts of those 3 guys,
   extracted and parameterized */

static int kcalc(int tok,int val1,int val2)
{
  switch (tok)
    {
    case BAR:
      return (val1 | val2);
    case EQ:
      return (val1 == val2);
    case XOR:
      return (val1 ^ val2);
    case AMP:
      return (val1 & val2);
    case ASR:
      return (val1 >> val2);
    case ASL:
      return (val1 << val2);
    case STAR:
      return (val1 * val2);
    case DIV:
      return (val1 / val2);
    case MOD:
      return (val1 % val2);
 
    default:
      printf("Internal error: kcalc: got token %X\n", tok);
 
    }
return 0;
}

int op_assoc(int tok,struct op_alist * ops)
 
{
  while (ops->tok && (ops->tok != tok))
    ++ops;
  return ((int)ops->gen);
}

/* helper fun for various hie frobs */

int hie_internal(struct op_alist * ops,struct expent * lval,int (*hienext)() )
/*
struct op_alist * ops;		  alist of ops we'll take here  
struct expent * lval;		    parent expr's lval  
int (*hienext) ();	        next higher level parser
*/
{
  int k;

  k = (*hienext) (lval);
  if (!op_assoc(curtok, ops))	/* sufficient to test if it's there... */
    {
      return (k);
    }
  else
    {
      struct expent lval2;
      char * svptr;
      int (*gen) ();
      int this_tok;

      svptr = outqi;
      exprhs(k, lval);
      while (1)
	{
	  if ( (gen = (void *)op_assoc((this_tok = curtok), ops)) )
	    {
	      getoprnd(hienext, &lval2);
	      if (isconst(lval, &lval2, svptr))
		{
		  liconst(lval->e_const =
			  kcalc(this_tok, lval->e_const, lval2.e_const));
		  lval->e_test &= ~E_CC; /* cc not set right */
		}
	      else
		{
		  (*gen) ();
		  lval->e_flags = E_MEXPR;
		  lval->e_test |= E_CC;	/* say we set cc */
		}
	    }
	  else
	    {
	      if (lval->e_flags == E_MCONST)
		{
		  outqi = svptr;
		}
	      return (0);
	    }
	}
    }
}

/*
  hie2( lval )
*/
struct op_alist hie2_ops[] =
{
  {BAR, or},
  {0, 0}
};

int hie2(struct expent * lval)
{
  return (hie_internal(hie2_ops, lval, hie3));
}

/*
  hie3( lval )
*/
struct op_alist hie3_ops[] =
{
  {XOR, xor},
  {0, 0}
};

int hie3(struct expent * lval) 
{
  return (hie_internal(hie3_ops, lval, hie4));
}

/*
  hie4( lval )
*/
struct op_alist hie4_ops[] =
{
  {AMP, and},
  {0, 0}
};

int hie4(struct expent * lval)
{
  return (hie_internal(hie4_ops, lval, hie5));
}

/*
  hie5( lval )
*/
struct op_alist hie5_ops[] =
{
  {EQ, eq},
  {NE, ne},
  {0, 0}
};

int hie5(struct expent * lval)
{
  return (hie_internal(hie5_ops, lval, hie6));
}

/*
  hie6( lval )
  process greater-than type comparators
*/

/* helper fun for hie6.  Where's FLET when you need it? */
static void hie6_internal(int isptr1,struct expent * lval,void (*gen) () )
/*
int isptr1;			      other value is ptr  
struct expent * lval;
int (*gen) ();			  code generator fun  
*/
{
  gettok();
  push();
  exprhs(hie7(lval), lval);
  (*gen) ((isptr1 || (lval->e_tptr[0] & T_UNSIGNED)));
}

int hie6(struct expent * lval)
{
  int k;

  k = hie7(lval);
  if ((curtok < LE) || (curtok > GT))
    {
      return (k);
    }
  else
    {
      struct expent lval2;
      int isptr1;

      exprhs(k, lval);
      isptr1 = lval->e_tptr[0] & T_UNSIGNED;
      lval->e_test = E_LOGL | E_CC; /* say we returned a logl val, and set cc */
      while (1)
	{
	  switch (curtok)
	    {
	    case LE:
	      hie6_internal(isptr1, &lval2, le);
	      break;
	    case GE:
	      hie6_internal(isptr1, &lval2, ge);
	      break;
	    case LT:
	      hie6_internal(isptr1, &lval2, lt);
	      break;
	    case GT:
	      hie6_internal(isptr1, &lval2, gt);
	      break;
	    default:
	      lval->e_flags = E_MEXPR;
	      return (0);
	    }
	}
    }
}
