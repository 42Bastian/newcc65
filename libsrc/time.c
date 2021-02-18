#define NOARGC  /* no arg count passing */
#define FIXARGC /* don't expect arg counts passed in */


/*
   This software is copyright 1989 by John Dunning.  See the file
   'COPYLEFT.JRD' for the full copyright notice.
*/

/*
   time-hacking stuff
*/

#define RTCLOK ((char *)0x12)

gtime(time)
char * time;		/* snapshot RTCLOK */
{
  time[0] = *(RTCLOK);
  time[1] = *(RTCLOK+1);
  time[2] = *(RTCLOK+2);
}

calctim(t0, xtime)
char * t0;			/* initial time */
char * xtime;			/* 4 bytes, hr, min, sec, 60th */
{
  char t1[3];
  int temp[4];
  
  gtime(t1);			/* get current time */
/* subtrace t0 from t1 */
#asm
  ldy #8		; index of t1
  jsr locysp		; point at it
  sta $E0		; tmp ptr
  stx $E1
  ldy #15		; index of t0
  jsr ldaxysp		; get pointer
  sta $D4
  stx $D5
  ldy #2
  sec
  lda ($E0),y		; get t1 byte
  sbc ($D4),y		; sub t0 byte
  sta ($E0),y		; back in t1
  dey
  lda ($E0),y		; get t1 byte
  sbc ($D4),y		; sub t0 byte
  sta ($E0),y		; back in t1
  dey
  lda ($E0),y		; get t1 byte
  sbc ($D4),y		; sub t0 byte
  sta ($E0),y		; back in t1
#endasm

/* tprintf(" t1 %x %x %x\n", t1[0], t1[1], t1[2]); */

  temp[3] = (t1[2] & 0x3F) * 100 / 64;		/* compute hundreths */
  temp[2] = ((t1[2] & 0xC0) / 64) | 		/* compute seconds */
            ((t1[1] & 0xFF) * 4) |
	    ((t1[0] & 0x1F) * 1024);

  temp[1] = temp[2] / 60;			/* ... minutes */
  temp[2] = temp[2] % 60;
  temp[0] = temp[1] / 60;			/* ... hours */
  temp[1] = temp[1] % 60;

/* tprintf(" temp %d %d %d %d\n", temp[0], temp[1], temp[2], temp[3]); */

  xtime[0] = temp[0];
  xtime[1] = temp[1];
  xtime[2] = temp[2];
  xtime[3] = temp[3];
}
