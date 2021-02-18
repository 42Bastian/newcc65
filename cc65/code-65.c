/*
   Code generator routines for CC65.  These are called by various
   parts of the compiler to generate primitive operations.
*/

/* changed jmp into lbra 42BS */
/* removed force-test     42BS */

#include <stdio.h>

#define UNSIGNED_FIX

#include "cc65.h"
#include "code-65.h"


/* io.c */
void flushout();
void outchar(char c);

/* preproc.c */
char * findmac();

/* end of prototypes */
/* output a string */

void ot(register char * str)
{
  while ( *str )
    {
      outchar(*str);
      ++str;
    }
}

/* print newline */ 
inline void nl()
{
  outchar('\n');
}

/* output a string, add a newline */
void ol(char * str)
{
  ot(str);
  nl();
}

void oljsr(char * str)
{
  ot("\tjsr\t");
  ol(str);
}

void oljsrpop(char * str)
{
  oljsr(str);
  popsp();
}

/* helper for konst1 */
void olstrdnl(char * str,int  num)
{
  ot(str);
  outdecnl(num);
}

void outabsdecl(char *n,int value)
{
  outchar('_');
  ot(n);
  ot("\t = ");
  outdecnl(value);
}

static char lda[] = "\tlda\t#";
static char ldx[] = "\tldx\t#";
static char xxhi, xxlo;

/* load konst into AX in such a way as to set cond codes correctly */
void olxxxhi()
{
  olstrdnl(ldx, xxhi);
}

void olxxxlo()
{
  olstrdnl(lda, xxlo);
}

void konst1(int k)
{
  xxhi = k >> 8;
  xxlo = k & 0xFF;
  
  if (xxhi == xxlo)
    {
      olxxxlo();
      ol("\ttax");
    }
  else if (xxhi)
    {
      olxxxlo();
      olxxxhi();
    }
  else
    {
      olxxxhi();
      olxxxlo(); /* this loses some; generates spurious negative... */
    }
}

void konst3(int k)
{
  olstrdnl("\tldy\t#", k);
}

char efn[] = {'e', 'n', 't', 'e', 'r', 'f', 'u', 'n'};
char efn_n = ' ';
char efn_0 = 0;

void startfun(char * name,int flag)
{
  if (n_funargs == -1)
    printf("Internal error! no argcount?\n");
  outgblc(name); /* print a global name with colon */
  if (findmac("FIXARGC"))
    if ( n_funargs <= 8 )
    {
      efn_n = n_funargs + '0';
    }
    else
    {
      konst3(n_funargs);
      efn_n = 0;
    }
  else
    efn_n = 0;
  if (flag)
    oljsr(efn);
}

/* util function */
inline int char_t_p(char * tptr)
{
  return ((tptr[0] & ~T_UNSIGNED) == T_CHAR);
}

/* Fetch a static memory cell into the primary register */
void getmem(char * lbl,char * tptr,char test_p)	/* jrd added test_p */
{
  if (char_t_p(tptr))
    if (tptr[0] & T_UNSIGNED)
    {
      ot("\tldau\t"); outgblnl(lbl);
    }
    else
    {
      ot("\tldas\t"); outgblnl(lbl);
    }
  else				/* loading word-sized thing */
    if (test_p)
    {
      ot("\tlda\t"); outgblnl(lbl);
      ot("\tora\t"); outgbl(lbl); ol("+1");
    }
    else
    {
      ot("\tldax\t"); outgblnl(lbl);
    }
}
/* Fetch a local static memory cell into the primary register */

void getlmem(int lbl,char * tptr,char test_p)	/* jrd added test_p */
{
  if (char_t_p(tptr))
    if (tptr[0] & T_UNSIGNED)
    {
      ot("\tldau\t"); outlabnl(lbl);
    }
    else
    {
      ot("\tldas\t"); outlabnl(lbl);
    }
  else				/* loading word-sized thing */
    if (test_p)
    {
      ot("\tlda\t"); outlabnl(lbl);
      ot("\tora\t"); outlab(lbl); ol("+1");
    }
    else
    {
      ot("\tldax\t"); outlabnl(lbl);
    }
}

/* and another... */

void konst3s(int offs)
{
  konst3(offs - oursp);
}

