
/*
   Expression parsing, part 3
*/

#include <stdio.h>
#include <string.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

/* internal prototypes */

int hie11(struct expent * lval);
int primary(struct expent * lval);

void scale(int n);
void cscale(struct expent * lval,int n);
void decrem(int n);
void increm(int n);
void store(struct expent * lval);
void exprhs(int k,struct expent *  lval);
void liconst(int cnst);
void lconst(int ctype,int cnst);
void outconst(int ctype,int cnst,int ischar);
void outconstprim(int ctype,int cnst);
void test(int label);
void needbrack(int btype);
void callfunction(int k,struct expent * lval);
int opref(int k,struct expent * lval);
int oppref(int k,struct expent *  lval);
int structref(struct expent *lval);
int getlabel();

/* external prototypes */

/*expr1.c*/
int hie0(struct expent * lval);
int hie1(struct expent * lval);
int  expr(int (*func) () ,struct expent * lval);
void needtok(int token,char * msg);

/*expr2.c*/

/*smytab.c*/
void addglb(struct hashent * psym,char * tarray,int sc);
void addloc(struct hashent * psym,char * tarray,int sc,int offs);
int SizeOf(char * tarray);
void encode(char *p,int w);
struct hashent * findsfld(char * sym,struct hashent * parent);

/*lexer.c*/
void gettok();
/* cruft.c */
int is_power_of_two(int n);
/*
  hie11( lval )
*/
int hie11(struct expent * lval)
{
int k;

  k = primary(lval);
  if ((curtok < LBRACK) || (curtok > PREF))
  {
    return (k);
  }
  else
  {
    struct expent lval2;
    char * svptr;
    char * tptr;
    char * tptr2;

    while (1)
    {
      if (curtok == LBRACK)
      {
        gettok();
        exprhs(k, lval);
        svptr = outqi;
        push();
        /*
        TOS now contains ptr to array elements.
         */
        exprhs(hie0(&lval2), &lval2);
        needbrack(RBRACK);
        if ((tptr = lval->e_tptr)[0] & T_POINTER)
        {
          if (tptr[0] == T_ARRAY)
            lval->e_tptr += 3;
          cscale(&lval2, SizeOf(++lval->e_tptr));
          if ((lval2.e_flags == E_MCONST) &&
              (lval2.e_const == 0))
          {
            popsp();
            outqi = svptr;
            goto end_array;
          }
        }
        else if ((tptr2 = lval2.e_tptr)[0] & T_POINTER)
        {
          if (tptr2[0] == T_ARRAY)
            lval2.e_tptr += 3;
          swapstk();
          scale(SizeOf(++lval2.e_tptr));
          lval->e_tptr = lval2.e_tptr;
        }
        else
        {
          Error("cannot subscript");
        }
        add();
end_array:
        lval->e_flags = E_MEXPR;
        lval->e_test = E_CC; /* 42BS*/
        k = lval->e_tptr[0] != T_ARRAY;
      }
      else if (curtok == LPAREN)
      {
        gettok();
        if (lval->e_tptr[0] != T_FUNC)
        {
          Illegal("function call");
        }
        callfunction(k, lval);
        lval->e_test = E_CC; /* funs always return cond codes */
        lval->e_flags = E_MEXPR;
        ++lval->e_tptr;
        k = 0;
      }
      else if (curtok == DOT)
      {
        k = opref(k, lval);
      }
      else if (curtok == PREF)
      {
        k = oppref(k, lval);
      }
      else
        return (k);
    }
  }
}

