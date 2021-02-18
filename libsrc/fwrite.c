#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
/*
   Item-stream write to fd.
   Entry: buf = address of source buffer
           sz = size of items in bytes
            n = number of items to write
           fd = file descriptor
   Returns a count of the items actually written or
   zero if an error occurred.
   May use ferror(), as always, to detect errors.
*/
fwrite(buf, sz, n, fd) 
char * buf; 
int sz, n, fd; 
{
  if (write(fd, buf, n*sz) <= 0)
	return (0);
  return (n);
}