#ifdef UNSIGNED_FIX
/* 
   this is another helper fun for generating the right variant of
   ldaxxx frob.  given a typespec and an extension, generate
   "lda" followed by <ext> for signed char, 'u'<ext> for unsigned 
   char, or 'x'<ext> for fixnum 
*/
void lda_helper(unsigned char * typestring,char * ext)
{
  ot("\tjsr\tlda");
  if (char_t_p(typestring))	/* some kind of char-sized thing? */
    {
    if (typestring[0] & T_UNSIGNED)
	    outchar('u');		/* call unsigned variant */
    }
  else
    outchar('x');		/* call word-sized variant */
  ol(ext);
}
#endif

/*
  getladr( offs )
  Fetch the address of the specified symbol
  into the primary register
*/
void getladr(int offs)
{
  konst3s(offs);		/* load Y with offset value */
  oljsr("locysp");		/* and compute location of SP@(Y) */
}

/*
  getloc( offs, tptr )
  Fetch specified local object (local var).
*/
void getloc(int offs,char * tptr)
 
{
//  konst3s(offs);		/* load Y with offset value */
 
#ifdef UNSIGNED_FIX
//  lda_helper(tptr, "ysp");
  
  if (char_t_p(tptr) )
  {
    if ( tptr[0] & T_UNSIGNED )
    {
      ol("\tldx\t#0");
      if (offs - oursp)
      {
        konst3s(offs);
        ol("\tlda\t(sp),y");
      }else
        ol("\tlda\t(sp)");
    }else
    {
      konst3s(offs);
      oljsr("ldaysp");
    }
  }else
  {
    konst3s(offs);
    oljsr("ldaxysp");
  }
#else
  oljsr((char_t_p(tptr) ? "ldaysp" : "ldaxysp"));
#endif
 
}

/*
  putloc( offs, tptr )
  Put data into local object.
*/
void putloc(int offs,char * tptr)
{
  /*konst3s(offs);*/		/* load Y with offset value */
  /*oljsr((char_t_p(tptr) ? "staysp" : "staxysp"));*/
  if (char_t_p(tptr))
    if (offs - oursp)
      {
      konst3s(offs);
      ol("\tsta\t(sp),y");
      }
     else
       ol("\tsta\t(sp)");
  else
    {
    konst3s(offs);
    oljsr("staxysp");
    }
 
}

/*
  Store the primary register into the specified
  static memory cell 
*/
void putmem(char * lab,char * tptr)
{
  ot("\tsta\t");
  outgblnl(lab);
  if (!char_t_p(tptr))
    {
      ot("\tstx\t");
      outgbl(lab);
      ol("+1");
    }
}
/*
  Store the primary register into the specified
  local static memory cell 
*/
void putlmem(int lab,char * tptr)
{
  ot("\tsta\t");
  outlabnl(lab);
 if (!char_t_p(tptr))
    {
      ot("\tstx\t");
      outlab(lab);
      ol("+1");
    }
} 
/*
  putstk( tptr )
  Store the specified object type in the primary register
  at the address on the top of the stack
*/
void putstk(struct expent * lval)
 
{
  if ((lval->e_flags == E_MEOFFS) && /* some kind of structure */
      (lval->e_const != 0))	/* and not first slot... */
    {
      konst3(lval->e_const);	/* load Y with structure offset */
#ifdef old_way
      if (lval->e_tptr[0] == T_CHAR)
	oljsr("staspidx");	/* store A at ((SP))+Y */
      else
	oljsr("staxspidx");	/* store AX at ((SP))+Y */
#else
      oljsr((char_t_p(lval->e_tptr) ? "staspidx" : "staxspidx"));
#endif
    }
  else
    {
#ifdef old_way
      if (lval->e_tptr[0] == T_CHAR)
	oljsr("staspp"); /* store A at ((SP)) */
      else
	oljsr("staxspp");	/* store AX at ((SP)) */
#else
      oljsr((char_t_p(lval->e_tptr) ? "staspp" : "staxspp"));
#endif
    }
  popsp();
}

/*
  indirect( lval )
  Fetch the specified object type indirect through the
  primary register into the primary register
*/
void indirect(struct expent * lval)
 
{
  if ((lval->e_flags == E_MEOFFS) && /* some kind of structure */
      (lval->e_const != 0))	/* and not slot 0 */
    {
      konst3(lval->e_const);	/* load Y with offset */
 
#ifdef UNSIGNED_FIX
      lda_helper(lval->e_tptr, "idx");
#else
      oljsr((char_t_p(lval->e_tptr) ? "ldaidx" : "ldaxidx"));
#endif
 
    }
  else
    {
 
#ifdef UNSIGNED_FIX
      lda_helper(lval->e_tptr, "i");
#else
      oljsr((char_t_p(lval->e_tptr) ? "ldai" : "ldaxi"));
#endif
    }
}
  
