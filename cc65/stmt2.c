
/*
   Statement parsing, part 2*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"


/*internal prototypes */
void doswitch();
void dofor();

/* external prototypes */
/*stmt1.c*/
void ns();
void addwhile(int sp,int loop,int lab,int linc,int lstat);
void delwhile();
int statement(); 
/*lexer.c*/
void gettok();
/* glb.c */
void constexp(struct expent * lval);

/* expr1.c */
void expression(int testing);
void needcol();
/*expr3.c*/
void needbrack(int btype);
int getlabel();
/*io.c*/
void outchar(char c);

struct swent
{
  int sw_const;
  int sw_lab;
};

/*
   Handle 'switch' statement here
*/
void doswitch()
{
int dlabel;		/* for default */
int lab;		/* exit label */
int label;		/* label for case */
int lcase;		/* label for compares */
struct expent lval;
struct swent * p;
struct swent * q;
struct swent swtab[100];
char * help,*save_help,*save_q;

  if ( (save_help = help = (char *)malloc(4000)) == NULL)
    fatal("No more memory !");

  gettok();
  dlabel = 0;			/* init */
  lab = getlabel();		/* get exit */
  p = swtab;
  addwhile(oursp, 0, lab, 0, 0);

  save_q = outqi;
  outqi = help;
  
  needbrack(LPAREN);
  expression(0);
  needbrack(RPAREN);
  *outqi = 0;
  outqi = save_q;
  
  /*
   * result of expr is in P
   */
  needbrack(LCURLY);
  jump(lcase = getlabel());
  while (curtok != RCURLY)
    {
      if ((curtok == CASE) || (curtok == DEFAULT))
	{
	  label = getlabel();
	  do
	    {
	      if (curtok == CASE)
	        {
	          gettok();
		  constexp(&lval);
		  p->sw_const = lval.e_const;
		  p->sw_lab = label;
		  ++p;
		}
	      else
		{
		  gettok();
		  dlabel = label;
	  	}
	      needcol();
	    } while ((curtok == CASE) || (curtok == DEFAULT));
          outcdf(label);
	}

    if (curtok != RCURLY)
      statement();
  }

 /* end of case */

  gettok();
  jump(lab);
  outcdf(lcase);
  /* and now paste condition */
  for( ; *help; ++help,++outqi )
    *outqi = *help;
    
  casejump();
  q = swtab;
  while (q != p)
    {
      wordpref();			/* want word constants */
      outlab(q->sw_lab);		/* output labl to jump to */
      outchar(',');
      outdecnl(q->sw_const);	/* output the corresponding value */
      ++q;
    }

  wordpref();
  outdecnl(0);		/* terminate list */

  
  if (dlabel)
    jump(dlabel);
  outcdf(lab);
  delwhile();
  free(save_help);
}

/*
   Handle 'for' statement here
*/
void dofor()
{
int loop;
int lab;
int linc;
int lstat;
char *help,* save_q,*save_help;
  
  if ( (save_help = help = (char *)malloc(4000)) == NULL)
    fatal("No more memory !");
  
  gettok();
  loop = getlabel();
  lab = getlabel();
  linc = getlabel();
  lstat = getlabel();
  addwhile(oursp, loop, lab, linc, lstat);
  needbrack(LPAREN);
  if (curtok != SEMI)
    {				/* exp1 */
      expression(0);
    }
  ns();
  outcdf(loop);
/* now, loop-comparison */
  if (curtok != SEMI)
    {				/* exp2 */
      expression(1);		/* set TEST-flag */
      falsejump(lab, 0);
    }
  ns();
  /*jump(lstat); not needed anymore 42BS*/
  
  /* save expr3 for later */
  
  save_q = outqi;
  outqi = help;
  
  outcdf(linc);
  if (curtok != RPAREN)
    {				/* exp3 */
      expression(0);
    }
  needbrack(RPAREN);
  jump(loop);
  *outqi = 0;
  /* ok, we got the inc-expr */
  
  /* now continue with the body */
  outqi = save_q;
  outcdf(lstat);
  statement();
  /* and now paste inc-expr at the end of the loop */
  for( ; *help; ++help,++outqi )
    *outqi = *help;
 
  /*jump(linc); not needed anymore */
    
  outcdf(lab);
  delwhile();
  free(save_help);
}
