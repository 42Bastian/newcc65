/*

  code-65.h
   
   (c) 1998 42Bastian Schick

   last modified:
   
   98/08/17   42BS  created
   
*/

#ifndef CODE_65_H

#define CODE_65_H

void ot(char * str);
void nl();
void ol(char * str);
void oljsr(char * str);
void oljsrpop(char * str);
void olstrdnl(char * str,int  num);
void olxxxhi();
void olxxxlo();
void konst1(int k);
void konst3(int k);
void startfun(char * name,int flag);
int  char_t_p(char * tptr);
void getmem(char * lbl,char * tptr,char test_p);
void getlmem(int lbl,char * tptr,char test_p);
void konst3s(int offs);
#ifdef UNSIGNED_FIX
  void lda_helper(unsigned char * typestring,char * ext);
#endif
void getladr(int offs);
void getloc(int offs,char * tptr);
void putloc(int offs,char * tptr);
void putmem(char * lab,char * tptr);
void putlmem(int lab,char * tptr);
void putstk(struct expent * lval);
void indirect(struct expent * lval);
void save();
void rstr();
void immed();
void immedgbl(int i);
void immedlab(int i);
void immedslt(int i);

void tst();
void push();
void push1();
void swapstk();
void call(char * lbl,int narg);
void ret(int ret);
void callstk(int narg);
void jump(int label);
void casejump();
void truejump(int label,int invert);
void falsejump(int labelint ,int invert);
void otx(char * str);
void mod_internal(int k,char * verb1,char * verb2);
int  modstk(int newsp);
void doublereg();
void add();
void sub();
void mult();
void _div();
void mod();
void or();
void xor();
void and();
void asr();
void asl();
void aslprim(int n);
void incprim(int n);
void decprim(int n);
void neg();
void lneg();
void com();
void inc();
void dec();
void eq();
void eq0();
void ne();
void lt(int nosign);
void le(int nosign);
void gt(int nosign);
void ge(int nosign);
void outdec(int numbr);
void outdecnl(int n);
void outcch(int numbr);
void outlabledef(int lbl,int value);
void outlab(int lbl);
void outlabnl(int lbl);
void outcdf(int labl);
void outdat(int n);
void outslt(int n);
void outsp(int n);
void outbv(char * bytes,int nbytes);
void wordpref();
void bytepref();
void outabsdecl(char *n,int value);
void outgbl(char * name);
void outgblnl(char * name);
void outgblc(char * name);
void outgoe(char * sname);
void outext(char *);
int  popsp();
int  pushsp();
void SaveRegs(int base,int cnt);
void RestoreRegs(int base,int cnt);
void segdata();
void segtext();
void segbss();
void segbsszp();


#endif
