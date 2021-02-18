
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/* stuff for parser */

#ifndef _TYPES_
#include "types.h"
#endif

struct parse
	{
	char * label;		/* line label */
	char * label_suffix;	/* stray non-white chars after label */
	char * opcode;		/* op str */
	char * arg[32];		/* up to 32 arg strings */
	char * comment;		/* comment str */
	int comment_column;	/* where the comment started */
	};

extern void parse_line();		/* parse a line into a parse struct */

