
/*
   Statement parsing, part 1
*/

#include <stdio.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

/* external prototypes */
/*stmt1.c*/
void doswitch();
void dofor();

/* expr1.c */
void expression(int testing);
void needtok(int token,char * msg);

/*expr3.c*/
int getlabel();
void test(int label);

/*symtab.c*/
void addloc(struct hashent * psym,char * tarray,int sc,int offs);

/* function.c */
void declloc();

/* lexer.c */
void gettok();
/* preproc.c */
void doasm2();

/* internal prototypes */
int statement(); 
void ns();
int  compound();
void doif();
void dowhile(char wtype);
void doreturn();
void dobreak();
void docont();
void addwhile(int sp,int loop,int lab,int linc,int lstat);
void delwhile();
int * readwhile();
void dogoto();
void dolabel();

/*
   Statement parser
   called whenever syntax requires
   a statement.
   this routine performs that
   statement
   and returns 1 if it is a branch, 0 otherwise
*/
int statement()
{

  if (curtok == IDENT)
    {
      if (nxttok == COLON)
	{
	  dolabel();
	}
      else
	{
	  expression(0);
	  ns();
	  return (0);
	}
    }
  switch (curtok)
    {
/*    case ASM:	 -- jrd
    	doasm();
	break;		*/
    case LCURLY:
      compound();
      break;
    case IF:
      doif();
      break;
    case WHILE:
      dowhile('w');
      break;
    case DO:
      dowhile('d');
      break;
    case SWITCH:
      doswitch();
      break;
    case RETURN:
      doreturn();
      ns();
      return (1);
    case BREAK:
      dobreak();
      ns();
      return (1);
    case CONTINUE:
      docont();
      ns();
      return (1);
    case FOR:
      dofor();
      break;
    case GOTO:
      dogoto();
      return (1);
    case ASM2:
      doasm2();
      break;
    case SEMI:
      /*
       * ignore it.
       */
      gettok();
      break;
    default:
      expression(0);
      ns();
    }
  return (0);
}

/*
   Semicolon enforcer called whenever syntax requires a semicolon
*/
void ns()
{
  needtok(SEMI, "semicolon");
}

/*
	compound()
	Compound statement.  Allow any number of statements, inside
	braces.
*/

int compound()
{
int isbrk;
char * locptr;
int oldsp;
int old_regbase;

  /*
   * gobble LCURLY
   */
  gettok();

  /*
   * parse local variable declarations if any
   */
  oldsp = oursp;
  locptr = lsptr;
  
  if ( (old_regbase = register_base) < 0 )
    register_base = old_regbase = REGISTER_BASE;
   
  declloc();

  SaveRegs( old_regbase,register_base - old_regbase );

  isbrk = 0;
  ++ncmp;
  while (curtok != RCURLY)
  {
    if (curtok == CEOF)
      break;
    else
	  {
	    isbrk = statement();
	  }
  }
  if (curtok == RCURLY)
    {
      gettok();
      --ncmp; /* close current level */
    }

  lsptr = locptr;
  
// restore register only if not on first level
// there it is done by exitfun

  if ( ncmp > 0 ) 
  {
    RestoreRegs(old_regbase,register_base - old_regbase);
    register_base = old_regbase;
  }

 if (isbrk)
    {
      oursp = oldsp;
    }
  else
    {
      oursp = modstk(oldsp);
    }

  return (isbrk);
}

/*
   Handle 'if' statement here
*/
void doif()
{
int flab1;
int flab2;

  gettok();
  flab1 = getlabel();
  test(flab1);
  statement();
  if (curtok != ELSE)
    {
      outcdf(flab1);
      return;
    }
  gettok();
  jump(flab2 = getlabel());
  outcdf(flab1);
  statement();
  outcdf(flab2);
}

/*
   Handle 'while' statement here
*/
void dowhile(char wtype)
{
int loop;
int lab;

  gettok();
  loop = getlabel();
  lab = getlabel();
  addwhile(oursp, loop, lab, 0, 0);
  outcdf(loop);
  if (wtype == 'w')
    {
      test(lab);
      statement();
    }
  else
    {
      statement();
      if (curtok == WHILE)
	{
	  gettok();
	}
      else
	{
	  Error("do with no while");
	}
      test(lab);
      ns();
      
    }
  jump(loop);
  outcdf(lab);
  delwhile();
}

/*
   Handle 'return' statement here
*/
void doreturn()
{
  gettok();
  if (curtok != SEMI)
/* func's always set CC, but this is done by exitfun-Routine */
    expression(0);

  modstk(0);
  
  ret(register_base - REGISTER_BASE);
}

/*
   Handle 'break' statement here
*/
void dobreak()
{
int * ptr;

  gettok();
  if ((ptr = readwhile()) == 0)
    return;
  modstk((ptr[wqsp]));
  jump(ptr[wqlab]);
}

/*
   Handle 'continue' statement here
*/
void docont()
{
int * ptr;

  gettok();
  if ((ptr = readwhile()) == 0)
    return;
  while (ptr >= wq)
    {
      if (ptr[wqloop])
	break;
      ptr -= wqsiz;
    }
  if (ptr < wq)
    {
      Error("no active whiles");
      return;
    }

  modstk((ptr[wqsp]));
  if (ptr[wqinc])
    {
      jump(ptr[wqinc]);
    }
  else
    {
      jump(ptr[wqloop]);
    }
}

void addwhile(int sp,int loop,int lab,int linc,int lstat)
{
  if (wqptr == wqmax)
    {
      Error("too many active whiles");
      return;
    }

  wqptr[wqsp] = sp;
  wqptr[wqloop] = loop;
  wqptr[wqlab] = lab;
  wqptr[wqinc] = linc;
  wqptr[wqstat] = lstat;
  wqptr += wqsiz;
}

void delwhile()
{
  wqptr -= wqsiz;
}

int * readwhile()
{
  if (wqptr == wq)
    {
      Error("no active whiles");
      return (0);
    }
  else
    return (wqptr - wqsiz);
}

/*
	dogoto()
	Process a goto statement.
*/
void dogoto()
{
struct hashent * psym;

  gettok();
  if (curtok != IDENT)
    {
      Need("goto label");
      return;
    }

  if ((psym = (struct hashent *) curval)->flag.l == 0)
    {
      addloc(psym, type_int, SC_INMEM, getlabel());
    }
  modstk(0);
  jump(psym->data.l);
  gettok();
}

/*
	dolabel()
	Define a label.
*/
void dolabel()
{
int lab = 0;

  if (((struct hashent *) curval)->flag.l == 0)
    {
      addloc((struct hashent *) curval, type_int, SC_INMEM, getlabel());
    }
  if (oursp)
    {
      jump(lab = getlabel());
    }
  outcdf(((struct hashent *) curval)->data.l);
  if (oursp)
    {
/*
   this modstk is here to get stack right 'cause anybody who
   jumps here will have reset the stack to zero relative to
   this frame before jumping.  Therefore, we modstk to twice
   the 'current' stack depth, thus causing a real modification
   by the current depth.
*/
      modstk(oursp * 2);	/* put the stack back */
      outcdf(lab);		/* define the label for code to jump around to */
      nl();
    }
  gettok();
  gettok();
}