/*
  primary( lval )
  This is the lowest level of the expression parser.
*/
int primary(struct expent * lval)
{
  int k;
  struct hashent * psym;
  char * tptr;
  int type;

  lval->e_test = 0;		/* not a test at all, yet */
  if (curtok == IDENT)
  {
    psym = (struct hashent *) curval;
//42BS
//{ struct hashent * ps;
//printf("primary:%s ",psym->name);
//if ( psym->typeptr )
//  printf("of type %s ",psym->typeptr->name);
//if ( psym->s_ptr )
//  printf("in struct %s ",psym->s_ptr);
//printf("\n");
//}
//
    gettok();
    lval->typeptr = psym->typeptr;

  /*
  check local symbol table.
  */
    if (psym->flag.l != 0)
    {
      lval->e_tptr = tptr = psym->tptr.l;
      if (psym->flag.l & SC_STACK)
      {
        lval->e_flags = E_MLOCAL | E_TLOFFS;
      }
      else
      { 
        lval->e_flags = E_MLOCAL | E_TLLAB;
      }
      lval->e_const = psym->data.l;

      if ((tptr[0] == T_FUNC) || (tptr[0] == T_ARRAY))
      {
        return (0);
      }
      return (1);
    }
  /*
  check global symbol table.
  */
    if (psym->flag.g != 0)
    {
      if ((type = psym->flag.g) & SC_STRUCT)
      {
        Error("struct tag/fld cannot be primary");
        return (0);
      }
      lval->e_tptr = tptr = psym->tptr.g;
      lval->e_const = psym->data.g;
      
      if (type == SC_ENUM)
      {
        lval->e_flags = E_MCONST;
        return (0);
      }
      else
      {
        lval->e_flags = E_MGLOBAL | E_MCONST | E_TGLAB;
      }
      if ((tptr[0] == T_FUNC) || (tptr[0] == T_ARRAY))
      {
        return (0);
      }
      return (1);
    }
  /*
  IDENT is either an auto-declared function or an undefined
  variable.
  */
    if (curtok == LPAREN)
    {
  /*
  declare function returning int.
  */
      addglb(psym, type_ifunc, SC_GLOBAL);
      lval->e_tptr = Lmalloc(3);
      strcpy(lval->e_tptr, type_ifunc);
      lval->e_flags = E_MGLOBAL | E_TGLAB;
      lval->e_const = psym->data.g;
      return (0);
    }
    addloc(psym, type_int, SC_STACK, 0);
    lval->e_flags = E_MLOCAL | E_TLOFFS;
    lval->e_tptr = type_int;
    lval->e_const = 0;
    Error("undefined symbol");
    return (1);
  }
  /*
    Character and integer constants.
  */
  if ((curtok == ICONST) || (curtok == CCONST))
  {
    lval->e_flags = E_MCONST | E_TCONST;
    lval->e_tptr = type_int;
    lval->e_const = curval;
    gettok();
    return (0);
  }

  /*
    Process parenthesized subexpression by calling the whole parser
    recursively.
  */
  if (curtok == LPAREN)
  {
    gettok();
    k = hie0(lval);
    needbrack(RPAREN);
    return (k);
  }

  /*
    String literals.
  */
  if (curtok == SCONST)
  {
    char tarray[6];

    lval->e_flags = E_MCONST | E_TLIT;
    tarray[0] = T_ARRAY;
    encode(tarray + 1, strlen(litq + curval) + 1);
    tarray[4] = T_CHAR;
    tarray[5] = '\0';
    lval->e_tptr = Lmalloc(strlen(tarray) + 1);
    strcpy(lval->e_tptr, tarray);
    lval->e_const = curval;
    gettok();
    return (0);
  }
  /*
    Illegal primary.
  */
  Error("invalid expression");
  lval->e_flags = E_MCONST;
  lval->e_tptr = type_int;
  return (0);
}

/*
   true if val1 -> int pointer or int array and val2 not ptr or array.
   used to decide whether to shift value left 1 when indexing.
*/
void scale(int n)
{
int i;
  if (n != 1)
  {
    if ( (i = is_power_of_two(n)) )
    {
      aslprim(i);
    }
    else
    {
      push();
      immed(n);
      mult();
    }
  } 
}

void cscale(struct expent * lval,int n)
{
/*
    if (lval->e_flags == E_MCONST) {
	lval->e_const *= n;
	rmvbyte(4);
	lconst(E_TCONST, lval->e_const);
	}
    else {
*/
  scale(n);
/*	}	*/
}

void decrem(int n)
{
  decprim(n);
}