/*
  save()
  Copy AX to hold register.
*/
void save()
{
/*	ol("\tstx\t<(svax+1)\n"
	   "\tsta\t<svax");
*/
	ol("\tpha\n\tphx");
}
/*
  rstr()
  Copy hold register to P.
*/

void rstr()
{
/*	
  ol("\tldx\t<(svax+1)\n"
     "\tlda\t<(svax)");
*/
	ol("\tplx\n\tpla");
}

/*
  immed()
  Print partial instruction to get an immediate value
  into the primary register.
*/
char LDAX[]="\tldax\t#";

void immed(int i)
{
  olstrdnl(LDAX,i);
}
void immedgbl(int i)
{
  ot(LDAX); outgblnl((char *)i);
}
void immedlab(int i)
{
  ot(LDAX); outlabnl(i);
}
void immedslt(int i)
{
  ot(LDAX); outslt(i); 
}
/* added by jrd.  force a test to set cond codes right */
void tst()
{
  oljsr("tstax"); /* 42BS again in 97/09/20 */
}

/*
  push()
  Push the primary register onto the stack
*/
void push()
{
  oljsr("pushax");
  pushsp();
}

/* don't know the difference ???? 42BS */
void push1()
{
  oljsr("pushax");
  pushsp(); 
}


/* Swap the primary register and the top of the stack */
void swapstk()
{
  oljsr("swapstk");
}

/* Call the specified subroutine name */
void call(char * lbl,int narg)

{
 if (!findmac("NOARGC")) /* don't generate argc if suppressed */
      konst3(narg >> 1);	/* want n args, not nbytes args */
  ot("\tjsr\t");
  outgblnl(lbl);

  oursp = oursp + narg;		/* callee pops args */
}

/* Return from subroutine */
void ret(int ret)
{
  if (interrupt)
    ol("\trts");
  else
    {
      konst3(ret);
      ol("\tjmp\texitfun");
  /* do not change into lbra !!!! 42BS */
    }
}

/*
  Perform subroutine call to value on top of stack.
  This really calls value in AX 
*/
void callstk(int narg)

{
  if (!findmac("NOARGC"))	/* don't generate argc if suppressed */
    konst3(narg >> 1);		/* load Y with arg count */
  oljsr("callax");		/* do the call */
  oursp = oursp + narg;		/* callee pops args */
/*  popsp();	*/		/* pop subr addr */
}

/* Jump to specified internal label number */
void jump(int label)
{ 
  ot("\tlbra\t");
  outlabnl(label);
}

/* output case-jump preample */
void casejump()
{
  oljsr("casejump");
}

/*
  truejump -- jump to lable if p nz
*/
void truejump(int label,int invert)
{
  if (invert)
    falsejump(label, 0);
  else
    {
      ot("\tlbne\t"); outlab(label); ol("\t;;; truejump");
    }
}

/*
  falsejump -- jump to lable if AX is zero
*/
void falsejump(int label,int invert)
{

  if (invert)
    truejump(label, 0);
  else
    {
      ot("\tlbeq\t"); outlab(label); ol("\t;;; falsejump");
    }
}

/*
  cmpjump -- compare p to constant
  and jump not-equal to label
*/
/* cmpjump(constant, label)
int	constant;
int	label;
{
  ol(";;; cmpjump");
} */

/*
  Modify the stack pointer to the
  new value indicated
*/
void otx(char * str)
{
  ot("\tjsr\t");
  ot(str);
  ot("sp");
}

void mod_internal(int k,char * verb1,char * verb2)
{
  if (k <= 8)
    {
      otx(verb1);
      outchar(k + '0');
    }
  else
    {
      konst3(k);
      otx(verb2);
    }
  nl();
}

int modstk(int newsp)
{
int k;

  if ((k = (newsp - oursp)) > 0)
    {
      mod_internal(k, "inc", "addy");
    }
  else if (k < 0)
    {
      mod_internal(-k, "dec", "suby");
    }
  return (newsp);
}

/* Double the primary register */
 
void doublereg()
{
  oljsr("aslax");
}

void incprim(int n)
{
  if ( n < 256 )
  {
    konst3(n);
    oljsr("addaxy");
  }
  else
  {
    push();
    immed(n);
    add();
  }
}
void decprim(int n)
{
  if ( n < 256 )
  {
    konst3(n);
    oljsr("subaxy");
  }
  else
  {
    push();
    immed(n);
    sub();
  }
}

/*
  Add the primary and secondary registers 
  (results in primary) 
*/
 
void add()
{
  oljsrpop("addtos");
}


