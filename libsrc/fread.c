#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   Item-stream read from fd.
   Entry: buf = address of target buffer
           sz = size of items in bytes
            n = number of items to read
           fd = file descriptor
   Returns a count of the items actually read.
   Use feof() and ferror() to determine file status.
*/
fread(buf, sz, n, fd) 
char * buf; 
int sz, n, fd; 
{
  return (read(fd, buf, n * sz) / sz);
}
