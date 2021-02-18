
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* rename a file */

int rename(old,new)
char * old;
char * new;
{
  char str[80];			/* buffer for rename string */
  char * ptr;

  fn_default(old, 0, str);	/* copy/default first name */
  strcat(str, ",");		/* put in the comma for ataridos */
  if (ptr = strchr(new, ':'))
    ++ptr;			/* skip the colon */
  else
    ptr = new;
  strcat(str, ptr);
  strcat(str, "\n");
  return(frename(str));	/* do the rename */
}
