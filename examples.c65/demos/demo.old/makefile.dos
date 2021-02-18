.extensions : .c65 .m65

.m65.obj:
	ra65 $*.m65

.c65.obj :
	cc65 -O $*.c65
#	xopt $*.m65		# this slows it down, but makes it smaller
	ra65 $*.m65
	del $*.m65

all : demo0.com demo1.com demo2.com demo3.com mazedemo.com sprite.com .SYMBOLIC
	@echo.
	@echo Don't forget to press the joypad to begin the demos.
	@echo.

demo0.com : demo0.obj
	link65 -b200 -s512 -o demo0.com demo0.obj

demo1.com : demo1.obj random.obj pixel.obj
	link65 -b200 -s512 -o demo1.com demo1.obj random.obj pixel.obj

demo2.com : demo2.obj random.obj pixel.obj
	link65 -b200 -s512 -o demo2.com demo2.obj random.obj pixel.obj

demo3.com : demo3.obj random.obj pixel.obj
	link65 -b200 -s1024 -o demo3.com demo3.obj random.obj pixel.obj

mazedemo.com : mazedemo.obj random.obj pixel.obj maze.obj
	link65 -b200 -s512 -o mazedemo.com mazedemo.obj random.obj pixel.obj maze.obj

sprite.com : sprite.obj
	link65 -b200 -s512 -o sprite.com sprite.obj

clean : .SYMBOLIC
	del *.obj
	del *.com
	del demo.h

demo0.obj : demo.c65
        @echo $#define DEMO0 1 > demo.h
	cc65 -O demo.c65
	rename demo.m65 demo0.m65
	ra65 demo0.m65
	del demo0.m65

demo1.obj : demo.c65
        @echo $#define DEMO1 1 > demo.h
	cc65 -O demo.c65
	rename demo.m65 demo1.m65
	ra65 demo1.m65
	del demo1.m65

demo2.obj : demo.c65
        @echo $#define DEMO2 1 > demo.h
	cc65 -O demo.c65
	rename demo.m65 demo2.m65
	ra65 demo2.m65
	del demo2.m65

demo3.obj : demo.c65
        @echo $#define DEMO3 1 > demo.h
	cc65 -O demo.c65
	rename demo.m65 demo3.m65
	ra65 demo3.m65
	del demo3.m65
	
sprite.obj : sprite.c65 sprite.h test.h
