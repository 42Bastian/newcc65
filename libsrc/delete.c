
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/* delete a file */
int delete(name)
char * name;
{
  char buf[80];

  fn_default(name, 0, buf);
  strcat(buf, "\n");
  return(fdelete(buf));
}

