/* Lynx-Lib header  */
/* created 97/11/13 */
/* 42B.Schick       */
/* changed :
      98/01/06  42BS  added IRQ_ENTRY and IRQ_EXIT

*/

#ifndef _LYNXLIB_

#define _LYNXLIB_

//#define RA65_ERROR

#ifndef RA65_ERROR

/* irq */
extern InitIRQ();
extern InstallIRQ();
extern DeInstallIRQ();

/* gfx */
extern DrawFBox();
extern SetPixel();
extern GetPixel();
extern DrawLine();
extern DrawSprite();

/* display stuff */
extern SetBuffers();
extern SwapBuffers();
extern Flip();

/* palette */
extern SetRGB();
extern char DefaultPAL[];

/* eeprom */
extern EE_Read();
extern EE_Write();
extern EE_Erase();

/* misc */
extern random();
extern InstallUploader();

extern TextOut();
extern TextOutExt();

extern BCDAdd();
extern BCDAddConst();
extern BCDSub();
extern BCDSubConst();
extern BCDToASCII();

// sample
extern SmpInit();
extern SmpStart();
extern SmpStop();
extern SmpActive();
#endif

/* divider for InstallUploader */
#define _62500Bd	1
#define _31250Bd	3
#define _9600Bd		12

#define IRQ_ENTRY()\
  asm(" ldx #sp\n"\
      " ldy <runtime_save_size\n"\
      " lda 0,x\n"\
      " pha\n"\
      " inx\n"\
      " dey\n"\
      " bpl *-5\n"\
      " jsr decsp2")

#define IRQ_EXIT()\
  asm(" ldx #tmp4\n"\
      " ldy <runtime_save_size\n"\
      " pla\n"\
      " sta 0,x\n"\
      " dex\n"\
      " dey\n"\
      " bpl *-5")

/* library-variables */
extern uchar * ScreenBuffer,
             * RenderBuffer;
#endif
