/*

   error.h
   
   (c) 1998 42Bastian Schick

   last modified:
   
   98/08/17   42BS  created
   
*/

#ifndef ERROR_H

#define ERROR_H
 
void Error(char * msg);
void Syntax();
void Illegal(char *msg);
void Missing(char * msg);
void MultDef(char * msg);
void Need(char * msg);
void needlval();
void ersum();
void PError(char *pfx,char * msg);
void fatal(char * msg);
void Warning(char * msg);

#endif