void increm(int n)
{
  incprim(n);
}
/*
	store( lval )
	Store primary reg into this reference
*/
void store(struct expent * lval)
{
int f;

  f = lval->e_flags;
 
  if (f == 0)
    {
      fatal("e_flags == 0");
    }
 
  if (f & E_MGLOBAL)
    {
      putmem((char *)lval->e_const, lval->e_tptr);
      if (char_t_p(lval->e_tptr))
        lval->e_test |= E_CC;
    }
  else if (f & E_MLOCAL)
    {
      if ((f & E_MCTYPE) == E_TLOFFS)
      {
        putloc(lval->e_const, lval->e_tptr);
        lval->e_test |= E_CC;
     }
      else
    { 
       putlmem(lval->e_const, lval->e_tptr);
       if (char_t_p(lval->e_tptr))
	 lval->e_test |= E_CC;
     }
  }
  else if (f & E_MEXPR)
    {
      putstk(lval);
      lval->e_test |= E_CC;
    }
  else
    {
 
      fatal("trying to store into constant");
 
    }
}

/*
  exprhs( k, lval )
*/
void exprhs(int k,struct expent * lval)
{
  int f;
  void immed();  

  f = lval->e_flags;
  if (k)			                      /* reference storable-p */
  {
    if (f & E_MGLOBAL)	            /* ref to globalvar */
	  {
	    getmem((char *)lval->e_const, lval->e_tptr, lval->e_test);
	    if (char_t_p(lval->e_tptr) || lval->e_test)
	      lval->e_test = E_CC;        /* 42BS */
	  }
    else if (f & E_MLOCAL)	        /* ref to localvar */
	  {
	    if ( f & E_TLOFFS)            /* ref to stack-var */
	    {  
        getloc(lval->e_const, lval->e_tptr);
	      if (char_t_p(lval->e_tptr))
          lval->e_test = E_CC;
      }
	    else                          /* ref to local static */
      {
	      getlmem(lval->e_const, lval->e_tptr, lval->e_test);
	      if (char_t_p(lval->e_tptr) || lval->e_test)
	        lval->e_test = E_CC;      /* 42BS 26 */
	    }
	  }
    else if (f & E_MCONST)	        /* ref to constant */
    {
  	  fatal("Constant with k = 1");
    }
    else
	  {
	    indirect(lval);	/* else must be locative */
	  }
/*	lval->e_test = E_CC;		.. say we set cc (all those do...) */
  } // k == 0
  else if (f == E_MEOFFS) /* reference not storable */
  {
	  push();
	  immed(lval->e_const); /* 42BS */
	  add();
  }
  else if (f != E_MEXPR)
  {
	  lconst(f & E_MCTYPE, lval->e_const);
  }
  /*	lval->e_test &= ~E_CC;	*/	/* say cc not set right */
  if (lval->e_test & E_TEST)	/* we testing this value? */
    tst();			/* yes, force a test */
}


/*
	liconst( const )
	Load primary reg with integer constant.
*/
void liconst(int cnst)
{
  immed(cnst);
}

/*
	lconst( func, ctype, const )
	Load primary reg with some constant value.
*/
void lconst(int ctype,int cnst)
{
  ctype &= E_MCTYPE;
  if (ctype == E_TLOFFS)
    {
      getladr(cnst);
    }
  else
    {
      outconstprim(ctype, cnst);
    }
}

/*
	outconst(ctype, const)
	Output psuedo-op appropriate to type of constant.
*/

void outconstprim(int ctype,int cnst)
{
  switch (ctype & E_MCTYPE)
    {
    case E_TCONST:		/* fixnum constant */
      immed(cnst);
      return;
    case E_TGLAB:		/* some kind of global */
      immedgbl(cnst);
      return;
    case E_TLIT:		/* a literal of some kind */
      immedslt(cnst);
      return;
    case E_TLLAB:
      immedlab(cnst);
      return;
    default: 
      printf("unknown constant: %d\n", ctype & E_MCTYPE);
      fatal("compiler error: unknown constant type");
    }
}
void outconst(int ctype,int cnst,int ischar)
{
  switch (ctype & E_MCTYPE)
    {
    case E_TCONST:		/* fixnum constant */
      outdecnl(cnst);
      return;
    case E_TGLAB:		/* some kind of global */
      outgbl((char *)cnst);
      nl();
      return;
    case E_TLIT:		/* a literal of some kind */
      outslt(cnst);
      return;
    case E_TLLAB:
      outlabnl(cnst);
      return;
    default: 
      printf("unknown constant: %d\n", ctype & E_MCTYPE);
      fatal("compiler error: unknown constant type");
 
    }
}
/*
	test( label )
	Generate code to perform test and jump if false.
*/
void test(int label)
{
int k;
struct expent lval;

  bzero(&lval, sizeof(lval));

  needbrack(LPAREN);

  k = expr(hie0, &lval); /* generate code to eval the expr */
 
  if (lval.e_flags == E_MCONST)
    {
      if (lval.e_const == 0)
	{
	  jump(label);
	  Warning("unreachable code");
	}
      needbrack(RPAREN);
      return;
    }
 
  /* if the expr hasn't set condition
     codes, set the force-test flag */
  if (!(lval.e_test & E_CC))
    lval.e_test |= E_TEST;

  exprhs(k, &lval);		/* load the value as apropriate */
  needbrack(RPAREN);

  if (lval.e_test & E_XINV)
    ol(";;; test inverted");
 
  falsejump(label, lval.e_test & E_XINV);
}

