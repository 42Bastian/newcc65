foreach c { startup atoi fputs fgets gets fprintf reverse iscntrl isprint itox atoib isalnum iscons ispunct itoab otoi strcmp dtoi isspace itod pmalloc utoi fclose isascii isgraph isupper itoo time xtoi fopen isatty islower isxdigit itou toascii open rename delete fname fread fwrite readargs strcat }
  cp \a8\cc8\lib\$c.c .
endfor

foreach m65 { stdio frename fdelete close closeall cputc cgetc cgets read write rwcommon tprintf heap itoa isalpha isodigit isdigit iswhite bzero bcopy strchr getch parselin tolower toupper }
  cp \a8\cc8\lib\$m65.m65 .
endfor

