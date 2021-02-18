/**  Expand include path.. 	-Intruder   Oct 1993
***
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIX
#  include <unistd.h>
#endif
#include <sys/stat.h>

extern int debug;

char *ExpandIncludePath(char *name)
{
  char *path;
  char *incpath;
  struct stat st;

  if(!stat(name, &st)) /* search current dir first.. */
   return name;

  path = (char *)malloc(512);
  if(!path)
   {
     fprintf(stderr, "ExpandIncludePath(): can't allocate memory\n");
     return name;
   }

  incpath = (char *)getenv("CC65INCLUDE");
  if (debug) printf("INCLUDE:%s\n",incpath);
  if(incpath)
   {
     strncpy(path, incpath, 512 - (strlen(name) + 5));

#ifndef MSDOS
     if(path[strlen(path)-1] != '/')
       strcat(path, "/");
#else
     if(path[strlen(path)-1] != '\\')
       strcat(path, "\\");
#endif    
   }

  strcat(path, name);
  if(!stat(path, &st))
   return (char *)path;
 
  /* fprintf(stderr, "ExpandIncludePath(): file not found (%s)\n", path); */ 
  return (char *)name;
}

int FileExists(char *file)
{
 struct stat st;
 if(!stat(file, &st))
  return 1;
 return 0;
}