/*
	needbrack( btype )
	Enforce closing bracket type.
*/
void needbrack(int btype)
{
  needtok(btype, "bracket");
}

/*
	callfunction(ptr)
	Perform a function call.  Called from hie11, this routine will
	either call the named function, or if the supplied ptr is zero,
	will call the contents of P.
*/
void callfunction(int k,struct expent * lval)
{
/* char * dptr; */
struct expent lval2;
int nargs;

  nargs = 0;
  if (lval->e_flags & E_MEXPR)
    {
      save();
    }
    
  while (curtok != RPAREN)
   
    {
      exprhs(hie1(&lval2), &lval2);
      push();
      nargs += 2;
      if (curtok != COMMA)
	break;
      gettok();
    }
  needbrack(RPAREN);
  
  if (lval->e_flags & E_MEXPR)
    {
      rstr();
      callstk(nargs);
    }
  else
    {
      call((char *)lval->e_const, nargs);
    }
}

/*
	opref( lval )
	Process . operator.
*/
int opref(int k,struct expent * lval)
{
  if (!(lval->e_tptr[0] & T_STRUCT))
    {
      Need("struct");
    }
  if (!(lval->e_flags & E_MEXPR))
    {
      lconst(lval->e_flags & E_MCTYPE, lval->e_const);
    }
  return (structref(lval));
}

/*
	oppref( k, lval )
	Process -> operator.
*/
int oppref(int k,struct expent *  lval)
{
char * tptr;

  tptr = (char *) lval->e_tptr;
  if ((tptr[0] != T_PTR) || !(tptr[1] & T_STRUCT))
    {
      Need("struct pointer");
    }
  exprhs(k, lval);
  return (structref(lval));
}

/*
	structref( lval )
	Process struct field after . or ->.
*/
int structref(struct expent *lval)
{
struct hashent * psym;
//struct hashent * ps2;

  gettok();
  if (curtok != IDENT)
    {
      Syntax();
      lval->e_tptr = type_int;
      return (0);
    }
  psym = (struct hashent *) curval;
  gettok();

if (lval->typeptr != psym->s_ptr)
{
  psym = findsfld(psym->name,lval->typeptr);
}

//printf("---\n");   
//if (lval->e_const)
//  printf("xpr3:var %s ",lval->e_const);
//if (lval->typeptr)
//  printf("of type  %s",lval->typeptr->name);
//printf("\nxpr3:tag %s of type ",psym->name);
//if ( psym->typeptr)
//  printf("struct %s\n",psym->typeptr->name);
//else
//  printf("standard\n");
//
//if (psym->s_ptr)
//  printf("member in %s\n",psym->s_ptr->name);

  if ((psym->flag.g & 0xFF) != SC_SFLD)
    {
      Need("struct field");
      lval->e_tptr = type_int;
      return (0);
    }

  lval->e_const = psym->data.g;
  lval->typeptr = psym->typeptr;
  lval->e_tptr = Lmalloc(strlen(psym->tptr.g) + 1);
  strcpy(lval->e_tptr, psym->tptr.g);
  lval->e_flags = E_MEOFFS;
  if (psym->tptr.g[0] == T_ARRAY)
    {
      return (0);
    }
  return (1);
}

/*
	getlabel()
	Get next unused label.
*/
int getlabel()
{
  return (++nxtlab);
}
