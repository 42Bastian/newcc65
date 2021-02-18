/* print the copyright notice...  -Intruder.. */
#include <stdio.h>

void print_copyleft()
{
printf(
"   This is the copyright notice for RA65, LINK65, LIBR65, and other\n"
"Atari 8-bit programs.  Said programs are Copyright 1989, by John R.\n"
"Dunning.  All rights reserved, with the following exceptions:\n"
"\n"
"    Anyone may copy or redistribute these programs, provided that:\n"
"\n"
"1:  You don't charge anything for the copy.  It is permissable to\n"
"    charge a nominal fee for media, etc.\n"
"\n"
"2:  All source code and documentation for the programs is made\n"
"    available as part of the distribution.\n"
"\n"
"3:  This copyright notice is preserved verbatim, and included in\n"
"    the distribution.\n"
);
  printf("\n[PRESS RETURN TO CONTINUE]");
  getchar();
printf("\r"
"    You are allowed to modify these programs, and redistribute the \n"
"modified versions, provided that the modifications are clearly noted.\n"
"\n"
"    There is NO WARRANTY with this software, it comes as is, and is\n"
"distributed in the hope that it may be useful.\n"
""
"    This copyright notice applies to any program which contains\n"
"this text, or the refers to this file.\n"
"\n"
"    This copyright notice is based on the one published by the Free\n"
"Software Foundation, sometimes known as the GNU project.  The idea\n"
"is the same as theirs, ie the software is free, and is intended to\n"
"stay that way.  Everybody has the right to copy, modify, and re-\n"
"distribute this software.  Nobody has the right to prevent anyone\n"
"else from copying, modifying or redistributing it.\n"
);
}