/* 
  Subtract the primary register from the secondary 
  (results in primary)
*/
void sub()
{
  oljsrpop("subtos");
}


/*
  Multiply the primary and secondary registers 
  (results in primary 
*/
void mult()
{
  
  if (interrupt)
  {
    static no_glb_flag = 1;
    
    if ( no_glb_flag )
      ol("\txref _multos"); // hack, but works :)
    no_glb_flag = 0;
    oljsrpop("_multos");
  }
  else
    oljsrpop("multos");
}
/*
  Divide the secondary register by the primary 
  (quotient in primary, remainder in secondary) 
*/

void _div()
{
  if (interrupt)
  {
    static no_glb_flag = 1;
    
    if ( no_glb_flag )
      ol("\txref _divtos"); // hack, but works :)
    no_glb_flag = 0;
    oljsrpop("_divtos");
  }
  else
    oljsrpop("divtos");
}

/*
  Compute remainder (mod) of secondary register divided
  by the primary (remainder in primary, quotient in secondary)
*/
 
void mod()
{
  oljsrpop("modtos");
}

/*
   Inclusive 'or' the primary and the secondary registers
   (results in primary) 
*/
void or()
{
   oljsrpop("or_tos");
}


/*
  Exclusive 'or' the primary and seconday registers 
  (results in primary) 
*/ 
void xor()
{
  oljsrpop("xortos");
}

/* 
  'And' the primary and secondary registers 
  (results in primary) 
*/
 
void and()
{
 
  oljsrpop("andtos");
}

 /*
  Arithmetic shift right the secondary register number of 
  times in primary (results in primary) 
*/
 
void asr()
{
 
  oljsrpop("asrtos");
}

/*
  Arithmetic left shift the secondary register number of 
  times in primary (results in primary) 
*/
 
void asl()
{
  oljsrpop("asltos");
}

void aslprim(int n)
{
  konst3(n);
  oljsr("aslaxy");
}

/* Form two's complement of primary register */
void neg()
{
 oljsr("negax");
}


/* Form logical complement of primary register */
 
void lneg()
{
  oljsr("lnegax");
}

/* Form one's complement of primary register */
void com()
{
 oljsr("complax");
}


/* Increment the primary register by one */
void inc()
{
  konst3(1);
  oljsr("addaxy");
}


/* Decrement the primary register by one */
void dec()
{
  oljsr("decax1");
}

 
/* 
  Following are the conditional operators 
  They compare the secondary register against the primary 
  and put a literal 1 in the primary if the condition is 
  true, otherwise they clear the primary register 
*/

/* Test for equal */
 
void eq()
{
  oljsrpop("toseqax");
}

 
/* zero-p */
 
void eq0()
{
  oljsr("axzerop");
}
 

/* Test for not equal */
 
void ne()
{
  oljsrpop("tosneax");
}



/* Test for less than */
void lt(int nosign)
 
{
  oljsrpop(nosign ? "tosultax" : "tosltax");
}

/* Test for less than or equal to */
void le(int nosign )
{
  oljsrpop(nosign  ? "tosuleax" : "tosleax");
}

/* Test for greater than */
void gt(int nosign)
{
  oljsrpop(nosign  ? "tosugtax" : "tosgtax");
}

/* Test for greater than or equal to */
void ge(int nosign)
{
  oljsrpop( nosign ? "tosugeax" : "tosgeax");
}

#ifdef old_cruft
...no longer need unsigned stuff...
/* Test for less than (unsigned) */
ult()
{
  oljsr("tosultax");
  popsp();
}

/* Test for less than or equal to (unsigned) */
ule()
{
  oljsr("tosuleax");
  popsp();
}

/* Test for greater than (unsigned) */
ugt()
{
  oljsr("tosugtax");
  popsp();
}

/* Test for greater than or equal to (unsigned) */
uge()
{
  oljsr("tosugeax");
  popsp();
}

#endif

#ifdef busted
static char dollr = '$';
static char xdig[4];
static char xd0 = '\0';
#endif

void outdec(int numbr)
{
char dummy[10];

sprintf(dummy,"%d",numbr);
 
ot(dummy);
}
#ifdef urgs
{
char digit[8];
int i;
 

#ifdef busted
/* use bummed assembler routine */
/* OOps!  Can't use itoa here, cause it uses the FP routines, and
  they trash page 5, which we use for the symbol table.  print it in hex.
  itoa(numbr, digit);
*/
  digit[0] = '$';
  for (i = 3; i >= 0; --i)
    {
      xdig[i] = (numbr & 0x0F) + '0';
      numbr >> = 4;
    }
/*  ot(digit); */
  ot(&dollr);
#else

  if (numbr < 0)
    {
      numbr = -numbr;
      outchar('-');
    }
  digit[0] = 0;
  for (i = 0; numbr > 0;)
    {
      digit[i] = numbr % 10;
      numbr = numbr / 10;
      ++i;
    }
  if (i == 0)
    i = 1;
  while (i > 0)
    outchar(digit[--i] + '0');
#endif
}
}
#endif

