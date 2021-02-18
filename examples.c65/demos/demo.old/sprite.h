/* SPRCTL0 */

#define BITS4  (0xc0)
#define BITS3  (0x80)
#define BITS2  (0x40)
#define BITS1  (0x00)

#define HFLIP  (0x20)
#define VFLIP  (0x10)

#define SHADOW         (0x07)
#define XORSHADOW      (0x06)
#define NONCOLLIDABLE  (0x05)
#define NORMAL         (0x04)
#define BOUNDARY       (0x03)
#define BOUNDARYSHADOW (0x02)
#define BKGRNDNOCOL    (0x01)
#define BKGRND         (0x00)

/* SPRCTL1 */

#define LITERAL        (0x80)
#define RELHVST        (0x30)
#define RELHVS         (0x20)
#define RELHV          (0x10)
#define RELPALETTE     (0x00)
#define EXISTPALETTE   (0x08)
#define SKIPSPRITE     (0x04)
#define DUP            (0x02)
#define DDOWN          (0x00)
#define DLEFT          (0x01)
#define DRIGHT         (0x00)

/* SPRCOLL */

#define DONTCOLLIDE    (0x20)

/* SCB data structures */
/* this stuff is vile  */
/* necessary to fix    */
/* the stupid compiler */

struct _scb {
  char sprctl0;
  char sprctl1;
  char sprcoll;
  char *next;
  char *sprdata;
  int x,y;
  int hscale, vscale;
  int stretch;
  int tilt;
};

struct scb_bit4 {
  struct _scb scb4;
  char cpal4[8];
  char coll4deposit;
};
#define B4COLLOFFSET (27)

struct scb_bit3 {
  struct _scb scb3;
  char cpal3[4];
  char coll3deposit;
};
#define B3COLLOFFSET (23)

struct scb_bit2 {
  struct _scb scb2;
  char cpal2[2];
  char coll2deposit;
};
#define B2COLLOFFSET (21)

struct scb_bit1 {
  struct _scb scb1;
  char cpal1[1];
  char coll1deposit;
};
#define B1COLLOFFSET (20)
