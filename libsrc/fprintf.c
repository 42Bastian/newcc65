/* #define NOARGC  /* no argument count passing */

/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/
#include <stdio.h>
/*
   fprintf(fd, ctlstring, arg, arg, ...) - Formatted print.
   Operates as described by Kernighan & Ritchie.
   b, c, d, o, s, u, and x specifications are supported.
   Note: b (binary) is a non-standard extension.
*/
fprintf(argc)
int argc; 
{
  int dummy[1];
  int * nxtarg;
/*  nxtarg = CCARGC() + &argc; */
  nxtarg = &argc + dummy[1] - 1;
  return(_pfguts(*nxtarg, --nxtarg));
}

/*
   printf(ctlstring, arg, arg, ...) - Formatted print.
   Operates as described by Kernighan & Ritchie.
   b, c, d, o, s, u, and x specifications are supported.
   Note: b (binary) is a non-standard extension.
*/
printf(args) 
int args; 
{
  int dummy[1];
  
/*  tprintf("printf: &args=%x &argc=%x argc=%d\n", &args, &dummy[1], dummy[1]); */
  return(_pfguts(stdout, &args + dummy[1] - 1));
}

/*
   _pfguts(fd, ctlstring, arg, arg, ...)
   Called by fprintf() and printf().
*/
_pfguts(fd, nxtarg)
int fd;
int *nxtarg; 
{
  int  arg, left, pad, cc, len, maxchr, width;
  char *ctl, *sptr, str[17];
  cc = 0;                                         
  ctl = *nxtarg--;                          
  while(*ctl)
    {
    if(*ctl!='%') { fputc(*ctl++, fd); ++cc; continue; }
     else 	  ++ctl;
    if(*ctl=='%') { fputc(*ctl++, fd); ++cc; continue; }
    if(*ctl=='-') { left = 1; ++ctl; } 
     else	  left = 0;       
    if(*ctl=='0') pad = '0'; else pad = ' ';           
    if(isdigit(*ctl)) 
      {
      width = atoi(ctl++);
      while(isdigit(*ctl)) ++ctl;
      }
     else width = 0;
    if(*ctl=='.') 
      {            
      maxchr = atoi(++ctl);
      while(isdigit(*ctl)) ++ctl;
      }
     else maxchr = 0;
    arg = *nxtarg--;
    sptr = str;
    switch(tolower(*ctl++))
      {
      case 'c': str[0] = arg; str[1] = NULL; break;
      case 's': sptr = arg;        break;
      case 'd': itoa(arg,str);     break;
      case 'b': itoab(arg,str,2);  break;
      case 'o': itoab(arg,str,8);  break;
      case 'u': itoab(arg,str,10); break;
      case 'x': itoab(arg,str,16); break;
      default:  return (cc);
      }
    len = strlen(sptr);
/*  tprintf(" pf: str '%s' len %d wid %d\n", sptr, len, width); */
    if(maxchr && maxchr<len) len = maxchr;
    if(width>len) width = width - len; else width = 0; 
/*  tprintf(" pf: str '%s' len %d wid %d iocb %x\n", sptr, len, width, fd); */
    if(!left) while(width--) {
/*  tprintf(" pf: pad, iocb %x, %d remaining\n", fd, width);  */
				fputc(pad,fd); ++cc;}
    while(len--) {fputc(*sptr++,fd); ++cc; }
    if(left) while(width--) {
/* tprintf(" pf: fin, %d remaining\n", width); 		*/
				fputc(pad,fd); ++cc;}  
    }
  return(cc);
  }

