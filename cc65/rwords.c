
/* CCCMD.C - look up reserved words */

#include <stdio.h>

#include "cc65.h"
#include "cclex.h"

/*preproc.c*/
int tok_assoc(char * sym,struct tok_elt *  toks);

struct tok_elt keywords[] =
{
  {"if", IF},
  {"long", LONG},
  {"register", REGISTER},
  {"static", STATIC},
  {"else", ELSE},
  {"char", CHAR},
  {"goto", GOTO},
/* never used?     { "typedef", TYPEDEF },	*/
  {"case", CASE},
  {"enum", ENUM},
  {"sizeof", SIZEOF},
  {"return", RETURN},
  {"extern", EXTERN},
  {"double", DOUBLE},
  {"float", FLOAT},
  {"void", VOID},
  {"int", INT},
  {"short", SHORT},
  {"for", FOR},
  {"unsigned", UNSIGNED},
  {"auto", AUTO},
  {"break", BREAK},
  {"switch", SWITCH},
  {"struct", STRUCT},
  {"continue", CONTINUE},
  {"while", WHILE},
  {"do", DO},
  {"default", DEFAULT},
  {"union", UNION},
  {"interrupt", IRQ},   /* 42BS */
  {"asm",ASM2},         /* 42BS */
  {"at",AT},            /* 42BS */
  {"zstatic",ZSTATIC},  /* 42BS */
/* what the fuck's this?    { "entry", ENTRY }, */
  {0, 0}
};

/*
char a_strcmp[] = {
    0x68, 0x68, 0x85, 0xf7, 0x68, 0x85, 0xf6, 0x68,
    0x85, 0xf9, 0x68, 0x85, 0xf8, 0xa0, 0xff, 0xc8, 0xb1,
    0xf6, 0xd1, 0xf8, 0xd0, 0x06, 0xaa, 0xd0, 0xf6,
    0xa9, 0x00, 0x60, 0xa9, 0x01, 0xa2, 0x00, 0x60
    };
*/
/*
  strcmp( s1, s2 )
  Compare two strings.
*/

/*	bogus...
strcmp(s1, s2)
char *s1, *s2;
{
    while (1) {
	if (*s1 > *s2) return( 1 );
	if (*s1 < *s2) return( -1 );
	if (*s1 == 0) return( 0 );
	++s1;
	++s2;
	}
}
 
*/

/*
  Search( string )
  Search for string in command table.
*/
int Search(char * string)
{
  return (tok_assoc(string, keywords));
}
