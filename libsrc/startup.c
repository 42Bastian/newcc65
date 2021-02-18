
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* standard startup piece for c programs.  replace this by
   by supplying your own _main if you want... */

/* int _argv[16]; .. use PRNBUF instead */

int _argv = 0x3C0;		/* Atari Rom OS's Printer buf, 32 bytes */

#ifdef parser
/* the C parser.  The .m65 one is in parselin.m65 */

static char ___ch;

/* arg line parser.  it's separate so it can be called independently */
_parseline(line, argv)
char * line;
int * argv;
{
  int nargs;
  char * p;

  nargs = 0;
  if ((p = line) == 0)
	return(0);
  while (*p)		/* while not at eos, scan */
	{
	while (iswhite(___ch = *p))
		*p++ = 0;	/* zap and skip whitespace */
	if ((___ch == 0) || (___ch == '\n'))
		break;		/* end of str, stop */
	*argv++ = p;		/* remember the pointer */
	++nargs;
	while (!iswhite(*p)) ++p;	/* find end of string */
	}
  return(nargs);
}

#endif

_main(cmdline)
char * cmdline;
{
  main(_parseline(cmdline, _argv), _argv);
}
