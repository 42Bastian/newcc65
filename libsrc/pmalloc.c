#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */


/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   homebrew memory management routines, by jrd
*/

/* these vars defined in heap.m65 */
extern int _himem;			/* pointer to next unused mem */

/* permanent alloc */
pmalloc(nbytes)
int nbytes;
{
  int ptr;

  ptr = _himem;		/* get current ptr */
  _himem += nbytes;	/* move up by nbytes */
  return(ptr);
}

