
/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/


/*
   handy util for fixing up file names
*/

fn_default(name, ext, target)
char * name;
char * ext;
char * target;
{
  char buf[80];		/* temp storage */

/* this is bummed for space, not speed; it'll copy the data around
   even when it doesn't have to... */

  if (!strchr(name, ':'))
    {
      strcpy(buf, "D:");
      strcat(buf, name);
      strcpy(target, buf);
    }
  else
    strcpy(target, name);
  if (ext)
    if (!strchr(target, '.'))
      {
	strcat(target, ext);
      }
}
