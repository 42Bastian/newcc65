#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */
extern int Udevice[];
/*
   Return "true" if fd is a device, else "false"
*/
isatty(fd) 
int fd; 
{
  return (Udevice[fd]);
}