void outdecnl(int n)
{
  outdec(n);
  nl();
}

void outcch(int numbr)
 
{
  outdec(numbr);
}

void outlabledef(int lbl,int value)
{
  outchar('L');
  outdec(lbl);
  ot(":\t=");
  outdecnl(value); 
}

void outlab(int lbl)
{
  outchar('L');
  outdec(lbl);
}

void outlabnl(int lbl)
{
  outlab(lbl);
  nl();
}

void outcdf(int labl)
{
 
  outlab(labl);
  ol(":");
 
}

void outdat(int n)
{
  ol(";;; outdat");
}

void outslt(int n)
{
/* this seems to get used for constant string values? */
  outlab(litlab); /* output current lit pool labl */
  outchar('+');
  outdec(n);
  nl();
}

/* reserve static storage, n bytes */
void outsp(int n)
{
//  ol("\tbss");
  ot("\tds\t");
  outdec(n);
  nl();
 // ol("\ttext");
}

/* output a row of bytes as a constant */
void outbv(char * bytes,int nbytes)
{
int bpl;
  
  while (nbytes)
    {
      nbytes -= (bpl = (nbytes > 16) ? 16 : nbytes);
      bytepref();
 kludge1:
      outdec(*bytes++);
      if (--bpl)
	{
	  outchar(',');
	  goto kludge1;
	}
	outchar('\n');
  
    }
}
#define SEG_TEXT   1
#define SEG_DATA   2
#define SEG_BSS    3
#define SEG_BSSZP  4
int CurrentSegment = SEG_TEXT;
void segdata()
{ 
  if ( CurrentSegment == SEG_DATA ) return;
  ol("\tdata");
  CurrentSegment = SEG_DATA;
}
void segtext()
{ 
  if ( CurrentSegment == SEG_TEXT ) return;
  ol("\ttext");
  CurrentSegment = SEG_TEXT;
}
void segbss()
{ 
  if ( CurrentSegment == SEG_BSS ) return;
  ol("\tbss");
  CurrentSegment = SEG_BSS;
}
void segbsszp()
{ 
  if ( CurrentSegment == SEG_BSSZP ) return;
  ol("\tbsszp");
  CurrentSegment = SEG_BSSZP;
}

/* prefix for word-sized constant */
void wordpref()
{ flushout();
  ot("\tdc.w\t");
}

/* prefix for byte-sized constant */
void bytepref()
{ flushout();
  ot("\tdc.b\t");
}

/* print a global name into the asm file.  This probly supercedes the
   one below... */
void outgbl(char * name)
{
  outchar('_');
  ot(name);
}

void outgblnl(char * name)
{
  outgbl(name);
  nl();
}

void outgblc(char * name)
 
{
  outgbl(name);
  ol("::");
}

/* output a global or external name */

void outgoe(char * sname)
 
{
  ot("\tglobal\t");
  outgblnl(sname);
}

void outext(char * sname)
 
{
  ot("\txref\t");
  outgblnl(sname);
}

int popsp()
{
  return (oursp += 2);
}

int pushsp()
{
  return (oursp -= 2);
}

void SaveRegs(int base,int save)
{
  if (save)
  {
    if ( save > 2 )
    {
      olstrdnl("\tldy\t#",save);
      olstrdnl("\tldx\t#",base+save-1);
      ol("\tjsr\tSaveRegs");
    }
    else
    {
      if ( save > 1 )
      {
        olstrdnl("\tlda\t",base+1);
        ol("\tpha");
      }
      olstrdnl("\tlda\t",base);
      ol("\tpha");
    }
  }
}

void RestoreRegs(int base,int save)
{
  if ( save > 2)
  {
    olstrdnl("\tldy\t#",save);
    olstrdnl("\tldx\t#",base);
    ol("\tjsr\tRestoreRegs");
  }
  else if ( save )
  {
    ot("\tpla\n"
       "\tsta\t");
    outdecnl(base);
    if (save>1)
    {
      ot("\tpla\n"
         "\tsta\t");
      outdecnl(1+base);
    }
  }
}
