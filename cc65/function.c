
/*	C functions definitions	*/

#include <stdio.h>
#include "cc65.h"
#include "cclex.h"
#include "error.h"
#include "code-65.h"

/*internal prototypes*/
void funargs();
void declargs();
void newfunc(struct hashent * psym);
void declloc();

/* external prototypes*/
 
/*symtab.c*/
void addloc(struct hashent * psym,char * tarray,int sc,int offs);
int  SizeOf(char * tarray);
 
/*expr3.c*/
int getlabel();

/*lexer.c*/
void gettok();

/*stmt1.c*/
int  compound();
/* glb.c */
void parseinit(char * tptr);
/* io.c*/
void flushout();
/*main.c*/
void dumplits();
int getsclass(int lv,int dflt);
int gettype(int dflt,struct hashent ** sptr);

extern char * Lmalloc();
extern struct hashent * decl();
extern struct hashent * declare();
 

extern int outcnt;
extern int stats;


/*
  Parse function dummy arguments.
*/
void funargs()
{
  n_funargs = 0;		/* -- jrd */

  if (curtok == RPAREN)
    {
      gettok();
      return;
    }
  if (lovptr != 0)
    {
      fatal("err 4");
    }

  while (1)
    {
      if (curtok == IDENT)
	{
	  addloc((struct hashent *) curval, NULL, SC_STACK, 0);
	  ++n_funargs;		/* -- jrd.  count the arg */
	  gettok();
	}
      else
	{
	  Syntax();
	}
      if (curtok == COMMA)
	{
	  gettok();
	}
      else if (curtok == RPAREN)
	{
	  gettok();
	  break;
	}
      else
	{
	  Syntax();
	}
    }
}

/*
  declargs( )
  Process argument declarations.
*/
void declargs()
{
  int i;
/*  char * p;*/
  struct hashent * psym;
  struct hashent * sadr;
  int stoff;
  char tarray[MAXTYPELEN];
  char * tptr;
  int type;

  while ((curtok != LCURLY)	/* && (curtok != ASM) */ )
    {
      getsclass(1, SC_STACK);
  
      type = gettype(INT, &sadr);
      while (1)
	{
	  absdecl = 0;
	  if ((psym = declare(tarray, type, sadr)) != NULL)
	    {
	      if (psym->flag.l == 0)
		{
		  Error("declaration of parameter not in argument list");
		  addloc(psym, tarray, SC_STACK, 0);
		}
	      else
		{
		  tptr = Lmalloc(strlen(tarray) + 1);
		  strcpy(tptr, tarray);
		  if (tptr[0] == T_ARRAY)
		    {
		      (tptr += 3)[0] = T_PTR;
		    }
		  psym->tptr.l = tptr;
		}
	    }

	  if (curtok != COMMA)
	    break;
	  gettok();
	}
      if (curtok == SEMI)
	{
	  gettok();
	}
    }

  /*
    Assign offsets and default undeclared arguments to int.
  */
  tarray[0] = T_INT;
  tarray[1] = '\0';
  stoff = 2;
  for (i = lovptr - 1; i >= 0; --i)
    {
      psym = lvtab[i];
      tptr = psym->tptr.l;
      if (tptr == NULL)
	{
	  /*
	   * default to "int"
	   */
	  tptr = Lmalloc(2);
	  strcpy(tptr, tarray);
	  psym->tptr.l = tptr;
	}
      psym->data.l = stoff;
      stoff += (SizeOf(tarray) + 1) & (~1);
    }
}

/*
  newfunc( psym )
  Parse argument declarations and function body.
*/
void newfunc(struct hashent * psym)
{
 
  int sbyte;
   
  psym->flag.g |= SC_DEFINED;
  /* this busted??     outcdf(psym->data.g); -- jrd */

  interrupt = 0;
  if (curtok == IRQ)
  {
    gettok();
    interrupt = 1;
    startfun(psym->name,0);
    /* no arguments for IRQ-routines !! */
  }
  else
  {
    startfun(psym->name,1);
    declargs();
  }

  if (curtok != LCURLY)
	{
	  Syntax();
	}
  oursp = 0;
 
  sbyte = outcnt;
 
  register_base = -1;

  if (!compound())
	{
	  ret(register_base - REGISTER_BASE);
	  dumplits();
	  n_funargs = -1;	/* reset arg counter */
	}

  if (stats)
	{
	  printf("%s: %d bytes\n", psym->name, outcnt - sbyte);
	}   
  flushout(); /* clear output-buffer */
}

/*
  declloc()
  Declare local variables.
*/
void declloc()
{
  int offs;
  int lab;
  struct hashent * psym;
  struct hashent * sadr;
  int sc;
  char tarray[MAXTYPELEN];
  int type;
 
  offs = oursp;

  while (1)
    {
      sc = getsclass(1, SC_STACK);
      
      if ((type = gettype(0, &sadr)) == 0)
      	break;
//printf("declloc : %s %d\n",sadr->name,sadr->misc);
      while (1)
	    {
	      absdecl = 0;
	      psym = declare(tarray, type, sadr);

	  {
	    int siz;

	    if (tarray[0] != T_FUNC)
	      siz = SizeOf(tarray);
	    else
	      siz = 2;
	    addloc(psym, tarray, sc, offs - siz);
	  }
	  if (tarray[0] != T_FUNC)
	    {
	      if ( sc & SC_STACK )
        { 
		    offs -= SizeOf(tarray);
		    }
	      else if (sc == SC_STATIC)
		    { 
		      if ( curtok == ASGN)
		      {
		        segdata();
		        gettok();
			      outcdf( psym->data.l = getlabel() );	
			      parseinit(tarray);
			      segtext();
			    }
			    else
			    {
	          segbss();
		        outcdf( psym->data.l = getlabel() );	 
 		        outsp(SizeOf(tarray));
		        segtext();
			    }
		    }
		    else if ( sc == SC_ZSTATIC)
		    {
		      if ( curtok == ASGN )
		      {
		        Error("Initializing of zstatic not allowed !");
			    }
			    
		      segbsszp();
		      outcdf( psym->data.l = getlabel() );
		      outsp(SizeOf(tarray));
		      segtext();
		    }
        else if ( sc == SC_REGISTER) /* 42BS */
        {
          if ( interrupt )
          {
            Error("No register variables allowed in ISRs!");
          }
          if ( ( register_base + SizeOf(tarray) )>0x7f )
          {

			       segbss();
             outcdf( psym->data.l = getlabel() );
             outsp(SizeOf(tarray));
		         segtext();
          }
	         else
	         {
            lab = getlabel();
            psym->data.l = lab;
		         outlabledef(lab,register_base);
            register_base += SizeOf(tarray);
          }
        }
	    }

	  if (curtok != COMMA)
	    break;
	  gettok();
	}
      if (curtok == SEMI)
	{
	  gettok();
	}
    }

   oursp = modstk(offs); 
}
